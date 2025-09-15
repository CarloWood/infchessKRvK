#include "sys.h"
#include "utils/AIAlert.h"
#include "utils/debug_ostream_operators.h"
#include "utils/at_scope_end.h"
#include "Graph.h"
#include "Uncompressed.h"
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

void handle_client(int client_fd, const Graph& graph)
{
  Dout(dc::notice, "New client connected, fd=" << client_fd);

  char buffer[4096];
  ssize_t bytes_received;

  while ((bytes_received = recv(client_fd, buffer, sizeof(buffer), 0)) > 0)
  {
    Dout(dc::notice, "Received " << bytes_received << " bytes from client");

    // Check if we received a multiple of sizeof(Board).
    ASSERT(bytes_received % sizeof(UncompressedBoard) == 0);

    int num_boards = bytes_received / sizeof(UncompressedBoard);
    Dout(dc::notice, "Processing " << num_boards << " board(s)");

    // Process each board and collect UncompressedInfo objects.
    std::vector<UncompressedInfo> data;
    data.reserve(num_boards * 2); // black and white to move for each board.

    for (int i = 0; i < num_boards; ++i)
    {
      UncompressedBoard uncompressed_board;
      std::memcpy(&uncompressed_board, buffer + i * sizeof(UncompressedBoard), sizeof(UncompressedBoard));
      Board board({uncompressed_board.bkx, uncompressed_board.bky},
          {uncompressed_board.wkx, uncompressed_board.wky}, {uncompressed_board.wrx, uncompressed_board.wry});

      Dout(dc::notice, "Processing board " << i << ": " << board);

      Info const& black_to_move_info = graph.get_info<black>(board);
      Info const& white_to_move_info = graph.get_info<white>(board);

      UncompressedInfo black_to_move_uncompressed_info{black_to_move_info.classification().ply_encoded(), black_to_move_info.classification().bits(), black_to_move_info.number_of_children()};
      UncompressedInfo white_to_move_uncompressed_info{white_to_move_info.classification().ply_encoded(), white_to_move_info.classification().bits(), white_to_move_info.number_of_children()};

      data.push_back(black_to_move_uncompressed_info);
      data.push_back(white_to_move_uncompressed_info);

      Dout(dc::notice, "Board " << i << " classifications: black=" << black_to_move_info << ", white=" << white_to_move_info);
    }

    // Send back all classifications.
    size_t response_size = data.size() * sizeof(UncompressedInfo);
    ssize_t bytes_sent = send(client_fd, data.data(), response_size, 0);

    ASSERT(bytes_sent == static_cast<ssize_t>(response_size));
    Dout(dc::notice, "Sent " << bytes_sent << " bytes (" << data.size() << " classifications)");
  }

  if (bytes_received == -1)
    Dout(dc::warning, "recv() error: " << strerror(errno));
  else
    Dout(dc::notice, "Client disconnected");

  close(client_fd);
}

int main()
{
  Debug(NAMESPACE_DEBUG::init());

  Dout(dc::notice, "sizeof(Board) = " << sizeof(Board));
  Dout(dc::notice, "sizeof(UncompressedBoard) = " << sizeof(UncompressedBoard));
  Dout(dc::notice, "sizeof(Info) = " << sizeof(Info));
  Dout(dc::notice, "sizeof(UncompressedInfo) = " << sizeof(UncompressedInfo));
  Dout(dc::notice, "sizeof(Classification) = " << sizeof(Classification));

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
    std::filesystem::path const prefix_directory = "/opt/ext4/nvme1/infchessKRvK";
    std::filesystem::path const data_directory = Graph::data_directory(prefix_directory);
    std::filesystem::path const data_filename = Graph::data_filename(prefix_directory);
    bool const file_exists = std::filesystem::exists(data_filename);

    if (!file_exists)
    {
      std::cerr << "The file " << data_filename << " does not exist!" << std::endl;
      return 1;
    }

    Dout(dc::notice, "Using existing file " << data_filename << ".");

    auto start = std::chrono::high_resolution_clock::now();

    // Only a new file is zero initialized.
    Graph const graph(prefix_directory, true);

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    std::cout << "Execution time (creating object Graph): " << (duration.count() / 1000000.0) << " seconds\n";

    // Set up socket server.
    int const port = 2000 + board_size_x;
    Dout(dc::notice, "Starting server on localhost:" << port);

    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == -1)
    {
      std::cerr << "Failed to create socket: " << strerror(errno) << std::endl;
      return 1;
    }
    auto&& close_fd = at_scope_end([server_fd]{ ::close(server_fd); });

    // Allow reuse of address.
    int opt = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1)
    {
      std::cerr << "setsockopt failed: " << strerror(errno) << std::endl;
      return 1;
    }

    struct sockaddr_in server_addr;
    std::memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    server_addr.sin_port = htons(port);

    if (bind(server_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1)
    {
      std::cerr << "Failed to bind socket: " << strerror(errno) << std::endl;
      return 1;
    }

    if (listen(server_fd, 5) == -1)
    {
      std::cerr << "Failed to listen on socket: " << strerror(errno) << std::endl;
      return 1;
    }

    std::cout << "Server listening on localhost:" << port << std::endl;
    std::cout << "Ready to accept connections..." << std::endl;

    // Accept connections and handle clients.
    while (true)
    {
      struct sockaddr_in client_addr;
      socklen_t client_addr_len = sizeof(client_addr);

      int client_fd = accept(server_fd, (struct sockaddr*)&client_addr, &client_addr_len);
      if (client_fd == -1)
      {
        Dout(dc::warning, "Failed to accept connection: " << strerror(errno));
        continue;
      }

      // Handle client in the same thread (single-threaded server).
      handle_client(client_fd, graph);
    }
  }
  catch (AIAlert::Error const& error)
  {
    std::cerr << "Fatal error: " << error << std::endl;
    return 1;
  }
}
