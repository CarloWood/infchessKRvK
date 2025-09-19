#include <QApplication>
#include "ChessWindowBase.h"

int main(int argc, char *argv[])
{
  QApplication app(argc, argv);
  GUI::ChessWindowBase window;
  window.show();
  return app.exec();
}
