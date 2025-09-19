#ifndef CHESSWINDOWBASE_H
#define CHESSWINDOWBASE_H

#pragma once

#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui {
class ChessWindow;
}
QT_END_NAMESPACE

namespace GUI {

// This is only defined for the real application.
#ifdef HAVE_UTILS_CONFIG_H
#define DECLARE_VIRT(on_action) virtual void on_action##_virt() = 0;
#else
// Use a dummy if this is compiled from qtcreator.
#define DECLARE_VIRT(on_action) void on_action##_virt() { }
#endif

class ChessWindowBase : public QMainWindow
{
  Q_OBJECT

public:
  ChessWindowBase();
  ~ChessWindowBase();

 private slots:
  void on_actionQuit_triggered();
  void on_actionEdit_mode_triggered();
  void on_actionSelect_All_triggered();

 protected:
  Ui::ChessWindow* ui_;

 private:
  DECLARE_VIRT(on_actionQuit_triggered);
  DECLARE_VIRT(on_actionEdit_mode_triggered);
  DECLARE_VIRT(on_actionSelect_All_triggered);
};

} // namespace GUI

#endif // CHESSWINDOWBASE_H
