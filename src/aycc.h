#include <string>
#include <vector>

#include "errors.h"
#include "tokens.h"

#ifndef SRC_AYCC_H_
#define SRC_AYCC_H_

class Aycc {
 public:
  Aycc(int argc, char** argv);

 public:
  bool run();

 private:
  std::string procFile(const std::string& file);
  std::string procCFile(const std::string& file);
  bool readCFile(const std::string& file, std::vector<char>& buffer);
  void showErrors();
  void showTokens(const std::vector<Token>& tokens);
  bool isErrorsOk();

 private:
  bool need_lexer_;
  std::vector<std::string> files_;
  std::vector<CompilerError> errors_;
};
#endif  // SRC_AYCC_H_
