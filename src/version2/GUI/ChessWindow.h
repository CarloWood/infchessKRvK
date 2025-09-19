#pragma once

#include "qtcreator01/ChessWindowBase.h"

namespace GUI {

class ChessWindow : public ChessWindowBase
{
 public:
  ChessWindow();
  ~ChessWindow();

 private slots:
  void on_actionQuit_triggered_virt() override;
  void on_actionEdit_mode_triggered_virt() override;
  void on_actionSelect_All_triggered_virt() override;
};

} // namespace GUI
