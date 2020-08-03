#include <iostream>
#include <map>
#include <fstream>
#include <vector>
#include <string>

struct Endpoint {
    int endpointNumber;
    int numServersConnected;
    int latencyFromDataCenter;

    //map cache number to latency amount
    std::map<int, int> latencyFromCache;
};

struct Video {
    //stuff
};

std::vector<Video> parseInput(std::string filename);


int main() {
    std::string filename;
    std::ifstream inputFile;

    std::cout<<"which file do you want to open? ";
    std::cin>>filename;

    std::vector<Video> videos = parseInput(filename);
}

std::vector<Video> parseInput(std::string filename) {
    std::ifstream inputFile(filename);

    //read first line
    int numVideos, numEndpoints, numRequests, numCache, cacheCapacity;
    inputFile >> numVideos >> numEndpoints >> numRequests >> numCache >> cacheCapacity;

    std::cout << numVideos << " " << numEndpoints << " " << numRequests << " " << numCache << " "
    << cacheCapacity <<std::endl;

    std::vector<Video> videoVector;

    //add video sizes
    for(uint32_t i = 0)


    inputFile.close();
    return videoVector;

}








