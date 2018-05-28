#include "tokens.h"

#include <cctype>
#include <regex>
#include <sstream>

const std::vector<TokenPair> Token::keyword_kinds_{
    {TokenKind::KEY_BOOL, "_Bool"},        {TokenKind::KEY_CHAR, "char"},
    {TokenKind::KEY_SHORT, "short"},       {TokenKind::KEY_INT, "int"},
    {TokenKind::KEY_LONG, "long"},         {TokenKind::KEY_SIGNED, "signed"},
    {TokenKind::KEY_UNSIGNED, "unsigned"}, {TokenKind::KEY_VOID, "void"},
    {TokenKind::KEY_RETURN, "return"},     {TokenKind::KEY_IF, "if"},
    {TokenKind::KEY_ELSE, "else"},         {TokenKind::KEY_WHILE, "while"},
    {TokenKind::KEY_FOR, "for"},           {TokenKind::KEY_BREAK, "break"},
    {TokenKind::KEY_CONTINUE, "continue"}, {TokenKind::KEY_AUTO, "auto"},
    {TokenKind::KEY_STATIC, "static"},     {TokenKind::KEY_EXTERN, "extern"},
    {TokenKind::KEY_STRUCT, "struct"},     {TokenKind::KEY_UNION, "union"},
    {TokenKind::KEY_CONST, "const"},       {TokenKind::KEY_TYPEDEF, "typedef"},
    {TokenKind::KEY_SIZEOF, "sizeof"}};

const std::vector<TokenPair> Token::symbol_kinds_{
    {TokenKind::SB_ADD, "+"},     {TokenKind::SB_MIN, "-"},
    {TokenKind::SB_MUL, "*"},     {TokenKind::SB_DIV, "/"},
    {TokenKind::SB_MOD, "%"},     {TokenKind::SB_INC, "++"},
    {TokenKind::SB_DEC, "--"},    {TokenKind::SB_EQU, "="},
    {TokenKind::SB_EQUADD, "+="}, {TokenKind::SB_EQUMIN, "-="},
    {TokenKind::SB_EQUMUL, "*="}, {TokenKind::SB_EQUDIV, "/="},
    {TokenKind::SB_EQUMOD, "%="}, {TokenKind::SB_EQ, "=="},
    {TokenKind::SB_NE, "!="},     {TokenKind::SB_LOGAND, "&&"},
    {TokenKind::SB_LOGOR, "||"},  {TokenKind::SB_NOT, "!"},
    {TokenKind::SB_LT, "<"},      {TokenKind::SB_GT, ">"},
    {TokenKind::SB_LE, "<="},     {TokenKind::SB_GE, ">="},
    {TokenKind::SB_AND, "&"},     {TokenKind::SB_POUND, "#"},
    {TokenKind::SB_SAL, "<<"},    {TokenKind::SB_SAR, ">>"},
    {TokenKind::SB_NEG, "~"},     {TokenKind::SB_DQUOTE, "\""},
    {TokenKind::SB_SQUOTE, "'"},  {TokenKind::SB_LL_BCT, "("},
    {TokenKind::SB_RL_BCT, ")"},  {TokenKind::SB_LB_BCT, "{"},
    {TokenKind::SB_RB_BCT, "}"},  {TokenKind::SB_LM_BCT, "["},
    {TokenKind::SB_RM_BCT, "]"},  {TokenKind::SB_COMMA, ","},
    {TokenKind::SB_SEMI, ";"},    {TokenKind::SB_DOT, "."},
    {TokenKind::SB_ARROW, "->"}};

Token::Token(const TokenKind& kind, const std::string& content,
             const std::string& rep, std::shared_ptr<Range> range)
    : kind_(kind), content_(content), rep_(rep), range_(range) {
  tokenPairInit();
}

TokenKind Token::getTokenKind() const { return kind_; }

std::string Token::getContent() const { return content_; }

std::string Token::getRep() const { return rep_; }

std::shared_ptr<Range> Token::getRange() const { return range_; }

