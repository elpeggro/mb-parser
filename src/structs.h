#ifndef MB_PARSER__STRUCTS_H_
#define MB_PARSER__STRUCTS_H_

#include <cstdint>
typedef struct {
  int32_t poc;
  char type;
} Frame;

typedef struct {
  int32_t poc;
  char type;
  int32_t x;
  int32_t y;
  int32_t list;
  int32_t poc_ref;
  int32_t idx;
} Macroblock;

typedef struct {
  std::vector<Frame> frames;
  std::vector<Macroblock> mbs;
} Chunk;

#endif //MB_PARSER__STRUCTS_H_
