#ifndef MB_PARSER_SRC_CHAINS_GRAPH_H_
#define MB_PARSER_SRC_CHAINS_GRAPH_H_

#include <memory>
#include <unordered_map>
#include <cstdint>
#include <iostream>
#include "Vertex.h"

class Graph {
 public:
  Graph() = default;
  ~Graph() {
    for (auto &vertex : vertices_) {
      vertex.second->clear();
      vertex.second.reset();
    }
  }
  std::shared_ptr<Vertex> insertOrGetVertex(uint32_t frame);
  void appendToBytestreamOrder(const std::shared_ptr<Vertex> &vertex) { bytestream_order_.push_back(vertex); }
  bool isAtBytestreamTail(uint32_t frame) {
    return bytestream_order_.empty() ? false : bytestream_order_.back()->getFrame() == frame;
  }
  void setMaxWeight(uint32_t weight) { if (weight > max_weight_) max_weight_ = weight; }
  void print();
  void walk(uint32_t mb_threshold);
  uint32_t getMaxWeight() const {
    return max_weight_;
  }
  uint32_t getMaxChain();
  void flush(const std::string &file);
 private:
  std::unordered_map<uint32_t, std::shared_ptr<Vertex>> vertices_;
  std::vector<std::shared_ptr<Vertex>> bytestream_order_;
  uint32_t max_weight_ = 0;
  std::vector<uint32_t> chain_weights_;
};

#endif //MB_PARSER_SRC_CHAINS_GRAPH_H_
