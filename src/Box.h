#include "utils/utf8_glyph_length.h"
#include <algorithm>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

class Box
{
 private:
  std::ostringstream oss;
  mutable std::vector<std::string> lines;
  mutable bool lines_cached   = false;
  mutable size_t width_cached = 0;

  static size_t width_of(std::string const line)
  {
    size_t width = 0;
    char8_t const* p = reinterpret_cast<char8_t const*>(line.data());
    while (*p)
    {
      int len = utils::utf8_glyph_length(p);
      p += len;
      ++width;
    }
    return width;
  }

  void update_cache() const
  {
    if (!lines_cached)
    {
      lines.clear();
      std::string content = oss.str();
      std::istringstream iss(content);
      std::string line;

      while (std::getline(iss, line)) { lines.push_back(line); }

      // Calculate width (longest line)
      width_cached = 0;
      for (const auto& line : lines) { width_cached = std::max(width_cached, width_of(line)); }

      lines_cached = true;
    }
  }

 public:
  // Return the ostringstream for writing
  std::ostream& stream()
  {
    lines_cached = false;  // Invalidate cache when stream is accessed
    return oss;
  }

  // Convert to vector of strings (line by line)
  std::vector<std::string> to_lines() const
  {
    update_cache();
    return lines;
  }

  // Get number of lines
  size_t height() const
  {
    update_cache();
    return lines.size();
  }

  // Get width (length of longest line)
  size_t width() const
  {
    update_cache();
    return width_cached;
  }

  // Get a specific line, padded to full width
  std::string get_line(size_t index) const
  {
    update_cache();
    if (index >= lines.size())
    {
      return std::string(width_cached, ' ');  // Return spaces if no line exists
    }

    std::string line = lines[index];
    size_t w = width_of(line);
    if (w < width_cached)
    {
      line.append(width_cached - w, ' ');  // Pad with spaces
    }
    return line;
  }
};

// Class to manage a row of boxes that can be added dynamically
class BoxRow
{
 private:
  std::vector<const Box*> boxes;
  size_t total_width = 0;

 public:
  // Add a box to the row and return the new total width
  size_t add_box(const Box& box)
  {
    boxes.push_back(&box);
    total_width += box.width();
    return total_width;
  }

  // Get current total width
  size_t width() const { return total_width; }

  // Get number of boxes in the row
  size_t count() const { return boxes.size(); }

  // Check if row is empty
  bool empty() const { return boxes.empty(); }

  // Flush (print) the current row and clear it
  void flush()
  {
    if (boxes.empty()) return;

    // Find the maximum height among all boxes
    size_t max_height = 0;
    for (const auto* box : boxes) { max_height = std::max(max_height, box->height()); }

    // Print each line
    for (size_t line_idx = 0; line_idx < max_height; ++line_idx)
    {
      for (size_t box_idx = 0; box_idx < boxes.size(); ++box_idx)
      {
        std::cout << boxes[box_idx]->get_line(line_idx);
        if (box_idx < boxes.size() - 1)
        {
          // Optional: add separator between boxes (remove if not wanted)
          std::cout << "  ";
        }
      }
      std::cout << '\n';
    }

    // Clear the row
    boxes.clear();
    total_width = 0;
  }

  // Clear the row without printing
  void clear()
  {
    boxes.clear();
    total_width = 0;
  }
};

// Function to concatenate multiple boxes side by side (kept for compatibility)
template <typename... Boxes>
void print_boxes_side_by_side(const Boxes&... boxes)
{
  BoxRow row;
  (row.add_box(boxes), ...);  // C++17 fold expression
  row.flush();
}
