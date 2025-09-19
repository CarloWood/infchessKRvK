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

} // namespace GUI
