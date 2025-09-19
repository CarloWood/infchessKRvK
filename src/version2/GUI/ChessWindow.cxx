#include "sys.h"
#include "ChessWindow.h"
#include "ChessApplication.h"
#include <ui_ChessWindow.h>
#include <QStyle>
#include "debug.h"

namespace GUI {

ChessWindow::ChessWindow()
{
  DoutEntering(dc::notice, "ChessWindow::ChessWindow() [" << (void*)this << "]");

  // Set an icon on the Quit action.
  QIcon quitIcon = QIcon::fromTheme("application-exit");
  if (quitIcon.isNull())
  {
    char const* const QT_QPA_PLATFORMTHEME = getenv("QT_QPA_PLATFORMTHEME");
    std::string status = "currently not set";
    if (QT_QPA_PLATFORMTHEME)
      status = "currently \"" + std::string(QT_QPA_PLATFORMTHEME) + "\"";
    std::cerr << "Warning: the current theme (\"" << QIcon::themeName().toUtf8().constData() << "\") does not support the freedesktop Icon "
      "Naming Specification. Perhaps you need to set a correct value of the QT_QPA_PLATFORMTHEME environment variable (" << status << ")." <<
      std::endl;
    quitIcon = qApp->style()->standardIcon(QStyle::SP_DialogCloseButton);
  }
  ui_->actionQuit->setIcon(quitIcon);

  // Make graphicsView fill as much of the main window as possible.
  setCentralWidget(ui_->graphicsView);
}

ChessWindow::~ChessWindow()
{
  DoutEntering(dc::notice, "ChessWindow::~ChessWindow() [" << (void*)this << "]");
}

void ChessWindow::on_actionQuit_triggered()
{
  DoutEntering(dc::notice, "ChessWindow::on_actionQuit_triggered()");
}

void ChessWindow::on_actionEdit_mode_triggered()
{
  DoutEntering(dc::notice, "ChessWindow::on_actionEdit_mode_triggered()");
}

} // namespace GUI
