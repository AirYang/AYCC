#include "lexer.h"

#include <cctype>

#include <algorithm>
#include <iostream>
#include <sstream>
#include <unordered_map>
#include <unordered_set>

Lexer::Lexer(const std::vector<char>& buffer, const std::string& filename,
             bool need_lexer)
    : buffer_(buffer), filename_(filename), need_lexer_(need_lexer) {}

void Lexer::tokenize(std::vector<Token>& tokens,
                     std::vector<CompilerError>& errors) {
  std::vector<std::vector<Tagged>> taggedlines;
  splitToTagged(taggedlines);
  joinExtendedLine(taggedlines);

  size_t count = 0;
  bool in_comment = false;
  if (need_lexer_) {
    std::cout << "----- ----- < " << filename_ << " tokens > ----- -----"
              << std::endl;
  }
  for (const auto& taggedline : taggedlines) {
    try {
      std::vector<Token> linetokens;
      tokenizeLine(taggedline, in_comment, linetokens, errors);
      // only for debug
      if (need_lexer_) {
        std::for_each(linetokens.begin(), linetokens.end(),
                      [&count](const Token& tk) {
                        std::cout << "    [" << count << "]" << tk << std::endl;
                      });
      }

      tokens.insert(tokens.end(), linetokens.begin(), linetokens.end());
      ++count;
    } catch (const CompilerError& e) {
      errors.push_back(e);
    }
  }
  if (need_lexer_) {
    std::cout << "----- ----- ----- < "
              << " > ----- ----- -----" << std::endl;
  }
}

void Lexer::splitToTagged(std::vector<std::vector<Tagged>>& taggedlines) {
  std::stringstream ss;
  std::for_each(buffer_.begin(), buffer_.end(), [&ss](char c) { ss << c; });

  for (std::string line; std::getline(ss, line);) {
    // for debug
    // std::cout << "[" << taggedlines.size() << "]" << line << std::endl;

    std::vector<Tagged> taggedline;
    for (const auto& c : line) {
      Position p(filename_, taggedlines.size() + 1, taggedline.size() + 1,
                 line);
      taggedline.push_back(Tagged(c, p));
    }
    taggedlines.push_back(taggedline);
  }
}

void Lexer::joinExtendedLine(std::vector<std::vector<Tagged>>& taggedlines) {
  for (size_t i = 0; i < taggedlines.size(); i++) {
    if ((taggedlines[i].size() > 0) &&
        (taggedlines[i][taggedlines[i].size() - 1].getC() == '\\')) {
      if (i + 1 < taggedlines.size()) {
        taggedlines[i].pop_back();
        taggedlines[i].insert(taggedlines[i].end(), taggedlines[i + 1].begin(),
                              taggedlines[i + 1].end());
        taggedlines.erase(taggedlines.begin() + i + 1);
        --i;
      } else {
        taggedlines.erase(taggedlines.begin() + i);
      }
    }
  }
}

