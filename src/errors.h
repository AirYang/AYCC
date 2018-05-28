#include <exception>
#include <iostream>
#include <memory>
#include <string>

#ifndef SRC_ERRORS_H_
#define SRC_ERRORS_H_

/*
 Position representing a position in source code
 file_ - Name of file
 line_ - Line numer in file
 col_ - Column number in file
 full_line_ - Full text of the line
 */
class Position {
 public:
  Position(const std::string& file, size_t line, size_t col,
           const std::string& full_line);

 public:
  Position& operator++();

  std::string getFile() const;
  size_t getLine() const;
  size_t getColumn() const;
  std::string getFullLine() const;

 private:
  std::string file_;
  size_t line_;
  size_t col_;
  std::string full_line_;
};

/*
 Range representing a range in source code
 be_ - Start position
 en_ - End position
 */
class Range {
 public:
  Range(const Position& be, const Position& en);

 public:
  friend Range operator+(const Range& lrg, const Range& rrg);

  Position getBegin() const;
  Position getEnd() const;

 private:
  Position be_;
  Position en_;
};

Range operator+(const Range& lrg, const Range& rrg);

/*
 CompilerError representing compile-time errors
 descrip_ - Description of the error
 range_ - Range at which the error appears
 warning_ - True if this is a warning
 */
class CompilerError : public std::exception {
 public:
  CompilerError(const std::string& descrip,
                std::shared_ptr<Range> range = std::shared_ptr<Range>(nullptr),
                bool warning = false);

 public:
  virtual const char* what() const throw();
  bool isWarning() const;
  friend bool operator<(const CompilerError& lce, const CompilerError& rce);
  friend std::ostream& operator<<(std::ostream& os, const CompilerError& ce);

 private:
  std::string descrip_;
  std::shared_ptr<Range> range_;
  bool warning_;
};

bool operator<(const CompilerError& lce, const CompilerError& rce);
std::ostream& operator<<(std::ostream& os, const CompilerError& ce);

class Tagged {
 public:
  Tagged(char c, const Position& p);

 public:
  char getC() const;
  Position getPosition() const;
  Range getRange() const;

 private:
  char c_;
  Position p_;
  Range r_;
};
#endif  // SRC_ERRORS_H_
