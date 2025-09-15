#include "sys.h"
#include "ChessMenuBar.h"
#include "ChessWindow.h"
#include "ChessApplication.h"

namespace GUI {

using namespace menu_keys;

//static
char const* ChessMenuBar::top_entry_label(TopEntries top_entry)
{
  switch (top_entry)
  {
    case File:
      return "File";
  }
  return "";
}

ChessMenuBar::ChessMenuBar(ChessWindow* main_window)
{
  // Prepare top level entries.
  for (int tl = 0; tl < number_of_top_entries; ++tl)
  {
    TopEntries te = static_cast<TopEntries>(tl);
    auto tl_item = Gtk::manage(new Gtk::ImageMenuItem(top_entry_label(te), true));
    append(*tl_item);
    m_submenus[te] = Gtk::manage(new Gtk::Menu);
    tl_item->set_submenu(*m_submenus[te]);
  }

  main_window->append_menu_entries(this);
  main_window->application().append_menu_entries(this);
  main_window->chessboard_widget().append_menu_entries(this);
}

} // namespace GUI
