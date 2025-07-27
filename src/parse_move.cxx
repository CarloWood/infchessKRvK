#include "sys.h"
#include "parse_move.h"
#include <iostream>
#include <stdexcept>
#include <string>

// This function was written by Claude Sonnet 4.
std::tuple<char, int, int> parseChessMove()
{
  std::string line;
  std::getline(std::cin, line);

  if (line.empty()) { throw std::invalid_argument("Empty input"); }

  // Extract piece (first character).
  char piece = line[0];
  if (piece != 'K' && piece != 'R') { throw std::invalid_argument("Invalid piece: must be 'K' or 'R'"); }

  // Find where the y-coordinate starts (first digit).
  size_t yStart = 1;
  while (yStart < line.length() && !std::isdigit(line[yStart])) { yStart++; }

  if (yStart == line.length()) { throw std::invalid_argument("No y-coordinate found"); }

  // Extract x-coordinate string.
  std::string xCoordStr = line.substr(1, yStart - 1);
  if (xCoordStr.empty()) { throw std::invalid_argument("No x-coordinate found"); }

  // Validate x-coordinate contains only lowercase letters.
  for (char c : xCoordStr)
  {
    if (c < 'a' || c > 'z') { throw std::invalid_argument("X-coordinate must contain only lowercase letters"); }
  }

  // Convert x-coordinate to integer (Excel-like column system).
  int xCoord = 0;
  for (char c : xCoordStr) { xCoord = xCoord * 26 + (c - 'a' + 1); }
  xCoord -= 1;  // Adjust to 0-based indexing.

  // Extract and convert y-coordinate.
  std::string yCoordStr = line.substr(yStart);
  int yCoord;
  try
  {
    yCoord = std::stoi(yCoordStr);
  }
  catch (const std::exception&)
  {
    throw std::invalid_argument("Invalid y-coordinate");
  }

  if (yCoord <= 0) { throw std::invalid_argument("Y-coordinate must be positive"); }

  // Return 0-based y-coordinate.
  return std::make_tuple(piece, xCoord, yCoord - 1);
}

std::tuple<char, int, int> parse_move(Color color, int board_size)
{
  std::tuple<char, int, int> result;
  for (;;)
  {
    try
    {
      std::cout << "Enter move for " << color << ": ";
      result = parseChessMove();
      auto [piece, x, y] = result;
      if (color == black && piece == 'R')
        throw std::invalid_argument("Black doesn't have a rook");
      if (x >= board_size)
        throw std::invalid_argument("The x-coordinate is outside the board");
      if (y >= board_size)
        throw std::invalid_argument("The y-coordinate is outside the board");
      break;
    }
    catch (const std::exception& e)
    {
      std::cerr << "Error: " << e.what() << std::endl;
    }
  }
  return result;
}