void Token::tokenPairInit() {
  if (((kind_ != TokenKind::NOT_A_KIND) && (content_.compare(""))) ||
      ((kind_ == TokenKind::NOT_A_KIND) && (!content_.compare(""))) ||
      (kind_ <= TokenKind::INCLUDE)) {
    return;
  }

  for (const auto& tp : keyword_kinds_) {
    if (kind_ == TokenKind::NOT_A_KIND) {
      kind_ = (!content_.compare(tp.second)) ? (tp.first) : (kind_);
    } else if (!content_.compare("")) {
      content_ = (kind_ == tp.first) ? (tp.second) : (content_);
    }
  }

  for (const auto& tp : symbol_kinds_) {
    if (kind_ == TokenKind::NOT_A_KIND) {
      kind_ = (!content_.compare(tp.second)) ? (tp.first) : (kind_);
    } else if (!content_.compare("")) {
      content_ = (kind_ == tp.first) ? (tp.second) : (content_);
    }
  }
}

TokenPair Token::findSymbolKind(const std::vector<Tagged>& taggedline,
                                size_t start_index) {
  // size_t maxLength = 0;
  TokenPair tp = {TokenKind::NOT_A_KIND, ""};
  for (const auto& symbolkind : symbol_kinds_) {
    size_t length = 0;
    for (const auto& c : symbolkind.second) {
      if (((length + start_index) < taggedline.size()) &&
          (taggedline[length + start_index].getC() == c)) {
        ++length;
        continue;
      }
      break;
    }
    if ((length == symbolkind.second.size()) && (length > tp.second.size())) {
      tp = symbolkind;
    }
  }
  return tp;
}

TokenPair Token::findKeyWordKind(const std::vector<Tagged>& taggedline,
                                 size_t start_index, size_t end_index) {
  std::string word = "";
  for (; start_index < end_index; ++start_index) {
    word += std::string(1, taggedline[start_index].getC());
  }

  for (const auto& keyword_kind : keyword_kinds_) {
    if (!keyword_kind.second.compare(word)) {
      return keyword_kind;
    }
  }
  return {TokenKind::NOT_A_KIND, ""};
}

std::string Token::findNumber(const std::vector<Tagged>& taggedline,
                              size_t start_index, size_t end_index) {
  std::string number = "";
  for (; start_index < end_index; ++start_index) {
    if (!isdigit(taggedline[start_index].getC())) {
      return "";
    }
    number += std::string(1, taggedline[start_index].getC());
  }
  return number;
}

std::string Token::findIdentifier(const std::vector<Tagged>& taggedline,
                                  size_t start_index, size_t end_index) {
  std::string id = "";
  for (; start_index < end_index; ++start_index) {
    id += std::string(1, taggedline[start_index].getC());
  }

  std::regex re("[_a-zA-Z][_a-zA-Z0-9]*$");
  id = (std::regex_match(id, re)) ? id : "";
  return id;
}

