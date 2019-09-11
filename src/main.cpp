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
uint32_t offset;
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
  while  (offset < file_size && *(file_mmap + offset) != sep && *(file_mmap + offset) != '\n') {
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
  std::string token  = parseString(sep);
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
  if (argc < 2) {
    cout << "usage: " << argv[0] << " <path/to/file>\n";
    return 1;
  }
  std::string log_file_path = argv[1];
  file_mmap = nullptr;
  file_size = 0;

  if (mmapFile(log_file_path) < 0) {
    return 1;
  }

  std::vector<Chunk> chunks;
  std::vector<Frame> frames;
  std::vector<Macroblock> mbs;
  offset = 0;
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
        if (next.type == 'I') {
          cout << "NEW CHUNK\n";
          chunks.emplace_back(Chunk{frames, mbs});
          frames.clear();
          mbs.clear();
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

  if (munmap(file_mmap, file_size) < 0) {
    cerr << "munmap: " << strerror(errno) << "\n";
  }
  ReferenceGraph graph(chunks[1]);
  graph.buildWeights();
  graph.printAsDot("1.dot");
  ReferenceGraph graph2(chunks[2]);
  graph2.buildWeights();
  graph2.printAsDot("2.dot");

  return 0;
}