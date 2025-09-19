#include "ChessWindowBase.h"
#include "ui_ChessWindow.h"

namespace GUI {

ChessWindowBase::ChessWindowBase() : ui_(new Ui::ChessWindow)
{
  // Add qtcreator generated UI to this window.
  ui_->setupUi(this);
}

ChessWindowBase::~ChessWindowBase()
{
  // Destroy the UI.
  delete ui_;
}

void ChessWindowBase::on_actionQuit_triggered()
{
  on_actionQuit_triggered_virt();
}

void ChessWindowBase::on_actionEdit_mode_triggered()
{
  on_actionEdit_mode_triggered_virt();
}

void ChessWindowBase::on_actionSelect_All_triggered()
{
  on_actionSelect_All_triggered_virt();
}


} // namespace GUI
