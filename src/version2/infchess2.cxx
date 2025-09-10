#include "sys.h"
#include "Graph.h"
#include "../parse_move.h"
#include "utils/AIAlert.h"
#include "utils/debug_ostream_operators.h"
#include "utils/threading/Gate.h"
#include "utils/itoa.h"
#include "threadpool/AIThreadPool.h"
#include <bitset>
#include <set>
#include "debug.h"

int main()
{
  Debug(NAMESPACE_DEBUG::init());

  Dout(dc::notice, "sizeof(Info) = " << sizeof(Info));

  // Get the size of the board.
  int const board_size_x = Size::board::x;
  int const board_size_y = Size::board::y;
  Dout(dc::notice, "Board size: " << board_size_x << "x" << board_size_y);

  // Get the size of a block.
  int const block_size_x = Size::block::x;
  int const block_size_y = Size::block::y;
  Dout(dc::notice, "Block size: " << block_size_x << "x" << block_size_y);
  Dout(dc::notice, "Classification::max_ply_upperbound = " << static_cast<uint32_t>(Classification::max_ply_upperbound));
  Dout(dc::notice, "Classification::ply_bits = " << static_cast<uint32_t>(Classification::ply_bits));

  try
  {
    // Construct the initial graph with all positions that are already mate.
    auto start = std::chrono::high_resolution_clock::now();

    std::filesystem::path const prefix_directory = "/opt/btrfs/infchessKRvK";
    std::filesystem::path const prefix_directory_bak = "/opt/ext4/infchessKRvK";
    std::filesystem::path const data_directory = Graph::data_directory(prefix_directory);
    std::filesystem::path const data_filename = Graph::data_filename(prefix_directory);
    bool const file_exists = std::filesystem::exists(data_filename);
    if (!file_exists)
    {
      std::filesystem::path const directory_path = data_filename.parent_path();
      bool const dir_exists = std::filesystem::exists(directory_path);
      if (!dir_exists)
      {
        Dout(dc::notice, "File does not exist; creating directory " << data_directory);
        std::filesystem::create_directories(data_directory);
      }
      else
        Dout(dc::notice, "File does not exist.");
    }
    else
      Dout(dc::notice, "Using existing file " << data_filename << ".");

    // Only a new file is zero initialized.
    Graph graph(prefix_directory, file_exists);
    std::vector<Board> already_mate;

    if (!file_exists)
    {
      int total_positions;
      int draw_positions = 0;
      int black_in_check_positions = 0;
      int mate_positions = 0;
      int stalemate_positions = 0;

      // Generate all possible positions.
      graph.classify();

      Dout(dc::notice, "Number of partitions: white: " << graph.white_to_move_infos().size() <<
          ", black: " << graph.black_to_move_infos().size());

      // Black to move positions.
      Dout(dc::notice, "Processing Black-To-Move-Partitions...");
      {
        auto const& black_to_move_infos = graph.black_to_move_infos();
        total_positions = 0;
        for (Partition current_partition = black_to_move_infos.ibegin();
            current_partition != black_to_move_infos.iend(); ++current_partition)
        {
          Dout(dc::notice, "  partition " << static_cast<PartitionIndex>(current_partition));
          for (PartitionElement current_partition_element = black_to_move_infos[current_partition].ibegin();
              current_partition_element != black_to_move_infos[current_partition].iend(); ++current_partition_element)
          {
            Info const& info = graph.get_info<black>(current_partition, current_partition_element);
            Classification const& pc = info.classification();
            if (!pc.is_legal())
              continue;
            ++total_positions;
            if (pc.is_draw())
              ++draw_positions;
            if (pc.is_check())
              ++black_in_check_positions;
            if (pc.is_mate())
            {
              ++mate_positions;
              already_mate.emplace_back(current_partition, current_partition_element);
            }
            if (pc.is_stalemate())
              ++stalemate_positions;
          }
        }
      }
      // White to move positions.
      Dout(dc::notice, "Processing White-To-Move-Partitions...");
      {
        auto const& white_to_move_infos = graph.white_to_move_infos();
        for (Graph::infos_type::index_type current_partition = white_to_move_infos.ibegin();
            current_partition != white_to_move_infos.iend(); ++current_partition)
        {
          Dout(dc::notice, "  partition " << current_partition);
          for (Graph::infos_type::value_type::index_type current_partition_element = white_to_move_infos[current_partition].ibegin();
              current_partition_element != white_to_move_infos[current_partition].iend(); ++current_partition_element)
          {
            Info const& info = graph.get_info<white>(current_partition, current_partition_element);
            Classification const& pc = info.classification();
            if (!pc.is_legal())
              continue;
            ++total_positions;
            if (pc.is_draw())
              ++draw_positions;
          }
        }
      }

      std::cout << "Version 2:" << std::endl;
      std::cout << "Total legal positions: " << total_positions << std::endl;
      std::cout << "Draw positions: " << draw_positions << std::endl;
      std::cout << "Black in check positions: " << black_in_check_positions << std::endl;
      std::cout << "Mate positions: " << mate_positions << std::endl;
      std::cout << "Stalemate positions: " << stalemate_positions << std::endl;
    }
    else
    {
      // Generate all positions that are already mate.
      for (int x = 0; x < Size::board_size_x; ++x)
      {
        for (int wx = 0; wx < Size::board_size_x; ++wx)
        {
          if (std::abs(wx - x) > 1)
          {
            already_mate.emplace_back(BlackKingSquare{x, 0}, WhiteKingSquare{x, 2}, WhiteRookSquare{wx, 0});
            if (x == 0)
              already_mate.emplace_back(BlackKingSquare{x, 0}, WhiteKingSquare{1, 2}, WhiteRookSquare{wx, 0});
          }
        }
      }
      for (int y = 0; y < Size::board_size_y; ++y)
      {
        for (int wy = 0; wy < Size::board_size_y; ++wy)
        {
          if (std::abs(wy - y) > 1)
          {
            already_mate.emplace_back(BlackKingSquare{0, y}, WhiteKingSquare{2, y}, WhiteRookSquare{0, wy});
            if (y == 0)
              already_mate.emplace_back(BlackKingSquare{0, y}, WhiteKingSquare{2, 1}, WhiteRookSquare{0, wy});
          }
        }
      }
      Dout(dc::notice, "Number of mate positions: " << already_mate.size());
    }

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    std::cout << "Execution time: " << (duration.count() / 1000000.0) << " seconds\n";

    if (file_exists)
    {
      Dout(dc::notice|continued_cf|flush_cf, "Resetting ply to unknown...");
      // Reset all ply to zero so we can test the code below.
      //FIXME: remove this.
      graph.reset_ply();
      Dout(dc::finish, " done");
    }

#if 0
    {
      Board board({3, 0}, {1, 1}, {1, 0});
      Color to_move(black);

      board.utf8art(std::cout, to_move);

      Board::neighbors_type parents;
      int number_of_parents = board.generate_neighbors<Board::parents, white>(parents);
      ASSERT(number_of_parents == 0);

      return 0;
    }
#endif

    // Run over all positions that are already mate (as per the classification)
    // and mark all position that can reach those as mate in 1 ply.
    std::vector<Board> white_to_move_parents;
    std::cout << "Setting ply to 0 for " << already_mate.size() << " positions." << std::endl;
    for (Board current_board : already_mate)
    {
//      current_board.debug_utf8art(DEBUGCHANNELS::dc::notice);
      Info& black_to_move_info = graph.get_info<black>(current_board);
      black_to_move_info.classification().set_mate_in_ply(0);
      black_to_move_info.black_to_move_set_maximum_ply_on_parents(current_board, graph, white_to_move_parents);
    }

    // Measure the time it takes to generate the graph.
    start = std::chrono::high_resolution_clock::now();

    Board initial_position;
    Color initial_to_move;
    {
      // white_to_move_parents are mate in `ply` moves.
      int ply = 0;
      while (!white_to_move_parents.empty())
      {
        ++ply;
        std::vector<Board> black_to_move_parents;
        // Run over all positions that are mate in an odd number of ply.
        {
          std::cout << "Setting ply to " << ply << " for up to " << white_to_move_parents.size() << " positions." << std::endl;
          for (Board white_to_move_board : white_to_move_parents)
          {
            Info& white_to_move_info = graph.get_info<white>(white_to_move_board);
            // All returned parents should be legal.
            ASSERT(white_to_move_info.classification().is_legal());
            ASSERT(white_to_move_info.classification().ply() == ply);
            size_t s = black_to_move_parents.size();
            white_to_move_info.white_to_move_set_minimum_ply_on_parents(white_to_move_board, graph, black_to_move_parents);
          }
        }

        if (!black_to_move_parents.empty())
        {
          initial_position = black_to_move_parents[0];
          initial_to_move = black;
        }
        else
          break;

        ++ply;
        white_to_move_parents.clear();
        // Run over all positions that are mate in an even number of ply.
        {
          std::cout << "Setting ply to " << ply << " for up to " << black_to_move_parents.size() << " positions." << std::endl;
          for (Board black_to_move_board : black_to_move_parents)
          {
            Info& black_to_move_info = graph.get_info<black>(black_to_move_board);
            // All returned parents should be legal.
            ASSERT(black_to_move_info.classification().is_legal());
            ASSERT(black_to_move_info.classification().ply() == ply);
            size_t s = white_to_move_parents.size();
            black_to_move_info.black_to_move_set_maximum_ply_on_parents(black_to_move_board, graph, white_to_move_parents);
          }
        }

        if (!white_to_move_parents.empty())
        {
          initial_position = white_to_move_parents[0];
          initial_to_move = white;
        }
      }
      std::cout << "max ply = " << ply << std::endl;
    }

    end = std::chrono::high_resolution_clock::now();
    duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    std::cout << "Execution time: " << (duration.count() / 1000000.0) << " seconds\n";

    Graph const g(prefix_directory_bak, true);

#if CW_DEBUG
    std::cout << "Testing contents for black to move..." << std::endl;
    auto const& black_to_move_infos = graph.black_to_move_infos();
    for (Partition current_partition = black_to_move_infos.ibegin();
        current_partition != black_to_move_infos.iend(); ++current_partition)
    {
      for (PartitionElement current_partition_element = black_to_move_infos[current_partition].ibegin();
          current_partition_element != black_to_move_infos[current_partition].iend(); ++current_partition_element)
      {
        Info const& info1 = graph.get_info<black>(current_partition, current_partition_element);
        Info const& info2 = g.get_info<black>(current_partition, current_partition_element);
        ASSERT(info1.classification() == info2.classification());

#if 0
        Dout(dc::notice, "Board for partition " << static_cast<PartitionIndex>(current_partition) <<
            " / element " << static_cast<InfoIndex>(current_partition_element) << " :");
        Board current_board(current_partition, current_partition_element);
        current_board.debug_utf8art(DEBUGCHANNELS::dc::notice);
        Dout(dc::notice, "Mate in " << info2.classification().ply() << " ply.");
#endif
      }
    }
    std::cout << "Testing contents for white to move..." << std::endl;
    auto const& white_to_move_infos = graph.white_to_move_infos();
    for (Partition current_partition = white_to_move_infos.ibegin();
        current_partition != white_to_move_infos.iend(); ++current_partition)
    {
      for (PartitionElement current_partition_element = white_to_move_infos[current_partition].ibegin();
          current_partition_element != white_to_move_infos[current_partition].iend(); ++current_partition_element)
      {
        Info const& info1 = graph.get_info<white>(current_partition, current_partition_element);
        Info const& info2 = g.get_info<white>(current_partition, current_partition_element);
        ASSERT(info1.classification() == info2.classification());

#if 0
        Dout(dc::notice, "Board for partition " << static_cast<PartitionIndex>(current_partition) <<
            " / element " << static_cast<InfoIndex>(current_partition_element) << " :");
        Board current_board(current_partition, current_partition_element);
        current_board.debug_utf8art(DEBUGCHANNELS::dc::notice);
        Dout(dc::notice, "Mate in " << info2.classification().ply() << " ply.");
#endif
      }
    }
#endif

    //Board::generate_neighbors_testsuite(graph);

#if 0
    Board board = initial_position;
    Color to_move = initial_to_move;
    for (;;)
    {
      Info const& info = to_move == black ? graph.get_info<black>(board) : graph.get_info<white>(board);
      int ply = info.classification().ply();

      std::cout << "\nCurrent position (" << to_move << " to move; mate in " << ply << " ply):\n";
      board.utf8art(std::cout, to_move);

      if (info.classification().is_mate())
        break;

      // Get all possible child positions.
      Board::neighbors_type child_positions;
      int number_of_children =
        to_move == black ? board.generate_neighbors<Board::children, black>(child_positions)
                         : board.generate_neighbors<Board::children, white>(child_positions);

      // Print all optimal moves:
      Dout(dc::notice|continued_cf, "Possible optimal moves: ");
      char const* separator = "";
      for (int i = 0; i < number_of_children; ++i)
      {
        Board b = child_positions[i];
        Info const& child_info = to_move == black ? graph.get_info<black>(b) : graph.get_info<white>(b);
        int child_ply = child_info.classification().ply();
        if (child_ply == ply - 1)
        {
          Dout(dc::continued, separator << board.get_move(b));
          separator = ", ";
        }
      }
      Dout(dc::finish, ".");

      std::cin.get();

#if 1
      bool first_time = true;
      for (int i = 0; i < number_of_children; ++i)
      {
        Board b = child_positions[i];
        Info const& child_info = to_move == black ? graph.get_info<black>(b) : graph.get_info<white>(b);
        int child_ply = child_info.classification().ply();
        if (child_ply == ply - 1 && (first_time || b > board))
        {
          first_time = false;
          board = b;
        }
      }
#else
      // Get a move from the user.
      for (;;)
      {
        auto [piece, x, y] = parse_move(to_move, board_size_x, board_size_y);
        Board new_board(board);
        if (to_move == white)
        {
          if (piece == 'R')
            new_board.set_white_rook_square({x, y});
          else
            new_board.set_white_king_square({x, y});
        }
        else if (piece == 'K')
          new_board.set_black_king_square({x, y});

        try
        {
          auto end = child_positions.cbegin() + number_of_children;
          auto found = std::find(child_positions.cbegin(), end, new_board);
          if (found == end)
            throw std::invalid_argument("Illegal move");
        }
        catch (std::exception const& e)
        {
          std::cerr << "Error: " << e.what() << std::endl;
          continue;
        }

        board = new_board;
        break;
      }
#endif

      to_move = to_move.opponent();
    }
#endif
  }
  catch (AIAlert::Error const& error)
  {
    Dout(dc::notice, "Fatal error: " << error);
  }
}
