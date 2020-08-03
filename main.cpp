#include <iostream>
#include <map>
#include <fstream>
#include <vector>
#include <string>

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

void parseInput(std::string filename);

std::vector<Video> VIDEO_VECTOR;
std::vector<Server> SERVER_VECTOR;


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

    //add video sizes
    for(uint32_t i = 0; i < numVideos; ++i) {
        Video newVideo;
        newVideo.id = i;
        inputFile >> newVideo.size;
        VIDEO_VECTOR.push_back(newVideo);
    }



    inputFile.close();
}