static const char* TokenKindToStr(TokenKind tk) {
  {
#define TOKENKIND_TO_STR(x) \
  case x:                   \
    return (#x);

    switch (tk) {
      TOKENKIND_TO_STR(TokenKind::IDENTIFIER)
      TOKENKIND_TO_STR(TokenKind::NUMBER)
      TOKENKIND_TO_STR(TokenKind::STRING)
      TOKENKIND_TO_STR(TokenKind::CHAR)
      TOKENKIND_TO_STR(TokenKind::INCLUDE)
      // KEYWORD
      TOKENKIND_TO_STR(TokenKind::KEY_BOOL)
      TOKENKIND_TO_STR(TokenKind::KEY_CHAR)
      TOKENKIND_TO_STR(TokenKind::KEY_SHORT)
      TOKENKIND_TO_STR(TokenKind::KEY_INT)
      TOKENKIND_TO_STR(TokenKind::KEY_LONG)
      TOKENKIND_TO_STR(TokenKind::KEY_SIGNED)
      TOKENKIND_TO_STR(TokenKind::KEY_UNSIGNED)
      TOKENKIND_TO_STR(TokenKind::KEY_VOID)
      TOKENKIND_TO_STR(TokenKind::KEY_RETURN)
      TOKENKIND_TO_STR(TokenKind::KEY_IF)
      TOKENKIND_TO_STR(TokenKind::KEY_ELSE)
      TOKENKIND_TO_STR(TokenKind::KEY_WHILE)
      TOKENKIND_TO_STR(TokenKind::KEY_FOR)
      TOKENKIND_TO_STR(TokenKind::KEY_BREAK)
      TOKENKIND_TO_STR(TokenKind::KEY_CONTINUE)
      TOKENKIND_TO_STR(TokenKind::KEY_AUTO)
      TOKENKIND_TO_STR(TokenKind::KEY_STATIC)
      TOKENKIND_TO_STR(TokenKind::KEY_EXTERN)
      TOKENKIND_TO_STR(TokenKind::KEY_STRUCT)
      TOKENKIND_TO_STR(TokenKind::KEY_UNION)
      TOKENKIND_TO_STR(TokenKind::KEY_CONST)
      TOKENKIND_TO_STR(TokenKind::KEY_TYPEDEF)
      TOKENKIND_TO_STR(TokenKind::KEY_SIZEOF)
      // SYMBOL
      TOKENKIND_TO_STR(TokenKind::SB_ADD)
      TOKENKIND_TO_STR(TokenKind::SB_MIN)
      TOKENKIND_TO_STR(TokenKind::SB_MUL)
      TOKENKIND_TO_STR(TokenKind::SB_DIV)
      TOKENKIND_TO_STR(TokenKind::SB_MOD)
      TOKENKIND_TO_STR(TokenKind::SB_INC)
      TOKENKIND_TO_STR(TokenKind::SB_DEC)
      TOKENKIND_TO_STR(TokenKind::SB_EQU)
      TOKENKIND_TO_STR(TokenKind::SB_EQUADD)
      TOKENKIND_TO_STR(TokenKind::SB_EQUMIN)
      TOKENKIND_TO_STR(TokenKind::SB_EQUMUL)
      TOKENKIND_TO_STR(TokenKind::SB_EQUDIV)
      TOKENKIND_TO_STR(TokenKind::SB_EQUMOD)
      TOKENKIND_TO_STR(TokenKind::SB_EQ)
      TOKENKIND_TO_STR(TokenKind::SB_NE)
      TOKENKIND_TO_STR(TokenKind::SB_LOGAND)
      TOKENKIND_TO_STR(TokenKind::SB_LOGOR)
      TOKENKIND_TO_STR(TokenKind::SB_NOT)
      TOKENKIND_TO_STR(TokenKind::SB_LT)
      TOKENKIND_TO_STR(TokenKind::SB_GT)
      TOKENKIND_TO_STR(TokenKind::SB_LE)
      TOKENKIND_TO_STR(TokenKind::SB_GE)
      TOKENKIND_TO_STR(TokenKind::SB_AND)
      TOKENKIND_TO_STR(TokenKind::SB_POUND)
      TOKENKIND_TO_STR(TokenKind::SB_SAL)
      TOKENKIND_TO_STR(TokenKind::SB_SAR)
      TOKENKIND_TO_STR(TokenKind::SB_NEG)
      TOKENKIND_TO_STR(TokenKind::SB_DQUOTE)
      TOKENKIND_TO_STR(TokenKind::SB_SQUOTE)
      TOKENKIND_TO_STR(TokenKind::SB_LL_BCT)
      TOKENKIND_TO_STR(TokenKind::SB_RL_BCT)
      TOKENKIND_TO_STR(TokenKind::SB_LB_BCT)
      TOKENKIND_TO_STR(TokenKind::SB_RB_BCT)
      TOKENKIND_TO_STR(TokenKind::SB_LM_BCT)
      TOKENKIND_TO_STR(TokenKind::SB_RM_BCT)
      TOKENKIND_TO_STR(TokenKind::SB_COMMA)
      TOKENKIND_TO_STR(TokenKind::SB_SEMI)
      TOKENKIND_TO_STR(TokenKind::SB_DOT)
      TOKENKIND_TO_STR(TokenKind::SB_ARROW)
      // not a kind
      TOKENKIND_TO_STR(TokenKind::NOT_A_KIND)
    }

#undef TOKENKIND_TO_STR
    return "Unsupported Token Kind";
  }
}

std::ostream& operator<<(std::ostream& os, const Token& tk) {
  os << "[" << TokenKindToStr(tk.kind_) << "] "
     << "[" << tk.content_ << "] "
     << "[" << tk.rep_ << "]";
  return os;
}