void Lexer::tokenizeLine(const std::vector<Tagged>& taggedline,
                         bool& in_comment, std::vector<Token>& linetokens,
                         std::vector<CompilerError>& errors) {
  size_t chunk_start = 0;
  size_t chunk_end = 0;

  bool include_line = false;
  bool seen_filename = false;

  while (chunk_end < taggedline.size()) {
    TokenPair symbol_kind = Token::findSymbolKind(taggedline, chunk_end);
    TokenPair next_symbol_kind =
        Token::findSymbolKind(taggedline, chunk_end + 1);

    if (matchIncludeCommand(linetokens)) {
      include_line = true;
    }

    // end comment
    if (in_comment) {
      if ((symbol_kind.first == TokenKind::SB_MUL) &&
          (next_symbol_kind.first == TokenKind::SB_DIV)) {
        in_comment = false;
        chunk_start = chunk_end + 2;
        chunk_end = chunk_start;
      } else {
        chunk_start = chunk_end + 1;
        chunk_end = chunk_start;
      }
    }
    // begin comment
    else if ((symbol_kind.first == TokenKind::SB_DIV) &&
             (next_symbol_kind.first == TokenKind::SB_MUL)) {
      chunkToToken(taggedline, chunk_start, chunk_end, linetokens);
      in_comment = true;
    }
    // single comment
    else if ((symbol_kind.first == TokenKind::SB_DIV) &&
             (next_symbol_kind.first == TokenKind::SB_DIV)) {
      break;
    }
    // skip blank
    else if (isblank(taggedline[chunk_end].getC())) {
      chunkToToken(taggedline, chunk_start, chunk_end, linetokens);
      chunk_start = chunk_end + 1;
      chunk_end = chunk_start;
    }
    // include line
    else if (include_line) {
      if (seen_filename) {
        throw CompilerError(
            "extra tokens at end of include directive",
            std::make_shared<Range>(taggedline[chunk_end].getRange().getBegin(),
                                    taggedline[chunk_end].getRange().getEnd()));
      }

      std::pair<std::string, size_t> read_result =
          readIncludeFilename(taggedline, chunk_end);
      linetokens.push_back(
          Token(TokenKind::INCLUDE, read_result.first, "",
                std::make_shared<Range>(
                    taggedline[chunk_end].getPosition(),
                    taggedline[read_result.second].getPosition())));
      chunk_start = read_result.second + 1;
      chunk_end = chunk_start;
      seen_filename = true;
    }
    // string
    else if ((symbol_kind.first == TokenKind::SB_DQUOTE) ||
             (symbol_kind.first == TokenKind::SB_SQUOTE)) {
      char quote;
      TokenKind kind;
      // bool add_null;
      if (symbol_kind.first == TokenKind::SB_DQUOTE) {
        quote = '\"';
        kind = TokenKind::STRING;
        // add_null = true;
      } else {
        quote = '\'';
        kind = TokenKind::CHAR;
        // add_null = false;
      }
      std::pair<std::string, size_t> read_result =
          readString(taggedline, chunk_end + 1, quote);
      std::string rep = "";
      for (size_t index = chunk_end; index < read_result.second + 1; ++index) {
        rep += std::string(1, taggedline[index].getC());
      }
      std::shared_ptr<Range> range =
          std::make_shared<Range>(taggedline[chunk_end].getPosition(),
                                  taggedline[read_result.second].getPosition());
      if ((kind == TokenKind::CHAR) && (read_result.first.size() == 0)) {
        errors.push_back(CompilerError("empty character constant", range));
      } else if ((kind == TokenKind::CHAR) && (read_result.first.size() > 1)) {
        errors.push_back(
            CompilerError("multiple characters in character constant", range));
      }
      linetokens.push_back(Token(kind, read_result.first, rep, range));
      chunk_start = read_result.second + 1;
      chunk_end = chunk_start;
    } else if (symbol_kind.first != TokenKind::NOT_A_KIND) {
      size_t symbol_start_index = chunk_end;
      size_t symbol_end_index = chunk_end + symbol_kind.second.size() - 1;

      Token symbol_token = Token(
          symbol_kind.first, "", "",
          std::make_shared<Range>(taggedline[symbol_start_index].getPosition(),
                                  taggedline[symbol_end_index].getPosition()));
      chunkToToken(taggedline, chunk_start, chunk_end, linetokens);
      linetokens.push_back(symbol_token);

      chunk_start = chunk_end + symbol_kind.second.size();
      chunk_end = chunk_start;
    } else {
      ++chunk_end;
    }
  }

  chunkToToken(taggedline, chunk_start, chunk_end, linetokens);
  if (((include_line) || (matchIncludeCommand(linetokens))) &&
      (!seen_filename)) {
    readIncludeFilename(taggedline, chunk_end);
  }
}

bool Lexer::matchIncludeCommand(const std::vector<Token>& linetokens) {
  return (linetokens.size() == 2) &&
         (linetokens[0].getTokenKind() == TokenKind::SB_POUND) &&
         (linetokens[1].getTokenKind() == TokenKind::IDENTIFIER) &&
         (!linetokens[1].getContent().compare("include"));
}

void Lexer::chunkToToken(const std::vector<Tagged>& taggedline,
                         size_t chunk_start, size_t chunk_end,
                         std::vector<Token>& linetokens) {
  if (chunk_start < chunk_end) {
    std::shared_ptr<Range> range =
        std::make_shared<Range>(taggedline[chunk_start].getPosition(),
                                taggedline[chunk_end - 1].getPosition());
    TokenPair keyword_kind =
        Token::findKeyWordKind(taggedline, chunk_start, chunk_end);
    if (keyword_kind.first != TokenKind::NOT_A_KIND) {
      linetokens.push_back(
          Token(keyword_kind.first, keyword_kind.second, "", range));
      return;
    }

    std::string number = Token::findNumber(taggedline, chunk_start, chunk_end);
    if (number.compare("")) {
      linetokens.push_back(Token(TokenKind::NUMBER, number, "", range));
      return;
    }

    std::string identifier =
        Token::findIdentifier(taggedline, chunk_start, chunk_end);
    if (identifier.compare("")) {
      linetokens.push_back(Token(TokenKind::IDENTIFIER, identifier, "", range));
      return;
    }

    std::string unreg_chunk = "";
    for (; chunk_start < chunk_end; ++chunk_start) {
      unreg_chunk += std::string(1, taggedline[chunk_start].getC());
    }
    // std::string unreg_chunk = "";
    // std::getline(ss, unreg_chunk);
    throw CompilerError("unrecognized token at " + unreg_chunk, range);
  }
}

