#include "sys.h"
#include "ChessApplication.h"
#include "ChessWindow.h"
#include "ChessMenuBar.h"
#include "utils/AIAlert.h"
#include "debug.h"

namespace GUI {

PreAppInit::PreAppInit()
{
  // Nothing yet.
}

#if 0
ChessApplication::ChessApplication(int& argc, char** argv) : QApplication(argc, argv)
{
  DoutEntering(dc::notice, "ChessApplication::ChessApplication(" << argc << ", " << argv << ")");

  QCoreApplication::setApplicationName(config::PROJECT_NAME);
  QCoreApplication::setApplicationVersion(config::PROJECT_VERSION);
  QCoreApplication::setOrganizationName("CarloWood");
  QCoreApplication::setOrganizationDomain("alinoe.com");

  // Set Application Attribute 'DontCreateNativeWidgetSiblings' to stop Qt from promoting alien
  // child widgets (like the scroll bars and coordinates) to native windows (the board view);
  // we know that they do not overlap, so that is not necessary.
  setAttribute(Qt::AA_DontCreateNativeWidgetSiblings);
}
#endif

ChessApplication::~ChessApplication()
{
  DoutEntering(dc::notice, "ChessApplication::~ChessApplication()");
}

#if 0
void ChessApplication::on_startup()
{
  DoutEntering(dc::notice, "ChessApplication::on_startup()");

  // Call the base class's implementation.
  Gtk::Application::on_startup();

  // Apply CSS to remove rounded corners from menus
  auto css_provider = Gtk::CssProvider::create();
  css_provider->load_from_data(R"(
    popover.menu,
    popover contents,
    menu,
    .menu {
      border-radius: 0;
      box-shadow: none;
    }

    popover.menu > arrow,
    popover > arrow {
      border-radius: 0;
    }
  )");

  auto display = Gdk::Display::get_default();
  Gtk::StyleContext::add_provider_for_display(
    display, css_provider, GTK_STYLE_PROVIDER_PRIORITY_APPLICATION
  );

  // FIXME(?) This was done somewhere else...
  // File menu:
  //add_action("quit", sigc::mem_fun(*this, &ChessApplication::on_menu_File_quit));
  add_action("quit", [this](){ on_menu_File_quit(); });

  // Set accelerator keys:
  set_accel_for_action("app.quit", "<Primary>q");

  m_refBuilder = Gtk::Builder::create();

  //Layout the actions in a menubar and a menu:
  Glib::ustring ui_info =
    "<interface>"
    "  <!-- menubar -->"
    "  <menu id='menu-infchessKRvK'>"
    "    <submenu>"
    "      <attribute name='label' translatable='yes'>_File</attribute>"
    "      <section>"
    "        <item>"
    "          <attribute name='label' translatable='yes'>_Quit</attribute>"
    "          <attribute name='action'>app.quit</attribute>"
    "          <attribute name='accel'>&lt;Primary&gt;q</attribute>"
    "        </item>"
    "      </section>"
    "    </submenu>"
    "  </menu>"
    "</interface>";

  try
  {
    m_refBuilder->add_from_string(ui_info);
  }
  catch (Glib::Error const& ex)
  {
    THROW_ALERT("Building menus failed: [WHAT]", AIArgs("[WHAT]", ex.what()));
  }

  // Get the menubar and the app menu, and add them to the application:
  auto gmenu = m_refBuilder->get_object<Gio::Menu>("menu-infchessKRvK");
  if (!gmenu)
  {
    g_warning("GMenu not found");
  }
  else
  {
    set_menubar(gmenu);
  }
}

void ChessApplication::on_activate()
{
  DoutEntering(dc::notice, "ChessApplication::on_activate()");

  // The application has been started, create and show the main window.
  main_window_ = create_main_window();
}

void ChessApplication::on_shutdown()
{
  DoutEntering(dc::notice, "ChessApplication::on_shutdown()");

  // Close any remaining open windows.
  std::vector<Gtk::Window*> windows = get_windows();
  Dout(dc::notice, "The application has " << windows.size() << " windows.");
  for (auto window : windows)
    window->hide();

  // Call the base class on_shutdown.
  Gtk::Application::on_shutdown();
}

ChessWindow* ChessApplication::create_main_window()
{
  DoutEntering(dc::notice, "ChessApplication::create_main_window()");

  ChessWindow* main_window = new ChessWindow(this);

#if 0
  // Make sure that the application runs as long this window is still open.
  add_window(*main_window);
  std::vector<Gtk::Window*> windows = get_windows();
  Dout(dc::notice, "The application has " << windows.size() << " windows.");
  ASSERT(G_IS_OBJECT(main_window->gobj()));

  // Delete the window when it is hidden.
  main_window->signal_hide().connect([this](){ on_main_window_hide(); });

  //main_window->show_all();
  main_window->set_show_menubar();
  main_window->set_visible(true);
#endif
  return main_window;
}

void ChessApplication::on_main_window_hide()
{
  DoutEntering(dc::notice, "ChessApplication::on_main_window_hide()");

  // Hiding the window removed it from the application.
  // Set our pointer to nullptr, just to be sure we won't access it again.
  // Delete the window.
  {
    delete main_window_;
    main_window_ = nullptr;
  }

  // Now that the main window is gone, just quit the application (even if there are other windows still open).
  quit();

  Dout(dc::notice, "Leaving ChessApplication::on_window_hide()");
}

void ChessApplication::on_menu_File_quit()
{
  DoutEntering(dc::notice, "ChessApplication::on_menu_File_quit()");

  // Gio::Application::quit() will make Gio::Application::run() return.
  quit();

  Dout(dc::notice, "Leaving ChessApplication::on_menu_File_quit()");
}
#endif

} // namespace GUI
