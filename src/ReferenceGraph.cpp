#include "ReferenceGraph.h"
#include <utility>
#include <fstream>
#include <iostream>

ReferenceGraph::ReferenceGraph(std::vector<Frame> frames_, std::vector<Macroblock>  mbs_) : frames(std::move(frames_)), mbs(std::move(mbs_)), max_weight(0) {

}
ReferenceGraph::ReferenceGraph(const Chunk &chunk) : frames(chunk.frames), mbs(chunk.mbs), max_weight(0) {

}
void ReferenceGraph::buildWeights() {
  for (auto &mb : mbs) {
    total_weights[mb.poc_ref]++;
    FrameWeights& ref_frame = graph[mb.poc_ref];
    ref_frame[mb.poc]++;
    if (ref_frame[mb.poc] > max_weight) {
      max_weight = ref_frame[mb.poc];
    }
  }
}

void ReferenceGraph::printAsDot(const std::string &path) {
  std::ofstream out(path);
  if (out.fail()) {
    std::cerr << "Error: Failed to open output file " << path << "\n";
    return;
  }
  out << "digraph G {\n";
  for (auto & frame_it : graph) {
    for (auto weight_it = frame_it.second.begin(); weight_it != frame_it.second.end(); weight_it++) {
      double edge_weight = ((double) weight_it->second / (double) max_weight) * MAX_THICKNESS;
      out << weight_it->first << " -> " << frame_it.first << " [label=\"" << weight_it->second << "\",penwidth=\"" << edge_weight << "\"];\n";
    }
  }
  out << "}";
  out.close();
}

void ReferenceGraph::printSummary(const std::string &path) {
  std::ofstream out(path);
  if (out.fail()) {
    std::cerr << "Error: Failed to open output file " << path << "\n";
    return;
  }
  for(auto & frame : frames) {
    out << frame.poc << " " << total_weights[frame.poc] << "\n";
  }
}
