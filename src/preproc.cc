#include "preproc.h"

#include <fstream>

#include <boost/filesystem.hpp>

PreProc::PreProc(const std::string& filename, bool need_lexer)
    : filename_(filename), need_lexer_(need_lexer) {}

void PreProc::retokenize(std::vector<Token>& tokens,
                         std::vector<CompilerError>& errors) {
  size_t index = 0;
  std::vector<Token> processed;
  for (; index < tokens.size() - 2;) {
    if ((tokens[index].getTokenKind() == TokenKind::SB_POUND) &&
        (tokens[index + 1].getTokenKind() == TokenKind::IDENTIFIER) &&
        (!tokens[index + 1].getContent().compare("include")) &&
        (tokens[index + 2].getTokenKind() == TokenKind::INCLUDE)) {
      try {
        std::vector<char> includefilebuffer;
        std::string includefilepath =
            readInludeFile(tokens[index + 2].getContent(), includefilebuffer);

        std::vector<Token> includetokens;
        Lexer includelexer(includefilebuffer, includefilepath, need_lexer_);
        includelexer.tokenize(includetokens, errors);

        PreProc preproc(includefilepath, need_lexer_);
        preproc.retokenize(includetokens, errors);

        processed.insert(processed.end(), includetokens.begin(),
                         includetokens.end());
      } catch (std::exception& ec) {
        errors.push_back(CompilerError("unable to read included file",
                                       tokens[index + 2].getRange()));
      }
      index += 3;
    } else {
      processed.push_back(tokens[index]);
      ++index;
    }
  }

  for (; index < tokens.size(); ++index) {
    processed.push_back(tokens[index]);
  }

  tokens = processed;
}

std::string PreProc::readInludeFile(const std::string& includefile,
                                    std::vector<char>& buffer) {
  namespace bf = boost::filesystem;
  bf::path includepath(filename_);

  // Standard library in include
  // Be compiled code in test

  // self include
  if (includefile[0] == '\"') {
    includepath = includepath.parent_path().append(includefile.begin() + 1,
                                                   includefile.end() - 1);
    std::cout << includepath.string() << std::endl;
  }
  // standard include
  else {
    includepath =
        bf::path(__FILE__).parent_path().parent_path().append("include").append(
            includefile.begin() + 1, includefile.end() - 1);
    std::cout << includepath.string() << std::endl;
  }

  std::ifstream ifst(includepath.string());
  if (!ifst.is_open()) {
    throw CompilerError("could't open include file " + includefile);
  }

  for (char c; ifst.get(c); buffer.push_back(c)) {
  }

  return includepath.string();
}