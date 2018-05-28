#include "aycc.h"

#include <algorithm>
#include <fstream>
#include <iostream>
#include <stdexcept>

#include "lexer.h"
#include "para_init.h"
#include "preproc.h"

Aycc::Aycc(int argc, char** argv) : need_lexer_(false), files_() {
  ParaInit para_init(argc, argv);
  need_lexer_ = para_init.needLexer();
  files_ = para_init.getFiles();
}

bool Aycc::run() {
  // proc every .c to produce .o
  std::vector<std::string> objs;
  for (const auto& file : files_) {
    std::string obj = procFile(file);
    if (obj.compare("")) {
      objs.push_back(obj);
    }
  }

  showErrors();

  if (objs.size() < files_.size()) {
    throw CompilerError(
        "not enough number of properly processed files to link");
  }

  return true;
}

std::string Aycc::procFile(const std::string& file) {
  if (file.size() < 2) {
    errors_.push_back(CompilerError("unknown file type [" + file + "]"));
    return "";
  }

  if (!file.substr(file.size() - 2).compare(".c")) {
    return procCFile(file);
  }

  if (!file.substr(file.size() - 2).compare(".o")) {
    return file;
  }

  errors_.push_back(CompilerError("unknown file type [" + file + "]"));
  return "";
}

std::string Aycc::procCFile(const std::string& file) {
  std::vector<char> buffer;
  if (!readCFile(file, buffer)) {
    errors_.push_back(CompilerError("file can't open [" + file + "]"));
    return "";
  }

  std::vector<Token> tokens;
  Lexer lexer(buffer, file, need_lexer_);
  lexer.tokenize(tokens, errors_);
  if (!isErrorsOk()) {
    return "";
  }

  PreProc preproc(file, need_lexer_);
  preproc.retokenize(tokens, errors_);
  if (!isErrorsOk()) {
    return "";
  }

  return file + ".o";
}

bool Aycc::readCFile(const std::string& file, std::vector<char>& buffer) {
  std::ifstream ifst(file);
  if (!ifst.is_open()) {
    return false;
  }

  for (char c; ifst.get(c); buffer.push_back(c)) {
  }

  return true;
}

void Aycc::showErrors() {
  std::for_each(errors_.begin(), errors_.end(),
                [](const CompilerError& ce) { std::cout << ce << std::endl; });
}

void Aycc::showTokens(const std::vector<Token>& tokens) {
  std::for_each(tokens.begin(), tokens.end(),
                [](const Token& tk) { std::cout << tk << std::endl; });
}

bool Aycc::isErrorsOk() {
  for (const auto& er : errors_) {
    if (!er.isWarning()) {
      return false;
    }
  }
  return true;
}
