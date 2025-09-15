#pragma once

#include <gtkmm.h>

namespace GUI {

class ChessWindow;
class ChessMenuBar;

class ChessApplication : public Gtk::Application
{
 private:
  ChessWindow* main_window_;

 protected:
  ChessApplication();
  ~ChessApplication() override;

 public:
  static Glib::RefPtr<ChessApplication> create();

  void append_menu_entries(ChessMenuBar* menubar);

 protected:
  void on_startup() override;
  void on_activate() override;

 private:
  ChessWindow* create_window();

  void on_menu_File_QUIT();

  void on_window_hide(Gtk::Window* window);
};

} // namespace GUI
