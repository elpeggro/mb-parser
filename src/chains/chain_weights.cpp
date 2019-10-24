#include <cstdint>
#include <string>
#include <iostream>
#include <unordered_map>
#include <fstream>
#include <cstring>
#include <cmath>

#include "Graph.h"

std::unordered_map<int32_t, uint32_t> MACROBLOCKS = {{144, 144}, {240, 400}, {360, 900},
                                                     {480, 1602}, {720, 3600}, {1080, 8100},
                                                     {1440, 14400}, {2160, 32400}};

using std::cout;
using std::cerr;

Graph getGraphFromFile(const std::string &graph_file) {
  Graph ret;
  std::ifstream file(graph_file);
  if (file.fail()) {
    throw std::runtime_error("Failed to open graph file " + graph_file + ": " + strerror(errno));
  }
  std::string header;
  // Consume headers
  file >> header >> header >> header;
  uint32_t frame = 0;
  uint32_t weight = 0;
  uint32_t referenced_by_frame = 0;
  while (file >> frame >> weight >> referenced_by_frame) {
    std::shared_ptr<Vertex> frame_vertex = ret.insertOrGetVertex(frame);
    if (!ret.isAtBytestreamTail(frame)) {
      ret.appendToBytestreamOrder(frame_vertex);
    }
    if (weight == 0) {
      // Sink
      continue;
    }
    std::shared_ptr<Vertex> successor_vertex = ret.insertOrGetVertex(referenced_by_frame);
    frame_vertex->addSuccessor(successor_vertex, weight);
    ret.setMaxWeight(weight);
  }
  return ret;
}

int main(int32_t argc, char **argv) {
  bool relative = false;
  int32_t absolute_resolution = 0;
  std::string relative_parameter = "--relative";
  std::string absolute_parameter = "--absolute";
  if (argc < 5 || argc > 6) {
    cout << "usage: " << argv[0]
         << " <input> <output> <threshold> (--relative | --absolute <resolution>)\n";
    return 1;
  }
  std::string input = argv[1];
  std::string output = argv[2];
  double threshold = std::stod(argv[3]);
  if (argv[4] == relative_parameter) {
    relative = true;
  } else if (argv[4] == absolute_parameter && argc == 6) {
    absolute_resolution = std::stoi(argv[5]);
  } else {
    cerr << "Unknown parameter or missing argument: " << argv[4] << "\n";
    return 1;
  }
  std::cout << "Input: " << input << " Output: " << output << "\n";
  Graph g = getGraphFromFile(input);
  uint32_t ref_size = 0;
  if (relative) {
    ref_size = g.getMaxWeight();
    std::cout << "Max weight: " << ref_size << "\n";
  } else {
    auto find_it = MACROBLOCKS.find(absolute_resolution);
    if (find_it == MACROBLOCKS.end()) {
      std::cerr << "Failed  to find macroblock count for resolution " << absolute_resolution << "\n";
      return 1;
    }
    ref_size = find_it->second;
  }
  uint32_t mb_threshold = std::ceil(ref_size * threshold / 100);
  std::cout << "Using threshold: " << mb_threshold << "\n";
  g.walk(mb_threshold);
  std::cout << "Maximum chain length: " << g.getMaxChain() << "\n";
  g.flush(output);
  std::cout << "Done.\n";
}
