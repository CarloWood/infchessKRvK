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

class ChessWindowBase : public QMainWindow
{
  Q_OBJECT

public:
  ChessWindowBase();
  ~ChessWindowBase();

 private slots:
  virtual void on_actionQuit_triggered() { };
  virtual void on_actionEdit_mode_triggered() { };

 protected:
  Ui::ChessWindow* ui_;
};

} // namespace GUI

#endif // CHESSWINDOWBASE_H
