#ifndef GAME_HPP
#define GAME_HPP

typedef std::pair<unsigned int, unsigned int> Cell;
typedef std::set<Cell> Board;

void threaded_get_next_board(Board &board,
        std::vector<std::thread> &threads,
        int num_threads,
        std::vector<Board> &slices);

void update_section(const Board &b, Board &slice,
                        size_t start_row, size_t end_row );

#endif // GAME_HPP
