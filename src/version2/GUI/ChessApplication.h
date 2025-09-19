#pragma once

#include <QApplication>

namespace GUI {

class ChessApplication;

class PreAppInit
{
  friend class ChessApplication;
  PreAppInit();
};

class ChessApplication : private PreAppInit, public QApplication
{
 public:
  // Inherit all constructors.
  using QApplication::QApplication;

  // Override just the constructor that I am interested in.
//  ChessApplication(int& argc, char** argv);
  ~ChessApplication() override;
};

} // namespace GUI
