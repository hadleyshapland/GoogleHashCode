#include <iostream>
#include <vector>
#include <unordered_map>

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

struct Endpoint {
  int id;
  unordered_map<int, int> connections; // server id -> latency
  vector<int> requests; // video ids this endpoint requests
};

int main() {
    std::cout << "Hello, World!" << std::endl;
    return 0;
}
