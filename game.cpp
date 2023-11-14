#include <array>
#include <map>
#include <stdlib.h>
#include <iostream>
#include <chrono>
#include <thread>
#include <sstream>
#include <utility>
#include <set>

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

void spawn_heavy_spaceship(Board& board, int x, int y) {
    for (int dx = 0; dx < 5; ++dx) {
        for (int dy = 0; dy < 4; ++dy) {
            board.erase({x + dx, y + dy});
        }
    }

    board.insert({x + 1, y});
    board.insert({x + 4, y});
    board.insert({x, y + 1});
    board.insert({x, y + 2});
    board.insert({x + 4, y + 2});
    board.insert({x, y + 3});
    board.insert({x + 1, y + 3});
    board.insert({x + 2, y + 3});
    board.insert({x + 3, y + 3});
}

void initialize_from_RLE(Board& board, const std::string& rle, int startX, int startY) {
    int x = startX, y = startY;
    std::istringstream rleStream(rle);
    char ch;
    int count;

    while (rleStream >> ch) {
        if (std::isdigit(ch)) {
            // If the character is a digit, read the full number
            rleStream.putback(ch);
            rleStream >> count;
        } else {
            // Single character run
            count = 1;
        }

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

int main() {
    Board board = {};

    const char *RLE = R""""(
        198bo$198bobo$175bobo20boo$176boo$176bo3$178bo$179bo$177b3o$248bxx7b3x
    $247bxbbx4bx4bx$247bxbbx4bx5bx$247bx3bx8bx$248bxx8bxx$172bo$170bobo75b
    xx8bxx$171boo29bo48bx8bx$174boo24bobo42bxx8bx5bx10bxx$173bobo25boo42bx
    9bx4bx11bxx$175bo22boo49bx7b3x$197bobo46bxbbx$6bo192bo$5bo$5b3o$3bo$4b
    o$bb3o38bxx38boo38bxx98boo68bxx$43bxx38boo38bxbbx96bobbo66bxbbx$116b3x
    97b3o67b3x$115bx3bxbx57boo34bo3bobo63bx3bxbx$115bx3bxbx3bxx53boo33bo3b
    obo3boo58bx3bxbx3bxx$116bxx61bo36boo68bxx$118bx3bx95bo3bo65bx3bx3$71bo
    17bo$72boo13boo27bxx5bxx91boo5boo61bxx5bxx$71boo15boo$115bx9bx89bo9bo
    59bx9bx$115bx3bxbx3bx89bo3bobo3bo59bx3bxbx3bx$115bx3bxbx3bx89bo3bobo3b
    o59bx3bxbx3bx$116bxbx3bxbx91bobo3bobo61bxbx3bxbx$117bx5bx93bo5bo63bx5b
    x4$73boo11boo$72bobo11bobo$74bo11bo5$43bxx38boo38bxx98boo68bxx$3b3o37b
    xx38boo38bxx98boo68bxx$3bo$4bo$3o$bbo$bo15$44bo$43bo$43b3o$41bo$42bo$
    40b3o68bxx68boo98bxx$111bxx68boo98bxx5$51bo$51bobo$45bobo3boo$35bo9boo
    $36bo9bo$34b3o$112bx5bx63bo5bo93bx5bx$111bxbx3bxbx61bobo3bobo91bxbx3bx
    bx$110bx3bxbx3bx59bo3bobo3bo89bx3bxbx3bx$110bx3bxbx3bx59bo3bobo3bo89bx
    3bxbx3bx$110bx9bx59bo9bo89bx9bx$53boo$52boo57bxx5bxx61boo5boo91bxx5bxx
    $54bo3$39bo73bx3bx65bo3bo95bx3bx$40bo77bxx68boo36bo61bxx$38b3o68bxx3bx
    bx3bx58boo3bobo3bo33boo53bxx3bxbx3bx$42bo71bxbx3bx63bobo3bo34boo57bxbx
    3bx$41bo75b3x67b3o97b3x$41b3o65bxbbx66bobbo96bxbbx$111bxx68boo98bxx5$
    206bo$206bobo117bxbbx$206boo22bo85b3x7bx$203boo25bobo69bxx11bx4bx9bx$
    203bobo24boo70bxx10bx5bx8bxx$203bo29boo80bx8bx$233bobo58bxx20bxx8bxx$
    8boo7b3o58bxx7b3x58boo7b3o73bo14bxx7b3x33bxbx$7bobbo4bo4bo56bxbbx4bx4b
    x56bobbo4bo4bo86bxbbx4bx4bx31b3x21bxx8bxx$7bobbo4bo5bo55bxbbx4bx5bx55b
    obbo4bo5bo56bo28bxbbx4bx5bx53bx8bx3bx$7bo3bo8bo56bx3bx8bx56bo3bo8bo26b
    o30bobo26bx3bx8bx29bxbx21bx5bx4bxbbx$8boo8boo58bxx8bxx58boo8boo27boo9b
    o19boo28bxx8bxx30bxx23bx4bx4bxbbx$186bobo8boo92bx24b3x7bxx$8boo8boo58b
    xx8bxx58boo8boo37bobo26b3o19bxx8bxx$11bo8bo60bx8bx60bo8bo65bo24bx8bx$
    5boo8bo5bo10boo41bxx8bx5bx10bxx41boo8bo5bo10boo53bo17bxx8bx5bx10bxx$5b
    o9bo4bo11boo41bx9bx4bx11bxx41bo9bo4bo11boo71bx9bx4bx11bxx$9bo7b3o59bx
    7b3x59bo7b3o89bx7b3x$6bobbo66bxbbx66bobbo79bo16bxbbx$214boo12boo$213b
    oo13bobo$215bo3$53boo68bxx68boo98bxx$53bobbo66bxbbx66bobbo96bxbbx$46b
    3o67b3x67b3o97b3x$45bo3bobo63bx3bxbx63bo3bobo93bx3bxbx$45bo3bobo3boo
    58bx3bxbx3bxx58bo3bobo3boo88bx3bxbx3bxx$46boo68bxx68boo98bxx$48bo3bo
    65bx3bx65bo3bo95bx3bx4$46boo5boo61bxx5bxx61boo5boo91bxx5bxx$$45bo9bo
    59bx9bx59bo9bo89bx9bx$45bo3bobo3bo59bx3bxbx3bx59bo3bobo3bo89bx3bxbx3bx
    $45bo3bobo3bo59bx3bxbx3bx59bo3bobo3bo89bx3bxbx3bx$46bobo3bobo61bxbx3bx
    bx61bobo3bobo91bxbx3bxbx$47bo5bo63bx5bx63bo5bo93bx5bx11$53boo68bxx68b
    oo98bxx$53boo68bxx68boo98bxx!

    )"""";
    initialize_from_RLE(board, RLE, 50, 50);

    unsigned int screen_width = 400;
    unsigned int screen_height = 200;


    int iterations = 5;
    int i = 0;

    while(i < iterations){
        clearScreen();

        std::stringstream buffer;

        for (const Cell& cell : board) {
            unsigned int r = cell.first, c = cell.second;
            // Position the cursor at the start of the changed cell
            std::cout << "\033[" << r + 1 << ";" << c + 1 << "H";
            std::cout << "█";
        }

        std::cout.flush();
        std::this_thread::sleep_for(std::chrono::milliseconds(11));
        get_next_board(board);
        //i++;
    }

    return 0;
}

