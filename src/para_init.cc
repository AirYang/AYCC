#include "para_init.h"

ParaInit::ParaInit(int argc, char** argv) : parser_(argc, argv) {
  parserInit();
  parser_.run_and_exit_if_error();
}

void ParaInit::parserInit() {
  parser_.set_optional<bool>("l", "lexer", false, "Need print lexer result");
  parser_.set_required<std::vector<std::string>>("f", "files",
                                                 "Input files [.c] or [.o]");
}

std::vector<std::string> ParaInit::getFiles() {
  return parser_.get<std::vector<std::string>>("f");
}

bool ParaInit::needLexer() { return parser_.get<bool>("l"); }