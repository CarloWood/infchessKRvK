#include "Color.h"
#include <iostream>

std::ostream& operator<<(std::ostream& os, Color color)
{
  switch (color.color_)
  {
    case black:
      os << "black";
      break;
    case white:
      os << "white";
      break;
  }
  return os;
}
