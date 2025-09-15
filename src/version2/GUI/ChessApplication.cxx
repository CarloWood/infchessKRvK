#include "sys.h"
#include "ChessApplication.h"
#include "ChessWindow.h"
#include "ChessMenuBar.h"
#include "debug.h"

namespace GUI {

ChessApplication::ChessApplication() : Gtk::Application("com.github.CarloWood.infchessKRvK")
{
  DoutEntering(dc::notice, "ChessApplication::ChessApplication()");
  Glib::set_application_name("K+R vs K");
}

ChessApplication::~ChessApplication()
{
  Dout(dc::notice, "Calling ChessApplication::~ChessApplication()");
}

Glib::RefPtr<ChessApplication> ChessApplication::create()
{
  return Glib::make_refptr_for_instance(new ChessApplication);
}

} // namespace GUI
