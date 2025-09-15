#include "sys.h"
#include "Uncompressed.h"
#include "../Color.h"
#include "utils/AIAlert.h"
#include "utils/at_scope_end.h"
#include "utils/debug_ostream_operators.h"
#include <sys/socket.h>         // socket(), connect(), send(), recv()
#include <netinet/in.h>         // sockaddr_in, htons()
#include <arpa/inet.h>          // inet_pton()
#include <unistd.h>             // close()
#include <stdexcept>            // std::runtime_error
#include <string>               // std::string
#include <cstring>              // strerror() and memset()
#include "debug.h"

class Server
{
 private:
  int port_;
  int fd_{-1};

 private:
  // --- Helper Functions for Robust I/O ---

  // Helper function to ensure all data is sent.
  // A single call to send() is not guaranteed to send all bytes.
  void send_all(void const* buffer, size_t len)
  {
    char const* ptr = static_cast<char const*>(buffer);
    while (len > 0)
    {
      ssize_t bytes_sent = send(fd_, ptr, len, 0);
      if (bytes_sent < 0)
      {
        // EINTR is a temporary interruption; we can just retry.
        if (errno == EINTR)
          continue;
        THROW_ALERTE("send() failed");
      }
      ptr += bytes_sent;
      len -= bytes_sent;
    }
  }

  // Helper function to ensure all data is received.
  // A single call to recv() is not guaranteed to receive all bytes requested.
  void recv_all(void* buffer, size_t len)
  {
    char* ptr = static_cast<char*>(buffer);
    while (len > 0)
    {
      ssize_t bytes_received = recv(fd_, ptr, len, 0);
      if (bytes_received < 0)
      {
        // EINTR is a temporary interruption; we can just retry.
        if (errno == EINTR)
          continue;
        THROW_ALERTE("recv() failed");
      }
      if (bytes_received == 0)
        THROW_ALERT("Connection closed prematurely by the server");
      ptr += bytes_received;
      ASSERT(len >= bytes_received);
      len -= bytes_received;
    }
  }

 public:
  Server(int port) : port_(port)
  {
    // 1. Create a socket.
    fd_ = socket(AF_INET, SOCK_STREAM, 0);
    if (fd_ < 0)
      THROW_ALERTE("Failed to create socket");
    auto&& close_fd = at_scope_end([this]{ ::close(fd_); fd_ = -1; });

    // 2. Define the server address.
    struct sockaddr_in serv_addr;
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port); // Convert port to network byte order.

    // Convert "localhost" (127.0.0.1) to the correct network address format.
    if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0)
      THROW_ALERTE("Invalid address or address not supported");

    // 3. Connect to the server.
    if (connect(fd_, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0)
      THROW_ALERTE("Connection failed");

    close_fd.disarm();
  }

  ~Server()
  {
    if (fd_ != -1)
      close(fd_);
  }

  // Disable copy constructor and copy assignment.
  Server(Server const&) = delete;
  Server& operator=(Server const&) = delete;

  std::array<UncompressedInfo, 2> get_data(UncompressedBoard const& board)
  {
    // Send `board` in host order over the socket.
    // We use a helper to ensure all data is sent, even if TCP sends it in chunks.
    send_all(&board, sizeof(UncompressedBoard));

    // Wait for sizeof(UncompressedInfo) bytes to be returned and interpret those as host order.
    // We use a helper to ensure we read the entire message.
    std::array<UncompressedInfo, 2> answer;
    recv_all(answer.data(), sizeof(answer));

    return answer;
  }
};

void diff_ply(UncompressedBoard const& p, Color to_move, int ply32, int ply64)
{
  static int max_diff = 0;
  static int min_draw_ply = 1000;

  if (ply32 == 0)
  {
    if (ply64 < min_draw_ply)
    {
      min_draw_ply = ply64;
      Dout(dc::notice, "black king: " <<(int)p.bkx << ", " <<(int)p.bky <<
          "; white king: " <<(int)p.wkx << ", " <<(int)p.wky <<
          "; white rook: " <<(int)p.wrx << ", " <<(int)p.wry <<
          " (" << to_move << " to move)" <<
          ": ply32 = " << ply32 << ", ply64 = " << ply64);
    }
  }
  else
  {
    int diff = ply32 - ply64;
    if (diff > max_diff)
    {
      max_diff = diff;
      Dout(dc::notice, "black king: " <<(int)p.bkx << ", " <<(int)p.bky <<
          "; white king: " <<(int)p.wkx << ", " <<(int)p.wky <<
          "; white rook: " <<(int)p.wrx << ", " <<(int)p.wry <<
          " (" << to_move << " to move)" <<
          ": ply32 = " << ply32 << ", ply64 = " << ply64);
    }
  }
}

int main()
{
  Debug(NAMESPACE_DEBUG::init());

  try
  {
    Server s32(2032);
    Server s64(2064);

    for (int bkx = 5; bkx < 16; ++bkx)
    {
      for (int bky = 0; bky < 16; ++bky)
      {
        std::cout << "Testing bk " << bkx << ", " << bky << std::endl;
        for (int wkx = 0; wkx < 16; ++wkx)
        {
          for (int wky = 0; wky < 16; ++wky)
          {
            for (int wrx = 0; wrx < 16; ++wrx)
            {
              for (int wry = 0; wry < 16; ++wry)
              {
                UncompressedBoard pieces(bkx, bky, wkx, wky, wrx, wry);
                std::array<UncompressedInfo, 2> data32 = s32.get_data(pieces);
                std::array<UncompressedInfo, 2> data64 = s64.get_data(pieces);

                ASSERT(data32[0].classification == data64[0].classification);
                ASSERT(data32[1].classification == data64[1].classification);
                if (data32[0].mate_in_ply_encoded != data64[0].mate_in_ply_encoded)
                {
                  diff_ply(pieces, black, data32[0].mate_in_ply_encoded, data64[0].mate_in_ply_encoded);
                }
                //ASSERT(data32[0].mate_in_ply_encoded == data64[0].mate_in_ply_encoded);
                if (data32[1].mate_in_ply_encoded > data64[1].mate_in_ply_encoded + 20)
                {
                  diff_ply(pieces, white, data32[1].mate_in_ply_encoded, data64[1].mate_in_ply_encoded);
                }
                //ASSERT(data32[1].mate_in_ply_encoded == data64[1].mate_in_ply_encoded);
                if (data32[0].number_of_children != data64[0].number_of_children)
                {
                  Dout(dc::notice, "black king: " << bkx << ", " << bky << "; white king: " << wkx << ", " << wky << "; white rook: " << wrx << ", " << wry << ": data32[0].number_of_children = " << data32[0].number_of_children << " and data64[0].number_of_children = " << data64[0].number_of_children);
                }
                ASSERT(data32[0].number_of_children == data64[0].number_of_children);
                if (data32[1].number_of_children > data64[1].number_of_children)
                {
                  Dout(dc::notice, "black king: " << bkx << ", " << bky << "; white king: " << wkx << ", " << wky << "; white rook: " << wrx << ", " << wry << ": data32[1].number_of_children = " << data32[1].number_of_children << " and data64[1].number_of_children = " << data64[1].number_of_children);
                }
                ASSERT(data32[1].number_of_children <= data64[1].number_of_children);
              }
            }
          }
        }
      }
    }

  }
  catch (AIAlert::Error const& error)
  {
    std::cerr << error << std::endl;
    return 1;
  }
}
