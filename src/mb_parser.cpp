#include <iostream>
#include <cstring>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <cstdint>
#include <unistd.h>
#include <vector>

#include "structs.h"
#include "ReferenceGraph.h"

using std::cout;
using std::cerr;

uint8_t *file_mmap;
size_t offset;
size_t file_size;

int32_t mmapFile(const std::string &path) {
  struct stat st{};
  if (stat(path.c_str(), &st) < 0) {
    cerr << "error while getting file size: " << strerror(errno) << "\n";
    return -1;
  }
  file_size = st.st_size;
  int32_t file_fd = open(path.c_str(), O_RDONLY);
  if (file_fd < 0) {
    cerr << "could not open fd: " << strerror(errno) << "\n";
    return -1;
  }
  file_mmap = static_cast<uint8_t *>(mmap(nullptr, file_size, PROT_READ, MAP_PRIVATE | MAP_POPULATE, file_fd, 0));
  if (file_mmap == MAP_FAILED) {
    close(file_fd);
    cerr << "could not mmap file: " << strerror(errno) << "\n";
    return -1;
  }
  close(file_fd);
  if (madvise(file_mmap, file_size, MADV_SEQUENTIAL) < 0) {
    cerr << "madvise: " << strerror(errno) << "\n";
  }
  return 0;
}

void seek(char to) {
  while (offset < file_size && *(file_mmap + offset) != to) {
    offset++;
  }
  offset++;
}

std::string parseString(char sep) {
  if (offset >= file_size) {
    cerr << "Error: EOF\n";
    return "EOF";
  }
  uint32_t end = 0;
  while (offset + end < file_size && *(file_mmap + offset + end) != sep && *(file_mmap + offset + end) != '\n') {
    end++;
  }
  std::string ret((char *) file_mmap + offset, end);
  offset += end + 1;
  return ret;
}

int32_t parseUnsignedInt32(char sep) {
  if (offset >= file_size) {
    cerr << "Error: EOF\n";
    return -1;
  }
  int32_t ret = 0;
  while (offset < file_size && *(file_mmap + offset) != sep && *(file_mmap + offset) != '\n') {
    if (*(file_mmap + offset) < 48 || *(file_mmap + offset) > 57) {
      cerr << "Error: Invalid number: " << *(file_mmap + offset) << "\n";
      return -1;
    }
    ret *= 10;
    ret += *(file_mmap + offset) - 48;
    offset++;
  }
  offset++;
  return ret;
}

bool parseToken(const std::string &expected, char sep) {
  std::string token = parseString(sep);
  if (token != expected) {
    cerr << "Error: Unexpected token: " << token << "\n";
    return false;
  }
  return true;
}

Frame parseFrame() {
  if (offset >= file_size) {
    cerr << "Error: EOF\n";
    return Frame{-1};
  }
  Frame ret{};
  if (!parseToken("type:", ' ')) {
    return Frame{-1};
  }
  if (offset >= file_size) {
    cerr << "Error: EOF\n";
    return Frame{-1};
  }
  ret.type = *(file_mmap + offset);
  offset += 2;
  if (!parseToken("poc:", ' ')) {
    return Frame{-1};
  }
  ret.poc = parseUnsignedInt32(' ');
  if (ret.poc < 0) {
    return Frame{-1};
  }
  return ret;
}

Macroblock parseMB() {
  if (offset >= file_size) {
    cerr << "Error: EOF\n";
    return Macroblock{-1};
  }
  Macroblock ret{};
  if (!parseToken("type:", ' ')) {
    return Macroblock{-1};
  }
  if (offset >= file_size) {
    cerr << "Error: EOF\n";
    return Macroblock{-1};
  }
  ret.type = *(file_mmap + offset);
  offset += 2;
  if (!parseToken("poc:", ' ')) {
    return Macroblock{-1};
  }
  ret.poc = parseUnsignedInt32(' ');
  if (ret.poc < 0) {
    return Macroblock{-1};
  }
  if (!parseToken("x:", ' ')) {
    return Macroblock{-1};
  }
  ret.x = parseUnsignedInt32(' ');
  if (ret.x < 0) {
    return Macroblock{-1};
  }
  if (!parseToken("y:", ' ')) {
    return Macroblock{-1};
  }
  ret.y = parseUnsignedInt32(' ');
  if (ret.y < 0) {
    return Macroblock{-1};
  }
  if (!parseToken("list:", ' ')) {
    return Macroblock{-1};
  }
  ret.list = parseUnsignedInt32(' ');
  if (ret.list < 0) {
    return Macroblock{-1};
  }
  if (!parseToken("poc_ref:", ' ')) {
    return Macroblock{-1};
  }
  ret.poc_ref = parseUnsignedInt32(' ');
  if (ret.poc_ref < 0) {
    return Macroblock{-1};
  }
  if (!parseToken("idx:", ' ')) {
    return Macroblock{-1};
  }
  ret.idx = parseUnsignedInt32(' ');
  if (ret.idx < 0) {
    return Macroblock{-1};
  }
  return ret;
}

