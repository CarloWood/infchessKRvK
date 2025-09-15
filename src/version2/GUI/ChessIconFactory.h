#pragma once

#include <gtkmm.h>
#include <array>

namespace GUI {
namespace menu_keys {

enum MenuEntryWithIconId
{
#if 0
  Export,
  Flip,
#endif
  number_of_custom_icons
};

enum MenuEntryWithoutIconId
{
#if 0
  ShowCandidates,
  ShowReachables,
  ShowAttacked,
  ShowDefendables,
  ShowDefendedBlack,
  ShowDefendedWhite,
  ShowMoves,
  PlacePieces,
  EditPosition,
  EditGame
#endif
};

std::string get_label(MenuEntryWithoutIconId menu_entry_id);

} // namespace menu_keys

class ChessIconFactory
{
 public:
  ChessIconFactory();

  char const* get_icon_name(menu_keys::MenuEntryWithIconId menu_entry_id) const { return m_icon_names[menu_entry_id]; }

 private:
  struct icon_info_st
  {
    char const* filepath;
    char const* icon_name;
    char const* label;
  };

 protected:
  static std::array<icon_info_st, menu_keys::number_of_custom_icons> s_icon_info;
  std::array<char const*, menu_keys::number_of_custom_icons> m_icon_names;
};

} // namespace GUI
