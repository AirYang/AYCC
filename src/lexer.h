#include <utility>
#include <vector>

#include "errors.h"
#include "tokens.h"

#ifndef SRC_LEXER_H_
#define SRC_LEXER_H_

class Lexer {
 public:
  Lexer(const std::vector<char>& buffer, const std::string& filename,
        bool need_lexer);

 public:
  void tokenize(std::vector<Token>& tokens, std::vector<CompilerError>& errors);

 private:
  void splitToTagged(std::vector<std::vector<Tagged>>& taggedlines);
  void joinExtendedLine(std::vector<std::vector<Tagged>>& taggedlines);
  void tokenizeLine(const std::vector<Tagged>& taggedline, bool& in_comment,
                    std::vector<Token>& linetokens,
                    std::vector<CompilerError>& errors);
  bool matchIncludeCommand(const std::vector<Token>& linetokens);
  void chunkToToken(const std::vector<Tagged>& taggedline, size_t chunk_start,
                    size_t chunk_end, std::vector<Token>& linetokens);
  std::pair<std::string, size_t> readIncludeFilename(
      const std::vector<Tagged>& taggedline, size_t start_index);
  std::pair<std::string, size_t> readString(
      const std::vector<Tagged>& taggedline, size_t start_index, char delim);

 private:
  std::vector<char> buffer_;
  std::string filename_;
  bool need_lexer_;
};
#endif  // SRC_LEXER_H_