std::pair<std::string, size_t> Lexer::readIncludeFilename(
    const std::vector<Tagged>& taggedline, size_t start_index) {
  char end_flag;
  if ((start_index < taggedline.size()) &&
      (taggedline[start_index].getC() == '\"')) {
    end_flag = '\"';
  } else if ((start_index < taggedline.size()) &&
             (taggedline[start_index].getC() == '<')) {
    end_flag = '>';
  } else {
    size_t tgi;

    if (start_index < taggedline.size()) {
      tgi = start_index;
    } else {
      tgi = taggedline.size() - 1;
    }

    throw CompilerError(
        "expected \"FILENAME\" or <FILENAME> after include directive",
        std::make_shared<Range>(taggedline[tgi].getPosition(),
                                taggedline[tgi].getPosition()));
  }

  size_t index = start_index + 1;
  try {
    while (taggedline[index].getC() != end_flag) {
      ++index;
    }
  } catch (const std::exception& e) {
    throw CompilerError(
        "missing terminating character for include filename",
        std::make_shared<Range>(taggedline[start_index].getPosition(),
                                taggedline[start_index].getPosition()));
  }

  std::string includename = "";
  for (; start_index < index + 1; ++start_index) {
    includename += std::string(1, taggedline[start_index].getC());
  }
  return {includename, index};
}

std::pair<std::string, size_t> Lexer::readString(
    const std::vector<Tagged>& taggedline, size_t start_index, char delim) {
  size_t index = start_index;
  std::string str;
  static const std::unordered_map<char, size_t> escapes{
      {'\'', 39}, {'\"', 34}, {'?', 63}, {'\\', 92}, {'a', 7}, {'b', 8},
      {'f', 12},  {'n', 10},  {'r', 13}, {'t', 9},   {'v', 11}};
  static const std::unordered_set<char> octdigits{'0', '1', '2', '3',
                                                  '4', '5', '6', '7'};
  static const std::unordered_set<char> hexdigits = {
      '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a',
      'b', 'c', 'd', 'e', 'f', 'A', 'B', 'C', 'D', 'E', 'F'};

  while (true) {
    if (index >= taggedline.size()) {
      throw CompilerError(
          "missing terminating quote",
          std::make_shared<Range>(taggedline[start_index].getPosition(),
                                  taggedline[start_index].getPosition()));
    } else if (taggedline[index].getC() == delim) {
      return {str, index};
    } else if (((index + 1) < taggedline.size()) &&
               (taggedline[index].getC() == '\\') &&
               (escapes.find(taggedline[index + 1].getC()) != escapes.end())) {
      str += std::string(
          1, static_cast<char>(escapes.at(taggedline[index + 1].getC())));
      index += 2;
    } else if ((index + 1 < taggedline.size()) &&
               (taggedline[index].getC() == '\\') &&
               (octdigits.find(taggedline[index + 1].getC()) !=
                octdigits.end())) {
      size_t octol_size = 1;
      size_t octal = taggedline[index + 1].getC() - '0';
      index += 2;
      while ((index < taggedline.size()) && (octol_size < 3) &&
             (octdigits.find(taggedline[index].getC()) != octdigits.end())) {
        octal = octal * 8 + (taggedline[index].getC() - '0');
        ++index;
        ++octol_size;
      }
      str += std::string(1, static_cast<char>(octal));
    } else if ((index + 2 < taggedline.size()) &&
               (taggedline[index].getC() == '\\') &&
               (taggedline[index + 1].getC() == 'x') &&
               (hexdigits.find(taggedline[index + 2].getC()) !=
                hexdigits.end())) {
      size_t hexa = 0;
      if (isdigit(taggedline[index + 2].getC())) {
        hexa = taggedline[index + 2].getC() - '0';
      } else {
        hexa = tolower(taggedline[index + 2].getC()) - 'a' + 10;
      }
      index += 3;
      while ((index < taggedline.size()) &&
             (hexdigits.find(taggedline[index].getC()) != hexdigits.end())) {
        if (isdigit(taggedline[index].getC())) {
          hexa = hexa * 16 + taggedline[index].getC() - '0';
        } else {
          hexa = hexa * 16 + tolower(taggedline[index].getC()) - 'a' + 10;
        }
        ++index;
      }
      str += std::string(1, static_cast<char>(hexa));
    } else {
      str += std::string(1, static_cast<char>(taggedline[index].getC()));
      ++index;
    }
  }
}