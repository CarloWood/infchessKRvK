#include "sys.h"
#include "Graph.h"
#include "utils/AIAlert.h"
#include "utils/debug_ostream_operators.h"
#include "GUI/ChessApplication.h"
#include "GUI/ChessWindow.h"

int main(int argc, char* argv[])
{
  Debug(NAMESPACE_DEBUG::init());
  Dout(dc::notice, "Entering main() ");

  // Get the size of the board.
  int const board_size_x = Size::board::x;
  int const board_size_y = Size::board::y;
  Dout(dc::notice, "Board size: " << board_size_x << "x" << board_size_y);

  // Get the size of a block.
  int const block_size_x = Size::block::x;
  int const block_size_y = Size::block::y;
  Dout(dc::notice, "Block size: " << block_size_x << "x" << block_size_y);

  // Create the QApplication object.
  GUI::ChessApplication application(argc, argv);

  // Create and show the QMainWindow object.
  GUI::ChessWindow main_window;
  main_window.show();

  // Run application.
  int exit_status = application.exec();

  // The window was closed.
  Dout(dc::notice, "Leaving main() with exit status " << exit_status);
  return exit_status;

#if 0
  try
  {
    std::filesystem::path const prefix_directory = "/opt/ext4/nvme1/infchessKRvK";
    std::filesystem::path const data_directory = Graph::data_directory(prefix_directory);
    std::filesystem::path const data_filename = Graph::data_filename(prefix_directory);
    bool const file_exists = std::filesystem::exists(data_filename);

    if (!file_exists)
    {
      std::cerr << "The file " << data_filename << " does not exist!" << std::endl;
      return 1;
    }

    Graph const graph(prefix_directory, true);
  }
  catch (AIAlert::Error const& error)
  {
    std::cerr << "Fatal error: " << error << std::endl;
  }
#endif
}
