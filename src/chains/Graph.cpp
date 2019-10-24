#include "Graph.h"
#include <fstream>
#include <cstring>

std::shared_ptr<Vertex> Graph::insertOrGetVertex(uint32_t frame) {
  auto find_it = vertices_.find(frame);
  if (find_it == vertices_.end()) {
    auto insert_pair = vertices_.emplace(frame, new Vertex(frame));
    return insert_pair.first->second;
  }
  return find_it->second;
}

void Graph::print() {
  for (const auto &vertex : bytestream_order_) {
    vertex->print();
  }
}

void Graph::walk(uint32_t mb_threshold) {
  for (auto &vertex : bytestream_order_) {
    uint32_t chain_weight = vertex->walk(UINT32_MAX, mb_threshold, 0);
    chain_weights_.push_back(chain_weight);
  }
}
void Graph::flush(const std::string &file) {
  std::ofstream f(file);
  if (f.fail()) {
    std::cerr << "Error: Failed to open output file " << file << ": " << strerror(errno) << "\n";
    return;
  }
  for (size_t i = 0; i < bytestream_order_.size(); i++) {
    f << bytestream_order_[i]->getFrame() << " " << chain_weights_[i] << "\n";
  }
}
uint32_t Graph::getMaxChain() {
  uint32_t max_chain = 0;
  for (const auto &vertex : bytestream_order_) {
    if (vertex->getMaxDepth() > max_chain) {
      max_chain = vertex->getMaxDepth();
    }
  }
  return max_chain;
}
