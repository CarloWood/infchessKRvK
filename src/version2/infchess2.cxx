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

  constexpr int max_number_of_tasks = 200;
  constexpr int min_number_of_parents_per_task = 100;

  AIThreadPool thread_pool(32);
  AIQueueHandle queue_handle = thread_pool.new_queue(max_number_of_tasks + 1);

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

    std::filesystem::path const prefix_directory_bak = "/opt/ext4/nvme0/infchessKRvK";
    std::filesystem::path const prefix_directory = "/opt/ext4/nvme2/infchessKRvK";
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
          Dout(dc::notice, "BEGIN white_to_move_parents immutable");
          int const number_of_parents = white_to_move_parents.size();
          // Round the number of tasks down, so we never have less than min_number_of_parents_per_task parents per task
          // (unless number_of_parents < min_number_of_parents_per_task).
          int const number_of_tasks = std::clamp(number_of_parents / min_number_of_parents_per_task, 1, max_number_of_tasks);
          // Round up, so that number_of_tasks tasks is sufficient to process all number_of_parents parents.
          int const number_of_parents_per_task = (number_of_parents + number_of_tasks - 1) / number_of_tasks;
          std::array<std::vector<Board>, max_number_of_tasks> task_black_to_move_parentss;
          // Distribute parents across exactly number_of_tasks tasks.
          // Each task gets either base or base+1 parents so the sum equals number_of_parents.
          int const base = number_of_parents / number_of_tasks;
          int const rem  = number_of_parents % number_of_tasks;
          std::cout << "Setting ply to " << ply << "/" << static_cast<uint32_t>(Classification::max_ply_upperbound) <<
            " for up to " << number_of_parents << " positions using " << number_of_tasks << " tasks." << std::endl;
          utils::threading::Gate until_all_tasks_finished;
          std::atomic_int unfinished_tasks = number_of_tasks;
          for (int task_n = 0; task_n < number_of_tasks; ++task_n)
          {
            int const parent_start = task_n * base + std::min(task_n, rem);
            int const parents_this_task = base + (task_n < rem ? 1 : 0);
            int const parent_end = std::min(parent_start + parents_this_task, number_of_parents);
            Dout(dc::notice, "PROCESSING task " << task_n << ": parent_start = " << parent_start << ", parent_end = " << parent_end);
            ASSERT(parents_this_task > 0);
            std::vector<Board>& task_black_to_move_parents = task_black_to_move_parentss[task_n];
            auto task = [parent_start, parent_end, ply,
                 &white_to_move_parents, &task_black_to_move_parents, &graph, &unfinished_tasks, &until_all_tasks_finished](){
              for (int parent = parent_start; parent < parent_end; ++parent)
              {
                Dout(dc::notice, "PROCESSING parent " << parent);
                Board white_to_move_board = white_to_move_parents[parent];
                // Access a non-const Info unique for this thread.
                Info& white_to_move_info = graph.get_info<white>(white_to_move_board);
                // All returned parents should be legal.
                ASSERT(white_to_move_info.classification().is_legal());
                ASSERT(white_to_move_info.classification().ply() == ply);
                white_to_move_info.white_to_move_set_minimum_ply_on_parents(white_to_move_board, graph, task_black_to_move_parents);
              }
              // If this was the last one, open the 'until_all_tasks_finished' gate.
              if (unfinished_tasks-- == 1)
                until_all_tasks_finished.open();
              // We're done.
              return false;
            };
            {
              // Get read access to AIThreadPool::m_queues.
              auto queues_access = thread_pool.queues_read_access();
              // Get a reference to one of the queues in m_queues.
              auto& queue = thread_pool.get_queue(queues_access, queue_handle);
              bool queue_full;
              {
                // Get producer accesses to this queue.
                auto queue_access = queue.producer_access();
                int length = queue_access.length();
                queue_full = length == queue.capacity();
                // I thought the queue was large enough?!
                ASSERT(!queue_full);
                if (!queue_full)
                {
                  // Place a lambda in the queue.
                  Dout(dc::notice, "Adding task " << task_n);
                  queue_access.move_in(task);
                }
              } // Release producer accesses, so another thread can write to this queue again.
              // This function must be called every time move_in was called
              // on a queue that was returned by thread_pool.get_queue.
              if (!queue_full) // Was move_in called?
                queue.notify_one();
            } // Release read access to AIThreadPool::m_queues so another thread can use AIThreadPool::new_queue again.
          }
          Dout(dc::notice, "Waiting for all tasks to finish...");
          until_all_tasks_finished.wait();
#ifdef CWDEBUG
          std::set<Board> black_to_move_parents_set;
