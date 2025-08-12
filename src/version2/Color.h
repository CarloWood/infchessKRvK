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

  constexpr operator color_type() const { return color_; }

  friend std::ostream& operator<<(std::ostream& os, Color color);
};
