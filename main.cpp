#include <iostream>
#include <map>
#include <fstream>
#include <vector>
#include <string>
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
  int datacenterLatency;
  unordered_map<int, int> connections; // server id -> latency
  vector<int> requests; // video ids this endpoint requests
};

void parseInput(std::string filename);

//std::vector<Video> VIDEO_VECTOR;
//std::vector<Server> SERVER_VECTOR;
//std::vector<Endpoint>

std::map<int, Video> VIDEO_MAP;
std::map<int, Server> SERVER_MAP;
std::map<int, Endpoint> ENDPOINT_MAP;

int main() {
    std::string filename;
    std::ifstream inputFile;

    std::cout<<"which file do you want to open? ";
    std::cin>>filename;

    parseInput(filename);
}


void parseInput(std::string filename) {
    std::ifstream inputFile(filename);

    //read first line
    int numVideos, numEndpoints, numRequests, numCache, cacheCapacity;
    inputFile >> numVideos >> numEndpoints >> numRequests >> numCache >> cacheCapacity;

    std::vector<Video> videoVector;

    //read second line (video sizes)
    for(uint32_t i = 0; i < numVideos; ++i) {
        Video newVideo;
        newVideo.id = i;
        inputFile >> newVideo.size;
        VIDEO_MAP[i] = newVideo;
    }

    //read next set of lines (num endpoints)
    for(uint32_t i = 0; i < numEndpoints; ++i) {
        Endpoint currentEndpoint;
        currentEndpoint.id = i;
        inputFile >> currentEndpoint.datacenterLatency;
        int numCache;
        inputFile >> numCache;

        for(uint32_t cache = 0; cache < numCache; ++cache) {
            int cacheNum;
            int latency;
            inputFile >> cacheNum >> latency;
            currentEndpoint.connections[cacheNum] = latency;
        }

        ENDPOINT_MAP[i] = currentEndpoint;
    }





    inputFile.close();
}

// Emit optimized server capacities in form
//   <num_servers>\n
//   (<server_id> (<video_id>)*\n)*
void emit(const vector<Server>& servers) {
  cout << servers.size() << "\n";
  for (const auto& server : servers) {
    cout << server.id;
    for (const auto& video : server.videos) {
      cout << " " << video;
    }
    cout << "\n";
  }
  cout << flush;
}