int main(int32_t argc, char **argv) {
  std::string dot_file_prefix;
  std::string summary_file_prefix;
  std::string dat_file_prefix;
  size_t segment_size = 0;
  std::string dot_parameter = "--dot";
  std::string summary_parameter = "--summary";
  std::string dat_parameter = "--dat";
  std::string segment_size_parameter = "--segment-size";
  if (argc < 2) {
    cout << "usage: " << argv[0]
         << " <path/to/file> [--dot <dot-file-prefix>] [--summary <summary-file-prefix>] [--dat <dat-file-prefix>] [--segment-size <segment-size>]\n";
    return 1;
  }
  std::string log_file_path = argv[1];
  for (int32_t i = 2; i < argc; i++) {
    std::string next_arg = argv[i];
    if (!next_arg.compare(0, next_arg.size(), dot_parameter) && i + 1 < argc) {
      dot_file_prefix = argv[i + 1];
      i++;
    } else if (!next_arg.compare(0, next_arg.size(), summary_parameter) && i + 1 < argc) {
      summary_file_prefix = argv[i + 1];
      i++;
    } else if (!next_arg.compare(0, next_arg.size(), dat_parameter) && i + 1 < argc) {
      dat_file_prefix = argv[i + 1];
      i++;
    } else if (!next_arg.compare(0, next_arg.size(), segment_size_parameter) && i + 1 < argc) {
      segment_size = std::stoul(argv[i + 1]);
      i++;
    } else {
      cerr << "Unknown parameter or missing argument: " << argv[i] << "\n";
      return 1;
    }
  }
  file_mmap = nullptr;
  file_size = 0;

  if (mmapFile(log_file_path) < 0) {
    return 1;
  }

  std::vector<Chunk> chunks;
  std::vector<Frame> frames;
  std::vector<Macroblock> mbs;
  bool first_chunk = true;
  if (segment_size > 0) {
    // Fixed segment size does not need first chunk detection.
    first_chunk = false;
  }
  offset = 0;
  cout << "Parsing file " << log_file_path << '\n';
  while (offset < file_size) {
    if (*(file_mmap + offset) != '[') {
      seek('\n');
      continue;
    }
    offset++;
    if (offset >= file_size) {
      cerr << "Error: EOF\n";
      return 1;
    }
    if (*(file_mmap + offset) == 'f') {
      // Skip 'frame] '
      offset += 7;
      Frame next = parseFrame();
      if (next.poc >= 0) {
        if ((segment_size > 0 && frames.size() == segment_size)
            || (segment_size == 0 && next.type == 'I')) {
          // Prevent that we add an empty first chunk.
          if (!first_chunk) {
            chunks.emplace_back(Chunk{frames, mbs});
            frames.clear();
            mbs.clear();
          } else {
            first_chunk = false;
          }
        }
        frames.push_back(next);
      }
    } else if (*(file_mmap + offset) == 'm') {
      // Skip 'mb] '
      offset += 4;
      Macroblock next = parseMB();
      if (next.poc >= 0) {
        mbs.push_back(next);
      }
    }
  }
  // Add last chunk
  if (!frames.empty() && !mbs.empty()) {
    chunks.emplace_back(Chunk{frames, mbs});
  }

  cout << "Parsed " << chunks.size() << " chunks\n";

  if (munmap(file_mmap, file_size) < 0) {
    cerr << "munmap: " << strerror(errno) << "\n";
  }

  uint32_t chunk_count = 1;
  for (auto &chunk : chunks) {
    cout << "Processing chunk " << chunk_count << " (" << chunk.frames.size() << " frames | " << chunk.mbs.size()
         << " macroblocks)\n";
    ReferenceGraph graph(chunk);
    graph.buildWeights();
    if (!dot_file_prefix.empty()) {
      std::string suffix = "-" + std::to_string(chunk_count) + ".dot";
      graph.printAsDot(dot_file_prefix + suffix);
    }
    if (!summary_file_prefix.empty()) {
      std::string suffix = "-" + std::to_string(chunk_count) + ".dat";
      graph.printSummary(summary_file_prefix + suffix);
    }
    if (!dat_file_prefix.empty()) {
      std::string suffix = "-" + std::to_string(chunk_count) + ".dat";
      graph.printAsDat(dat_file_prefix + suffix);
    }
    chunk_count++;
  }
  return 0;
}