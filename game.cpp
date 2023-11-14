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
#include <fstream>

using Cell = std::pair<unsigned int, unsigned int>;
using Board = std::set<Cell>;

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

void clearScreen() {
    #ifdef _WIN32
    system("cls");
    #else
    system("clear");
    #endif
}

void initialize_from_random_soup(
        Board &board, unsigned int width, unsigned int height) {

    board = {};
    for(int i = 0; i < height; i++){
        for(int j = 0; j < width; j++){
            int alive = rand() % 2 + 0.5;
            if(alive == 1) board.insert({i, j});
        }
    }
}

void initialize_from_RLE(Board& board, const std::string& rle,
                                            int startX, int startY) {
    board = {};

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


void update_section(const Board &b, Board &slice,
        size_t start_row, size_t end_row ) {

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
        if(alive && (n_count == 2 || n_count == 3)) slice.insert(c);
        else if(!alive && n_count == 3) slice.insert(c);
    }
}

void threaded_get_next_board(
        Board &board,
        std::vector<std::thread> &threads,
        int num_threads){

    int rows_per_thread = board.size() / num_threads;

    Board next_board;

    std::vector<Board> slices(num_threads);

    for(int i = 0; i < num_threads; i++){
        int start_row = i * rows_per_thread;
        int end_row = (i + 1) * rows_per_thread;

        if(i == num_threads - 1){
            end_row = board.size() - 1;
        }

        threads[i] = std::thread(update_section,
                std::ref(board),
                std::ref(slices[i]),
                start_row, end_row);
    }

    for(auto &t : threads){
        t.join();
    }

    for(const Board &slice : slices){
        next_board.insert(slice.begin(), slice.end());
    }

    board = std::move(next_board);

}

void run_multithreaded(Board &board, int iterations, bool print) {

    int i = 0;

    int num_threads = std::thread::hardware_concurrency();
    std::vector<std::thread> threads(num_threads);

    while(i < iterations){
        if(print) {
            std::stringstream buffer;

            buffer << "\033[2J\033[H";

            for (const Cell& cell : board) {
                unsigned int r = cell.first, c = cell.second;
                buffer << "\033[" << r + 1 << ";" << c + 1 << "H";
                buffer << "□";
            }

            // Print the entire buffer to std::cout in one go
            clearScreen();
            std::cout << buffer.str();
            std::cout.flush();
            std::this_thread::sleep_for(std::chrono::milliseconds(30));
        }

        threaded_get_next_board(board, threads, num_threads);
        //i++;
    }
}

int main() {
    //std::string RLE_file = "2c5-spaceship-gun-p416.txt";
    //std::string RLE_file = "smaller-ship.txt";
    //std::string RLE_file = "oscillator.txt";
    std::string RLE_file = "queen_bee.txt";

    std::ifstream file("./rle_files/" + RLE_file);

    if (!file.is_open()) {
        std::cerr << "Error opening file: " << RLE_file << std::endl;
        return 1;
    }

    std::stringstream buffer;
    buffer << file.rdbuf();

    file.close();

    std::string contents = buffer.str();

    const char* RLE = contents.c_str();

    Board board = {};

    int iterations = 1000;
    bool print = true;
    unsigned int board_height = board.size() + 2;
    unsigned int board_width = board.size() + 2;

    //initialize_from_random_soup(board, board_height, board_width);
    initialize_from_RLE(board, RLE, board_width/2, board_height/2);

    auto start = std::chrono::high_resolution_clock::now();
    run_multithreaded(board, iterations, print);
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed_threaded = end - start;

    std::cout << "Elapsed time print: " <<
        elapsed_threaded.count() << " seconds" << std::endl;

    return 0;
}

