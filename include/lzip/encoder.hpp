#pragma once
#include "bitio.hpp"
#include "dictionary.hpp"
#include <iostream>
#include <string>
#include <vector>

namespace lzw {

inline void execEncoding(std::istream &input, const std::string &filename) {
  auto [dict, lookup] = initDictionary();
  int nextCode{256};
  int codeSize{9};

  constexpr int MAXBITS{20};
  constexpr int MAXCODE{(1 << MAXBITS) - 1};

  auto bp = newBitPacker();
  int p{-1};
  std::vector<uint8_t> buf(4096);

  while (input) {
    input.read(reinterpret_cast<char *>(buf.data()), buf.size());
    std::streamsize n = input.gcount();
    for (std::streamsize i = 0; i < n; ++i) {
      uint8_t c{buf[i]};
      DictKey key{p, int(c)};
      auto it = lookup.find(key);
      if (it != lookup.end()) {
        p = it->second;
      } else {
        if (p != -1)
          bp.writeCode(p, codeSize);
        if (nextCode < MAXCODE) {
          dict.push_back(DictionaryEntry{p, c});
          lookup[key] = nextCode;
          ++nextCode;
          if (nextCode == (1 << codeSize) && codeSize < MAXBITS)
            ++codeSize;
        }
        p = c;
      }
    }
  }

  if (p != -1)
    bp.writeCode(p, codeSize);
  bp.flushRemaining();
  bp.writeOutputToFile(filename + ".lzw");
}

inline void startEncoding(std::istream &input, const std::string &filename) {
  try {
    execEncoding(input, filename);
  } catch (...) {
    std::cerr << "Error Encoding\n";
  }
}

} // namespace lzw
