#ifndef MB_PARSER__REFERENCEGRAPH_H_
#define MB_PARSER__REFERENCEGRAPH_H_

#include <vector>
#include <cstdint>
#include <map>
#include "structs.h"

class ReferenceGraph {
 public:
  ReferenceGraph() = delete;
  ReferenceGraph(std::vector<Frame> frames_, std::vector<Macroblock> mbs_);
  ReferenceGraph(const Chunk &chunk);
  void buildWeights();
  void printAsDot(const std::string &path);

 private:
  static constexpr double MAX_THICKNESS = 5;
  typedef std::map<uint32_t, uint32_t> FrameWeights;
  const std::vector<Frame> frames;
  const std::vector<Macroblock> mbs;
  uint32_t max_weight;
  std::map<uint32_t, FrameWeights> graph;
};

#endif //MB_PARSER__REFERENCEGRAPH_H_
