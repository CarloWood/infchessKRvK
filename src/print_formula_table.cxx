#include "sys.h"
#include "print_formula_table.h"
#include <iomanip>
#include <iostream>

struct StreamStateGuard
{
  std::ios& stream;
  std::ios state;

  StreamStateGuard(std::ios& s) : stream(s), state(nullptr)
  {
    state.copyfmt(s);           // Save state.
  }

  ~StreamStateGuard()
  {
    stream.copyfmt(state);      // Auto-restore.
  }
};

int print_formula_table(int m, std::vector<int> const& values)
{
  int const spacing = 4;
  int const number_of_values = values.size();

  // The input vector is considered to be comprised of values
  // that equal k_n + 2 * n, where n is the index of the vector.

  // Calculate all k_n.
  std::vector<int> k_values(number_of_values);
  for (int n = 0; n < number_of_values; ++n)
    k_values[n] = values[n] - 2 * n;

  // Get the final value of k.
  // Unfortunately, the last entries can contain errors because they are
  // close to the virtual edge: if black steps over that edge, the position
  // is declared a draw, so black might attempt to go for that goal instead
  // of the real optimal play.
  //
  // The heuristic that I'm using is that if there is a group with a size
  // of at least five then the first of that must be the stable_n value.
  //
  // For example, assume we have the following k values:
  //
  //        n = ... 23 24 25 26 27 28 29 30 31 |
  //   values = ... 54 58 60 62 64 66 68 72 76 |
  // k_values = ...  8 10 10 10 10 10 10 12 14 |  (values - 2 * n)
  //
  // Then 10 is the final_k and the first 10 corresponds to stable_n (25).              // From the example:
  int stable_n = number_of_values - 1;                                                  // stable_n = 31
  int final_k = k_values[stable_n];                                                     // final_k = 14
  int count = 1;                                                                        // count = 1

  // Determine the smallest N for which final_k + 2 * n == values[n] for all n >= N.    // From the example:
  int prev_k = final_k;                                                                 // prev_k = 14
                                                                                        // Loop: first          second     third
  for (int n = stable_n - 1; n > 1; --n)                                                // n = 30               n = 29     n = 28
    if (k_values[n] == prev_k)          // The same k value as the previous one?        // 12 == 14?            10 == 12?  10 == 10?
    {
      if (final_k == prev_k)                                                            //                                 14 == 10?
        --stable_n;
      else if (++count == 5)                                                            //                                 count = 2
      {
        stable_n = n;
        final_k = prev_k;
      }
    }
    else if (count == 5)                                                                // 1 == 5?              1 == 5?
      break;
    else
    {
      prev_k = k_values[n];                                                             // prev_k = 12          prev_k = 10
      count = 1;                                                                        // count = 1            count = 1
    }

  // Print first line, e.g: n =  0   1   2   3   4   5   ⩾6
  std::cout << "     n =  0";
  for (int n = 1; n < stable_n; ++n)
    std::cout << std::setw(spacing) << n;
  std::cout << "   ⩾" << stable_n << std::endl;

  // Find groups for display (consecutive same k values, excluding the stable tail).
  struct Group
  {
    int start;  // First value of the range.
    int end;    // One past the end.
    int k;      // Corresponding k value for this range.
  };
  std::vector<Group> groups;

  // Process positions 1 to stable_n - 1.
  int start = 1;
  while (start < stable_n)
  {
    int end = start + 1;
    while (end < stable_n && k_values[end] == k_values[start])
      ++end;
    groups.push_back({start, end, k_values[start]});
    start = end;
  }

  // Print second line: grouping with ⏟   characters (only for groups of size > 1).
  {
    StreamStateGuard guard(std::cout);
    std::cout << "       " << std::setw(spacing) << ' ';
    for (auto const& group : groups)
    {
      size_t group_size = group.end - group.start;

      if (group_size > 1)
      {
        // Print ⏟   characters for the group.
        //      n =  0   1   2   3   4   5   ⩾6
        //               `---.---´   `-.-´
        //               012345678   01234
        // brace_width =     9         5

        int brace_width   = (group_size - 1) * spacing + 1;
        int dot_pos       = (spacing * (group_size - 1)) / 2;     // Center the '.' under the middle of group.
        int left_padding  = (brace_width - 3) / 2;
        int right_padding = brace_width - 3 - left_padding;

        std::cout << std::setw(spacing) << '`' << std::setfill('-') << std::setw(left_padding) << '-' <<
          '.' << std::setw(right_padding) << '-' << "´" << std::setfill(' ');
      }
      else
      {
        // Single value, print spaces.
        std::cout << std::setw(spacing) << ' ';
      }
    }
    std::cout << std::endl;
  }

  {
    StreamStateGuard guard(std::cout);
    // Print third line: values and formulas.
    std::cout << "m =" << std::setw(3) << m << " :" << std::setw(spacing - 1) << values[0];

    for (auto const& group : groups)
    {
      size_t group_size = group.end - group.start;
      if (group_size > 1)
      {
        // Group with formula.
        std::string formula = std::to_string(group.k) + "+2n";

        // Calculate spacing needed for the group.
        //      n =  0   1   2   3   4   ⩾5
        // m =  2 :  6   8     10+2n    12+2n             <-- formula_width = length(10+2n) = 5.
        //                ===----^----                    plus_pos = 4
        //                   01234
        //                012345678901                    total_width = 12
        //
        int total_width   = group_size * spacing;
        int formula_width = formula.length();
        int plus_pos      = (spacing * (group_size - 1)) / 2;     // Center the '+' under the middle of group.
        int right_padding = total_width - spacing - plus_pos - 2; // 2 = length("2n").
        int left_padding  = total_width - formula_width - right_padding;

        std::cout << std::setw(left_padding + formula_width) << formula;
        std::cout << std::setw(right_padding) << "";
      }
      else
      {
        // Single value
        std::cout << std::setw(group_size * spacing) << values[group.start];
      }
    }
  }

  // Handle the ⩾N case (stable formula).
  std::cout << "  " << std::to_string(final_k) << "+2n" << std::endl;

  return final_k;
}

#ifdef EXAMPLE
// Example usage
int main()
{
  // First example: corrected vector
  std::vector<int> example1 = {6, 12, 14, 16, 20, 22, 26, 28, 30, 32, 34, 36, 38};
  print_formula_table(2, example1);

  std::cout << std::endl;

  // Second example from the prompt
  std::vector<int> example2 = {6, 8, 12, 14, 16, 20, 24, 26, 28, 30, 32, 34, 36, 38};
  print_formula_table(2, example2);

  return 0;
}
#endif
