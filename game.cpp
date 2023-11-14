#include <array>
#include <map>
#include <stdlib.h>
#include <iostream>
#include <chrono>
#include <thread>
#include <sstream>
#include <utility>
#include <set>
#include <thread>
#include <mutex>

using Cell = std::pair<unsigned int, unsigned int>;
using Board = std::set<Cell>;

std::mutex new_board_mutex;

Board get_neighbours(Cell c) {
    Board neighbours;
    for(int dx = -1; dx <= 1; dx++){
        for(int dy = -1; dy <= 1; dy++){
            if(dx == 0 && dy == 0) continue;
            neighbours.insert({c.first + dx, c.second + dy});
        }
    }

    return neighbours;
}

void get_next_board(Board &b) {
    Board new_b;
    Board candidates = b;

    for(const Cell &c : b){
        Board neighbours = get_neighbours(c);
        candidates.insert(neighbours.begin(), neighbours.end());
    }

    for(const Cell &c : candidates){
        int n_count = 0;

        for(const Cell &n : get_neighbours(c)){
            if(b.count(n)) n_count ++;
        }

        bool alive = b.count(c);
        if(alive && (n_count == 2 || n_count == 3)) new_b.insert(c);
        else if(!alive && n_count == 3) new_b.insert(c);
    }

    b = std::move(new_b);
}


void clearScreen() {
    #ifdef _WIN32
    system("cls");
    #else
    system("clear");
    #endif
}

void initialize_from_RLE(Board& board, const std::string& rle,
                                            int startX, int startY) {
    int x = startX, y = startY;
    std::istringstream rleStream(rle);
    char ch;

    while (rleStream.get(ch)) {
        std::string countStr;

        // Accumulate digits to form the count number
        while (std::isdigit(ch)) {
            countStr.push_back(ch);
            if (!rleStream.get(ch)) {
                break;
            }
        }

        int count = countStr.empty() ? 1 : std::stoi(countStr);

        switch (ch) {
            case 'b': // Dead cell
            case '.':
                x += count;
                break;
            case 'o': // Alive cell
                for (int i = 0; i < count; ++i, ++x) {
                    board.insert({x, y});
                }
                break;
            case '$': // End of line
                y += count;
                x = startX;
                break;
        }
    }
}


void update_section(const Board &b, Board &new_board,
        size_t start_row, size_t end_row ) {

    Board new_section;
    Board candidates;

    auto it = b.begin();
    std::advance(it, start_row);

    // slice candidates by index
    for(size_t i = start_row; i < end_row && it != b.end(); i++, it++){
        candidates.insert(*it);
    }

    Board c_neighbours;
    for(const Cell &c : candidates) {
        Board neighbours = get_neighbours(c);
        c_neighbours.insert(neighbours.begin(), neighbours.end());
    }
    candidates.insert(c_neighbours.begin(), c_neighbours.end());

    for(const Cell &c : candidates){
        int n_count = 0;

        for(const Cell &n : get_neighbours(c)){
            if(b.count(n)) n_count ++;
        }

        bool alive = b.count(c);
        if(alive && (n_count == 2 || n_count == 3)) new_section.insert(c);
        else if(!alive && n_count == 3) new_section.insert(c);
    }
    std::lock_guard<std::mutex> lock(new_board_mutex);
    new_board.insert(new_section.begin(), new_section.end());
}

void threaded_get_next_board(
        Board &board,
        std::vector<std::thread> &threads,
        int num_threads){

    int rows_per_thread = board.size() / num_threads;

    Board next_board;

    for(int i = 0; i < num_threads; i++){
        int start_row = i * rows_per_thread;
        int end_row = (i + 1) * rows_per_thread;

        if(i == num_threads - 1){
            end_row = board.size() - 1;
        }

        threads[i] = std::thread(update_section,
                std::ref(board),
                std::ref(next_board),
                start_row, end_row);
    }

    for(auto &t : threads){
        t.join();
    }

    board = std::move(next_board);
}

void initialize_from_random_soup(
        Board &board, unsigned int width, unsigned int height) {

    for(int i = 0; i < height; i++){
        for(int j = 0; j < width; j++){
            int alive = rand() % 2 + 0.5;
            if(alive == 1) board.insert({i, j});
        }
    }
}


int main() {

    const char *RLE = R""""(
        26b2o16b2o$26b2o7b2o7b2o$35b2o5$10b2o48b2o$10b2o48b2o3$28bo14bo$28b3o10b3o2$
29b3o8b3o5$18b2o32b2o$18b2o32b2o4$18b2o32b2o$18b2o32b2o5$29b3o8b3o2$28b3o10b
3o$28bo14bo3$10b2o48b2o$10b2o48b2o5$35b2o$26b2o7b2o7b2o$26b2o16b2o4$8b3o$7bo
2bo$7bo2bo$7b2o4$17b3o$17bo2bo$20bo$3o15b3o$o$o2bo$b3o4$12b2o$10bo2bo$10bo2bo
$10b3o!
    )"""";

    Board board = {};

    unsigned int board_height = 400;
    unsigned int board_width = 200;

    //initialize_from_RLE(board, RLE, board_width/2, board_height/2);
    initialize_from_random_soup(board, board_height, board_width);

    int iterations = 100;
    int i = 0;

    int num_threads = std::thread::hardware_concurrency();
    std::vector<std::thread> threads(num_threads);

    auto start = std::chrono::high_resolution_clock::now();
    while(i < iterations){
        clearScreen();

        for (const Cell& cell : board) {
           unsigned int r = cell.first, c = cell.second;
           //Position the cursor at the start of the changed cell
           std::cout << "\033[" << r + 1 << ";" << c + 1 << "H";
           std::cout << "█";
        }

        std::cout.flush();

        threaded_get_next_board(board, threads, num_threads);
        i++;
    }
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed_threaded = end - start;

    start = std::chrono::high_resolution_clock::now();

    i = 0;

    initialize_from_random_soup(board, board_height, board_width);
    while(i < iterations){
        clearScreen();

        for (const Cell& cell : board) {
          unsigned int r = cell.first, c = cell.second;
          //Position the cursor at the start of the changed cell
          std::cout << "\033[" << r + 1 << ";" << c + 1 << "H";
          std::cout << "█";
        }

        std::cout.flush();

        get_next_board(board);
        i++;
    }
    end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed_single = end - start;

    std::cout << "Elapsed time Threaded: " <<
        elapsed_threaded.count() << " seconds" << std::endl;

    std::cout << "Elapsed time single thread: " <<
                elapsed_single.count() << " seconds" << std::endl;

    return 0;
}

