#include "sys.h"
#include "Classification.h"
#include <iostream>

#ifdef CWDEBUG
void Classification::print_on(std::ostream& os) const
{
  os << "{bits:" << static_cast<int>(bits_) << ", mate_in_moves:" << mate_in_moves_ << '}';
}
#endif