#endif
          for (int task_n = 0; task_n < number_of_tasks; ++task_n)
          {
            std::vector<Board> const& task_black_to_move_parents = task_black_to_move_parentss[task_n];
            // Append task_black_to_move_parents to black_to_move_parents.
            std::ranges::copy(task_black_to_move_parents, std::back_inserter(black_to_move_parents));
#ifdef CWDEBUG
            // Check that there are no duplicates.
            for (auto&& board : task_black_to_move_parents)
            {
              auto ibp = black_to_move_parents_set.insert(board);
              ASSERT(ibp.second);
            }
#endif
          }
          Dout(dc::notice, "END white_to_move_parents immutable");
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
          int const number_of_parents = black_to_move_parents.size();
          // Round the number of tasks down, so we never have less than min_number_of_parents_per_task parents per task
          // (unless number_of_parents < min_number_of_parents_per_task).
          int const number_of_tasks = std::clamp(number_of_parents / min_number_of_parents_per_task, 1, max_number_of_tasks);
          // Round up, so that number_of_tasks tasks is sufficient to process all number_of_parents parents.
          int const number_of_parents_per_task = (number_of_parents + number_of_tasks - 1) / number_of_tasks;
          std::array<std::vector<Board>, max_number_of_tasks> task_white_to_move_parentss;
          // Distribute parents across exactly number_of_tasks tasks.
          // Each task gets either base or base+1 parents so the sum equals number_of_parents.
          int const base = number_of_parents / number_of_tasks;
          int const rem  = number_of_parents % number_of_tasks;
          std::cout << "Setting ply to " << ply << "/" << static_cast<uint32_t>(Classification::max_ply_upperbound) <<
            " for up to " << number_of_parents << " positions using " << number_of_tasks << " tasks." << std::endl;
          utils::threading::Gate until_all_tasks_finished;
          std::atomic_int unfinished_tasks = number_of_tasks;
          for (int task_n = 0; task_n < number_of_tasks; ++task_n)
          {
            int const parent_start = task_n * base + std::min(task_n, rem);
            int const parents_this_task = base + (task_n < rem ? 1 : 0);
            int const parent_end = std::min(parent_start + parents_this_task, number_of_parents);
            Dout(dc::notice, "PROCESSING task " << task_n << ": parent_start = " << parent_start << ", parent_end = " << parent_end);
            ASSERT(parents_this_task > 0);
            std::vector<Board>& task_white_to_move_parents = task_white_to_move_parentss[task_n];
            auto task = [parent_start, parent_end, ply,
                 &black_to_move_parents, &task_white_to_move_parents, &graph, &unfinished_tasks, &until_all_tasks_finished](){
              for (int parent = parent_start; parent < parent_end; ++parent)
              {
                Board black_to_move_board = black_to_move_parents[parent];
                // Access a non-const Info unique for this thread.
                Info& black_to_move_info = graph.get_info<black>(black_to_move_board);
                // All returned parents should be legal.
                ASSERT(black_to_move_info.classification().is_legal());
                ASSERT(black_to_move_info.classification().ply() == ply);
                black_to_move_info.black_to_move_set_maximum_ply_on_parents(black_to_move_board, graph, task_white_to_move_parents);
              }
              // If this was the last one, open the 'until_all_tasks_finished' gate.
              if (unfinished_tasks-- == 1)
                until_all_tasks_finished.open();
              // We're done.
              return false;
            };
            {
              // Get read access to AIThreadPool::m_queues.
              auto queues_access = thread_pool.queues_read_access();
              // Get a reference to one of the queues in m_queues.
              auto& queue = thread_pool.get_queue(queues_access, queue_handle);
              bool queue_full;
              {
                // Get producer accesses to this queue.
                auto queue_access = queue.producer_access();
                int length = queue_access.length();
                queue_full = length == queue.capacity();
                // I thought the queue was large enough?!
                ASSERT(!queue_full);
                if (!queue_full)
                {
                  // Place a lambda in the queue.
                  queue_access.move_in(task);
                }
              } // Release producer accesses, so another thread can write to this queue again.
              // This function must be called every time move_in was called
              // on a queue that was returned by thread_pool.get_queue.
              if (!queue_full) // Was move_in called?
                queue.notify_one();
            } // Release read access to AIThreadPool::m_queues so another thread can use AIThreadPool::new_queue again.
          }
          until_all_tasks_finished.wait();
          std::set<Board> white_to_move_parents_set;
          for (int task_n = 0; task_n < number_of_tasks; ++task_n)
          {
            std::vector<Board> const& task_white_to_move_parents = task_white_to_move_parentss[task_n];
            // Append task_white_to_move_parents to white_to_move_parents.
            std::ranges::copy(task_white_to_move_parents, std::back_inserter(white_to_move_parents));
#ifdef CWDEBUG
            for (auto&& board : task_white_to_move_parents)
            {
              auto ibp = white_to_move_parents_set.insert(board);
              if (!ibp.second)
              {
                NAMESPACE_DEBUG::Mark m;
                Dout(dc::notice, "task_n: " << task_n << "; the board " << board << " was already added!");
                Dout(dc::notice, "number_of_tasks = " << number_of_tasks);
                std::array<char, 12> buf;
                for (int task_n2 = 0; task_n2 < number_of_tasks; ++task_n2)
                {
                  std::vector<Board> const& task_white_to_move_parents2 = task_white_to_move_parentss[task_n2];
                  Dout(dc::notice, "parents from task " << task_n2 << ": ");
                  NAMESPACE_DEBUG::Mark m2(utils::itoa(buf, task_n2));
                  for (Board b : task_white_to_move_parents2)
                    Dout(dc::notice, b);
                }
              }
              ASSERT(ibp.second);
            }
#endif
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
    std::cout << "Data written to " << data_filename << std::endl;

    Graph const g(prefix_directory_bak, true, true);

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
    std::cerr << "Fatal error: " << error << std::endl;
  }
}
