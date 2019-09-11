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
  explicit ReferenceGraph(const Chunk &chunk);
  void buildWeights();
  void printAsDot(const std::string &path);
  void printSummary(const std::string &path);

 private:
  static constexpr double MAX_THICKNESS = 5;
  /**
   * Mapping for a single frame that keeps track of how many MB references from other frames exist.
   * Maps: Other frame number -> Number of references
   */
  typedef std::map<uint32_t, uint32_t> FrameWeights;
  const std::vector<Frame> frames;
  const std::vector<Macroblock> mbs;
  uint32_t max_weight;
  /// Mapping that keeps track of weights for every frame in the chunk.
  std::map<uint32_t, FrameWeights> graph;
  /// Mapping that keeps track of the total weight of each frame.
  std::map<uint32_t, uint32_t> total_weights;
};

#endif //MB_PARSER__REFERENCEGRAPH_H_
