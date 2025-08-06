#pragma once

#include <iosfwd>

enum color_type
{
  black,
  white
};

class Color
{
 private:
  color_type color_;

 public:
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

  friend std::ostream& operator<<(std::ostream& os, Color color);

  friend bool operator==(Color lhs, Color rhs)
  {
    return lhs.color_ == rhs.color_;
  }
};
