#include "Color.h"
#include <tuple>

// Returns a tuple of [piece, x, y] where piece is 'K' or 'R' and x and y are in the range [0, board_size).
std::tuple<char, int, int> parse_move(Color color, int board_size_x, int board_size_y);
