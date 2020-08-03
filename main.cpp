#include <iostream>
#include <vector>

using namespace std;

struct Video {
  int id;
  int size;
  vector<int> requestedBy; // endpoint IDs requesting the video
};

struct Server {
  int id; // -1 for data center
  int size; // -1 for data center
  vector<int> videos; // video ids on this server
};

struct Connection {
  int serverId;
  int latency;
};

struct Endpoint {
  int id;
  vector<Connection> connectedTo;
  vector<int> requests; // video ids this endpoint requests
};

int main() {
    std::cout << "Hello, World!" << std::endl;
    return 0;
}
