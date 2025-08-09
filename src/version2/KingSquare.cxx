#include "sys.h"
#include "KingSquare.h"

#ifdef CWDEBUG
void KingSquare::print_on(std::ostream& os) const
{
  os << '{';
  os << "x_coord:" << x_coord() <<
      ", y_coord:" << y_coord() <<
      ", block_index:" << block_index() <<
      ", block_square.x:" << block_square().x_coord() <<
      ", block_square.y:" << block_square().y_coord();
  os << '}';
}
#endif
