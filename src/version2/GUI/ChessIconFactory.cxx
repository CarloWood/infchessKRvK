#include "sys.h"
#include "ChessIconFactory.h"
#include "debug.h"

namespace GUI {

//static
std::array<ChessIconFactory::icon_info_st, menu_keys::number_of_custom_icons> ChessIconFactory::s_icon_info = {{
#if 0
  { "pixmaps/export.png", "export", "Export" },
  { "pixmaps/flip.png", "flip", "Flip" }
#endif
}};

namespace menu_keys {

std::string get_label(MenuEntryWithoutIconId menu_entry_id)
{
#if 0
  switch (menu_entry_id)
  {
    case ShowCandidates:
      return "Show candidates";
    case ShowReachables:
      return "Show reachables";
    case ShowAttacked:
      return "Show attacked";
    case ShowDefendables:
      return "Show defendables";
    case ShowDefendedBlack:
      return "Show defended black";
    case ShowDefendedWhite:
      return "Show defended white";
    case ShowMoves:
      return "Show moves";
    case PlacePieces:
      return "Place pieces";
    case EditPosition:
      return "Edit position";
    case EditGame:
      return "Edit game";
  }
#endif
  return "Unknown label";
}

} // namespace menukeys

ChessIconFactory::ChessIconFactory()
{
  Glib::RefPtr<Gtk::IconFactory> factory = Gtk::IconFactory::create();
  for (int i = 0; i < s_icon_info.size(); ++i)
  {
    Gtk::IconSource source;
    try
    {
      Glib::RefPtr<Gdk::Pixbuf> const& pixbuf = Gdk::Pixbuf::create_from_file(s_icon_info[i].filepath);
      source.set_pixbuf(pixbuf);
    }
    catch(Glib::Exception const& ex)
    {
      DoutFatal(dc::fatal, ex.what());
    }
    source.set_size(Gtk::ICON_SIZE_SMALL_TOOLBAR);
    source.set_size_wildcarded(); // Icon may be scaled.
    Glib::RefPtr<Gtk::IconSet> icon_set = Gtk::IconSet::create();
    icon_set->add_source(source);
    char const* const icon_name(s_icon_info[i].icon_name);
    factory->add(icon_name, icon_set);
    Gtk::Stock::add(Gtk::StockItem(icon_name, s_icon_info[i].label));
  }
  factory->add_default();
  for (int i = 0; i < s_icon_info.size(); ++i)
    m_icon_names[i] = s_icon_info[i].icon_name;
}

} // namespace GUI
