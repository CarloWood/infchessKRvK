#pragma once

#include <iosfwd>

enum color_type
{
  black,
  white
};

struct Color
{
  color_type color_;

  constexpr Color() : color_{black} { }
  constexpr Color(color_type color) : color_(color) { }

  Color& operator=(color_type color)
  {
    color_ = color;
    return *this;
  }

  [[nodiscard]] constexpr Color opponent() const
  {
    return {color_ == black ? white : black};
  }
};

std::ostream& operator<<(std::ostream& os, Color color);

inline bool operator==(Color lhs, Color rhs)
{
  return lhs.color_ == rhs.color_;
}
