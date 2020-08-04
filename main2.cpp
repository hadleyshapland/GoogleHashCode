#include <iostream>
#include <iterator>
#include <map>
#include <algorithm>
#include <numeric>
#include <fstream>
#include <vector>
#include <string>
#include <unordered_map>
#include <unordered_set>

using namespace std;
using VideoId = int;

static constexpr int DATA_CENTER = -1;

struct Video {
  int id;
  int size;
  unordered_set<int> requestedBy; // endpoint IDs requesting the video
  void print() const {
    cerr << id << " " << size << " req by ";
    for (const auto& req : requestedBy) {
      cerr << req << " ";
    }
  }
};

struct Server {
  int id; // -1 for data center
  int size; // -1 for data center
  vector<int> videos; // video ids on this server
  unordered_set<int> videosSet;
  void print() const {
    cerr << id << " " << size << " has videos ";
    for (const auto& video : videos) {
      cerr << video << " ";
    }
  }
};

struct Endpoint {
  int id;
  unordered_map<int, int> connections; // server id -> latency
  unordered_map<int, int> videoRequests; // video id -> # of requests
  void print() const {
    cerr << id << " " << " has conn ";
    for (const auto& conn : connections) {
      cerr << conn.first << " ";
    }
    cerr << " has videos ";
    for (const auto& vid : videoRequests) {
      cerr << vid.first << " ";
    }
  }
};

void parseInput(std::string filename);

std::vector<Video> VIDEOS;
std::vector<Server> SERVERS;
std::vector<Endpoint> ENDPOINTS;


