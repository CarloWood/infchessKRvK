#include "sys.h"
#include "BlockIndex.h"
#include <iostream>

#ifdef CWDEBUG
void BlockIndex::print_on(std::ostream& os) const
{
  os << '{';
  os << "index:" << (int)index_ <<
      ", x_coord:" << (x_coord() / Size::block::x) <<
      ", y_coord:" << (y_coord() / Size::block::y);
  os << '}';
}
#endif
