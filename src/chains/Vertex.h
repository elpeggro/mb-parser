#ifndef MB_PARSER_SRC_CHAINS_VERTEX_H_
#define MB_PARSER_SRC_CHAINS_VERTEX_H_

#include <memory>
#include <vector>
#include <iostream>

class Vertex {
 public:
  Vertex() = delete;
  explicit Vertex(uint32_t frame) : frame_(frame), max_depth_(0) {}
  ~Vertex() {
    for (auto &vertex : successors_) {
      vertex.reset();
    }
  }
  void clear() {
    for (auto &vertex : successors_) {
      vertex.reset();
    }
    successors_.clear();
  }
  void addSuccessor(const std::shared_ptr<Vertex> &successor, uint32_t weight) {
    successors_.push_back(successor);
    weights_.push_back(weight);
  };
  uint32_t walk(uint32_t max_weight_influence, uint32_t mb_threshold, uint32_t depth);
  uint32_t getFrame() const { return frame_; }
  uint32_t getMaxDepth() const { return max_depth_; }
  void print();
 private:
  uint32_t frame_;
  std::vector<std::shared_ptr<Vertex>> successors_;
  std::vector<uint32_t> weights_;
  uint32_t max_depth_;
};

#endif //MB_PARSER_SRC_CHAINS_VERTEX_H_
