#pragma once

#include "qtcreator01/ChessWindowBase.h"

namespace GUI {

class ChessWindow : public ChessWindowBase
{
 public:
  ChessWindow();
  ~ChessWindow();

 private slots:
  void on_actionQuit_triggered() override;
  void on_actionEdit_mode_triggered() override;
};

} // namespace GUI
