#include <iostream>
#include <map>
#include <algorithm>
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
    std::string filename, outputName;
    std::ifstream inputFile;

    std::cout<<"which file do you want to open? ";
    std::cin>>filename;

    std::cout<<"which file do you want to write to? ";
    std::cin>>outputName;

    parseInput(filename);

    std::map<int, std::vector<int>> result;

    std::vector<VideoId> sortedVideos = sortBySizeAndRequests();
    for (const auto& videoId : sortedVideos) {
      int cacheId = getBestCacheFromVideoRequest(VIDEO_MAP[videoId].requestedBy, videoId);
      result[cacheid].push_back(videoId);
    }

    outputFile(outputName, result)
    
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

template<typename T>
vector<int> keys_of_map(const T& mp) {
  vector<int> keys(mp.size());
  for (const auto& p : mp) {
    keys.push_back(p.first);
  }
  return keys;
}

vector<VideoId> sortBySizeAndRequests() {
  // size / total # of requests. smaller = better
  //   e.g. 100 MB, 10 req (ratio 10) is worse than 200 MB, 50 req (ratio 4).
  //
  // TODO: Maybe we should be looking at the total # of requests per endpoint,
  // rather than aggregating all endpoint requests, because later on we try to
  // attach the video to only one (the best) cache.
  auto score = [](int videoId) -> double {
    const auto& video = VIDEO_MAP[videoId];
    const double size = video.size;
    double requests = 0;
    for (int req : video.requestedBy) {
      auto& ep = ENDPOINT_MAP[req];
      requests += ep.videoRequests[videoId];
    }

    return (requests / size);
  };

  vector<VideoId> videos = keys_of_map(VIDEO_MAP);
  sort(videos.begin(), videos.end(), score);
  return videos;
}

int getBestCacheFromVideoRequest(int videoId) {
  int minScore = -1;
  int minLatencyCache = -1;

  for (const auto& endPointID : VIDEOS[videoId].requestedBy) {
    Endpoint endpoint = ENDPOINTS[endPointID];

    int numVidRequests = endpoint.videoRequests[videoId];
    int dataCenterLatency = endpoint.connections[DATA_CENTER];
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

void outputFile(std::string& fileName, std::map<int, std::vector<int>> result) {
  std::ofstream outputFile(fileName);

  ofstream << result.size() << std::endl;

  for (Map.Entry<int, std::vector<int>> entry : result.entrySet()) {
    int cacheId = entry.getKey();
    std::vector vec = entry.getValue();
    ofstream << cacheId << " ";
    std::copy (result.begin(), result.end(), std::ostream_iterator<int>(ofstream, " ");
    ofstream << std::endl;
  }
}