void parseInput(std::string filename) {
    std::ifstream inputFile(filename);

    //read first line
    int numVideos, numEndpoints, numRequests, numCache, cacheCapacity;
    inputFile >> numVideos >> numEndpoints >> numRequests >> numCache >> cacheCapacity;

    VIDEOS.reserve(numVideos);
    ENDPOINTS.reserve(numEndpoints);
    SERVERS = vector<Server>(numCache, Server{});

    std::vector<Video> videoVector;

    //read second line (video sizes)
    for(uint32_t i = 0; i < numVideos; ++i) {
        Video newVideo;
        newVideo.id = i;
        inputFile >> newVideo.size;
        VIDEOS.push_back(newVideo);
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

        ENDPOINTS.push_back(currentEndpoint);
    }

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

    cerr << "VIDEOS" << endl;
    for (const auto& video : VIDEOS) {
      cerr << "\t"; video.print(); cerr << endl;;
    }
    cerr << "ENDPOINTS" << endl;
    for (const auto& ep : ENDPOINTS) {
      cerr << "\t"; ep.print(); cerr << endl;;
    }
    cerr << "SERVERS" << endl;
    for (const auto& server : SERVERS) {
      cerr << "\t"; server.print(); cerr << endl;;
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

double calc_score(const vector<Endpoint>& endpoints) {
  double score{0};

  int vidId, vidNumReq, bestScore, scoreCand;
  for (const auto& endpoint : endpoints) {
    for (const auto& vidReq : endpoint.videoRequests) {
      vidId = vidReq.first;
      vidNumReq = vidReq.second;
      bestScore = 0;
      for (const auto& conn : endpoint.connections) {
        if (conn.first == -1) continue;
        const auto& server = SERVERS[conn.first];
        if (server.videosSet.find(vidReq.first) != server.videosSet.end()) {
          scoreCand = (endpoint.connections.find(DATA_CENTER)->second - conn.second) * vidNumReq;
          cout << vidId << ": " << (endpoint.connections.find(DATA_CENTER)->second - conn.second) << " " << vidNumReq << endl;
          bestScore = max(scoreCand, bestScore);
        }
      }
      score += bestScore;
    }
  }

  double allRequests = 0;
  for (const auto& vid : VIDEOS){
    for (const auto& req : vid.requestedBy) {
      allRequests += ENDPOINTS[req].videoRequests[vid.id];
    }
  }
  cout << "allRequests: " << allRequests << endl;

  return score / allRequests * 1000;
}

std::vector<std::vector<int>> constructDefaultLatencies() {
    // Construct a 2d array where the rows represent endpoints
    // and the columns represent their latency per video
    std::vector<std::vector<int>> latenciesPerEndpoint;

    for (auto endpoint : ENDPOINTS) {
        std::vector<int> latencies;
        for (size_t i = 0; i < VIDEOS.size(); i++) {
            latencies.push_back(endpoint.connections[DATA_CENTER]);
        }
        latenciesPerEndpoint.push_back(latencies);
    }

    return latenciesPerEndpoint;
}

double videoValue(int videoId, int serverId, const std::vector<std::vector<int>> &latencyMatrix) {
    size_t value = 0;

    for (const int& endpointId : VIDEOS[videoId].requestedBy) {
        std::vector<int> endpointLatencies = latencyMatrix[endpointId];
        int latencyDiff = latencyMatrix[endpointId][videoId] - ENDPOINTS[endpointId].connections[serverId];
        if (latencyDiff > 0) {
            int numVids = ENDPOINTS[endpointId].videoRequests[videoId];
            value = latencyDiff * numVids;
        }
    }

    return value / (double) VIDEOS[videoId].size;
}

std::vector<int> getOptimalAllocation(Server &server, std::vector<std::vector<int>> latencyMatrix) {
    size_t spaceLeft = server.size;
    std::vector<int> videosInServer;
    std::vector<int> videoIds(VIDEOS.size());
    std::iota(std::begin(videoIds), std::end(videoIds), 0);
    sort(
            videoIds.begin(),
            videoIds.end(),
            [server, latencyMatrix](const int & a, const int & b) -> bool {
                return videoValue(a, server.id, latencyMatrix) > videoValue(a, server.id, latencyMatrix);
            });
    for (const auto & video: VIDEOS) {
        if (spaceLeft > video.size) {
            videosInServer.push_back(video.id);
            spaceLeft -= video.size;
        }
    }

    return videosInServer;
}

void updateEndPoints(std::vector<std::vector<int>> &latencyMatrix, const Server &server) {
    for (size_t i = 0; i < ENDPOINTS.size(); i++) {
        std::vector<int> endpointLatencies = latencyMatrix[i];
        Endpoint endpoint = ENDPOINTS[i];
        for ( const auto &videoId : server.videos) {
            if (endpoint.videoRequests.count(videoId) && endpointLatencies[videoId] > endpoint.connections[server.id]) {
                endpointLatencies[videoId] = endpoint.connections[server.id];
            }
        }
    }
}

void serverByServer() {
    std::vector<std::vector<int>> latencyMatrix = constructDefaultLatencies();
    for (auto & server : SERVERS) { // TODO: make sure data center isn't included
        server.videos = getOptimalAllocation(server, latencyMatrix);
        updateEndPoints(latencyMatrix, server);
    }
}

void outputFile(std::string& fileName, unordered_map<int, std::vector<int>> result) {
  std::ofstream outputFile(fileName);

  outputFile << result.size() << std::endl;

  for (const auto& entry : result) {
    int cacheId = entry.first;
    const auto& vec = entry.second;
    outputFile << cacheId << " ";
    std::copy (vec.begin(), vec.end(), std::ostream_iterator<int>(outputFile, " "));
    outputFile << std::endl;
  }
}

int main(int argc, char** argv) {
    std::string filename(argv[1]), outputName(argv[2]);

    cerr << "reading " << filename << ", writing " << outputName << endl;

    parseInput(filename);

//    std::map<int, std::vector<int>> result;

//    std::vector<VideoId> sortedVideos = sortBySizeAndRequests();
//    for (const auto& videoId : sortedVideos) {
//      int cacheId = getBestCacheFromVideoRequest(videoId);
//      result[cacheId].push_back(videoId);
//    }
//    std::map<int, std::vector<int>> result;
    unordered_map<int, std::vector<int>> result;

    serverByServer();
    for (const auto& server: SERVERS) {
        for (const auto &video: server.videos) {
            result[server.id].push_back(video);
        }
    }

    // std::vector<VideoId> sortedVideos = sortBySizeAndRequests();
    // cerr << "sorted videos: ";
    // for (auto i : sortedVideos) cerr << i << " ";
    // cerr << endl;
    // for (const auto& videoId : sortedVideos) {
    //   int cacheId = getBestCacheFromVideoRequest(videoId);
    //   if (cacheId == -1) continue;
    //   cerr << cacheId << endl;
    //   result[cacheId].push_back(videoId);
    //   SERVERS[cacheId].size -= VIDEOS[videoId].size;
    // }

    // Update endpoints and print score
    for (const auto& p : result) {
      SERVERS[p.first].videos = p.second;
      SERVERS[p.first].videosSet = unordered_set<int>(p.second.begin(), p.second.end());
    }

    const int score = calc_score(ENDPOINTS);
    cerr << flush;
    cerr << "Score: " << score << endl;

    outputFile(outputName, result);
}
