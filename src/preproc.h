
#include "errors.h"
#include "lexer.h"
#include "tokens.h"

#ifndef SRC_PREPROC_H_
#define SRC_PREPROC_H_

class PreProc {
 public:
  PreProc(const std::string& filename, bool need_lexer);

 public:
  void retokenize(std::vector<Token>& tokens,
                  std::vector<CompilerError>& errors);

 private:
  std::string readInludeFile(const std::string& includefile,
                             std::vector<char>& buffer);

 private:
  std::string filename_;
  bool need_lexer_;
};

#endif  // SRC_PREPROC_H_
