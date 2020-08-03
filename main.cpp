#include <iostream>
#include <map>
#include <fstream>
#include <vector>
#include <string>
#include <unordered_map>
#include <unordered_set>


using namespace std;

static constexpr int DATA_CENTER = 1;

struct Video {
  int id;
  int size;
  unordered_set<int> requestedBy; // endpoint IDs requesting the video
};

struct Server {
  int id; // -1 for data center
  int size; // -1 for data center
  vector<int> videos; // video ids on this server
  unordered_set<int> videosSet;
};

struct Endpoint {
  int id;
  unordered_map<int, int> connections; // server id -> latency
  map<int, int> videoRequests; // video id -> # of requests
};

void parseInput(std::string filename);

std::vector<Video> VIDEOS;
std::vector<Server> SERVERS;
std::vector<Endpoint> ENDPOINTS;


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
        VIDEOS[i] = newVideo;
    }

    //read next set of lines (num endpoints)
    for(uint32_t i = 0; i < numEndpoints; ++i) {
        Endpoint currentEndpoint;
        currentEndpoint.id = i;
        int datacenterLatency;
        inputFile >> datacenterLatency;
        currentEndpoint.connections[DATA_CENTER] = datacenterLatency;
        int numCache;
        inputFile >> numCache;

        for(uint32_t cache = 0; cache < numCache; ++cache) {
            int cacheNum;
            int latency;
            inputFile >> cacheNum >> latency;
            currentEndpoint.connections[cacheNum] = latency;

            Server newServer;
            newServer.size = cacheCapacity;
            newServer.id = cache;
            SERVERS[cache] = newServer;
        }

        ENDPOINTS[i] = currentEndpoint;
    }

//    TODO idk what this does lol
//    for (const auto& endpoint : ENDPOINTS) {
//      for (const auto& videoPair : endpoint.videoRequests) {
//        VIDEOS[videoPair].requestedBy.insert(endpoint);
//      }
//    }

    //iterate through video requests
    for(uint32_t i = 0; i < numRequests; ++i) {
        int videoId;
        int endpointId;
        int numRequest;
        inputFile >> videoId >> endpointId >> numRequest;

        VIDEOS[videoId].requestedBy.insert(endpointId);
        ENDPOINTS[endpointId].videoRequests[videoId] = numRequest;
    }

    //TESTING PARSE



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

int calc_score(const vector<Endpoint>& endpoints) {
  int score{0};

  int vidId, vidNumReq, bestScore, scoreCand;
  for (const auto& endpoint : endpoints) {
    for (const auto& vidReq : endpoint.videoRequests) {
      vidId = vidReq.first;
      vidNumReq = vidReq.second;
      bestScore = 0;
      for (const auto& conn : endpoint.connections) {
        const auto& server = SERVERS[conn.first];
        if (server.videosSet.find(vidReq.first) != server.videosSet.end()) {
          scoreCand = (endpoint.connections.find(DATA_CENTER)->second - conn.second) * vidNumReq;
          bestScore = max(scoreCand, bestScore);
        }
      }
      score += bestScore;
    }
  }

  return score;
}

int getBestCacheFromVideoRequest(const vector<int>& allEndPointIDs, int videoId) {
  int minScore = -1;
  int minLatencyCache = -1;
  int dataCenterLatency = allEndPointIDs[-1];
  
  for (const auto& endPointID : allEndPointIDs) {
    Endpoint endpoint = ENDPOINTS[endPointID];
    int numVidRequests = endpoint.videoRequests[videoId];
		
    for (const auto& connection : endpoint.connections) { // connection = <server id, latency>
      int latency = connection.second;
      int candScore = numVidRequests * (dataCenterLatency - latency);
      if (minScore == -1 || candScore < minScore) {
          minScore = candScore;
          minLatencyCache = connection.first;
      }
    }
  }
    
  return minLatencyCache;

}
