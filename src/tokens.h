#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "errors.h"

#ifndef SRC_TOKENS_H_
#define SRC_TOKENS_H_

enum class TokenKind {
  IDENTIFIER = 0,
  NUMBER,
  STRING,
  CHAR,
  INCLUDE,
  // KEYWORD
  KEY_BOOL,
  KEY_CHAR,
  KEY_SHORT,
  KEY_INT,
  KEY_LONG,
  KEY_SIGNED,
  KEY_UNSIGNED,
  KEY_VOID,
  KEY_RETURN,
  KEY_IF,
  KEY_ELSE,
  KEY_WHILE,
  KEY_FOR,
  KEY_BREAK,
  KEY_CONTINUE,
  KEY_AUTO,
  KEY_STATIC,
  KEY_EXTERN,
  KEY_STRUCT,
  KEY_UNION,
  KEY_CONST,
  KEY_TYPEDEF,
  KEY_SIZEOF,
  // SYMBOL
  SB_ADD,
  SB_MIN,
  SB_MUL,
  SB_DIV,
  SB_MOD,
  SB_INC,
  SB_DEC,
  SB_EQU,
  SB_EQUADD,
  SB_EQUMIN,
  SB_EQUMUL,
  SB_EQUDIV,
  SB_EQUMOD,
  SB_EQ,
  SB_NE,
  SB_LOGAND,
  SB_LOGOR,
  SB_NOT,
  SB_LT,
  SB_GT,
  SB_LE,
  SB_GE,
  SB_AND,
  SB_POUND,
  SB_SAL,
  SB_SAR,
  SB_NEG,
  SB_DQUOTE,
  SB_SQUOTE,
  SB_LL_BCT,
  SB_RL_BCT,
  SB_LB_BCT,
  SB_RB_BCT,
  SB_LM_BCT,
  SB_RM_BCT,
  SB_COMMA,
  SB_SEMI,
  SB_DOT,
  SB_ARROW,
  NOT_A_KIND
};

using TokenPair = std::pair<TokenKind, std::string>;

class Token {
 public:
  Token(const TokenKind& kind = TokenKind::NOT_A_KIND,
        const std::string& content = "", const std::string& rep = "",
        std::shared_ptr<Range> range = std::shared_ptr<Range>(nullptr));

 public:
  TokenKind getTokenKind() const;
  std::string getContent() const;
  std::string getRep() const;
  std::shared_ptr<Range> getRange() const;

  static TokenPair findSymbolKind(const std::vector<Tagged>& taggedline,
                                  size_t start_index);

  static TokenPair findKeyWordKind(const std::vector<Tagged>& taggedline,
                                   size_t start_index, size_t end_index);

  static std::string findNumber(const std::vector<Tagged>& taggedline,
                                size_t start_index, size_t end_index);

  static std::string findIdentifier(const std::vector<Tagged>& taggedline,
                                    size_t start_index, size_t end_index);

  friend std::ostream& operator<<(std::ostream& os, const Token& ce);

 private:
  void tokenPairInit();

 private:
  TokenKind kind_;
  std::string content_;
  std::string rep_;
  std::shared_ptr<Range> range_;

 private:
  static const std::vector<TokenPair> keyword_kinds_;
  static const std::vector<TokenPair> symbol_kinds_;
};

std::ostream& operator<<(std::ostream& os, const Token& tk);
#endif  // SRC_TOKENS_H_
