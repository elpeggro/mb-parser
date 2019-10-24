#include "Vertex.h"

void Vertex::print() {
  std::cout << frame_ << "\n";
  for (const auto &vertex : successors_) {
    std::cout << "  ->" << vertex->getFrame() << "\n";
  }
}

uint32_t Vertex::walk(uint32_t max_weight_influence, uint32_t mb_threshold, uint32_t depth) {
  if (depth > max_depth_) {
    max_depth_ = depth;
  }
  uint32_t ret = 0;
  for (size_t i = 0; i < successors_.size(); i++) {
    uint32_t new_weight_influence = (weights_[i] < max_weight_influence) ? weights_[i] : max_weight_influence;
    ret += new_weight_influence;
    if (new_weight_influence < mb_threshold) {
      continue;
    }
    if (new_weight_influence == weights_[i]) {
      // Normal edge weight is the limiting factor. This case will happen often so we can reuse a previously calculated
      // value.
      if (successor_chain_weights_[i] == -1) {
        successor_chain_weights_[i] = successors_[i]->walk(new_weight_influence, mb_threshold, depth + 1);
      }
      ret += successor_chain_weights_[i];
    } else {
      // Limiting factor comes from further up in the chain so we need to do a 'custom' walk.
      ret += successors_[i]->walk(new_weight_influence, mb_threshold, depth + 1);
    }
  }
  return ret;
}
