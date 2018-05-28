#include "errors.h"

Position::Position(const std::string& file, size_t line, size_t col,
                   const std::string& full_line)
    : file_(file), line_(line), col_(col), full_line_(full_line) {}

Position& Position::operator++() {
  ++col_;
  return *this;
}

std::string Position::getFile() const { return file_; }

size_t Position::getLine() const { return line_; }

size_t Position::getColumn() const { return col_; }

std::string Position::getFullLine() const { return full_line_; }

Range::Range(const Position& be, const Position& en) : be_(be), en_(en) {}

Range operator+(const Range& lrg, const Range& rrg) {
  return Range(lrg.be_, rrg.en_);
}

Position Range::getBegin() const { return be_; }

Position Range::getEnd() const { return en_; }

CompilerError::CompilerError(const std::string& descrip,
                             std::shared_ptr<Range> range, bool warning)
    : descrip_(descrip), range_(range), warning_(warning) {}

const char* CompilerError::what() const throw() {
  std::string location = "";
  std::string type = (warning_) ? ("warning") : ("error");

  if (range_.get() != nullptr) {
    location += range_->getBegin().getFile() + ": ";
    location += "[ (" + std::to_string(range_->getBegin().getLine()) + "," +
                std::to_string(range_->getBegin().getColumn()) + ") ";
    location += "(" + std::to_string(range_->getEnd().getLine()) + "," +
                std::to_string(range_->getEnd().getColumn()) + ") ] ";
  }

  std::string info = "";
  info += "[" + type + "] " + location + descrip_;
  return info.c_str();
}

bool CompilerError::isWarning() const { return warning_; }

bool operator<(const CompilerError& lce, const CompilerError& rce) {
  // nullptr is before with range
  if (lce.range_.get() == nullptr) {
    return rce.range_.get() != nullptr;
  }

  if (rce.range_.get() == nullptr) {
    return false;
  }

  return (lce.range_->getBegin().getLine() <
          rce.range_->getBegin().getLine()) ||
         ((lce.range_->getBegin().getLine() ==
           rce.range_->getBegin().getLine()) &&
          (lce.range_->getBegin().getColumn() <
           rce.range_->getBegin().getColumn()));
}

std::ostream& operator<<(std::ostream& os, const CompilerError& ce) {
  std::string location = "";
  std::string type = (ce.warning_) ? ("warning") : ("error");

  if (ce.range_.get() != nullptr) {
    location += ce.range_->getBegin().getFile() + ": ";
    location += "[ (" + std::to_string(ce.range_->getBegin().getLine()) + "," +
                std::to_string(ce.range_->getBegin().getColumn()) + ") ";
    location += "(" + std::to_string(ce.range_->getEnd().getLine()) + "," +
                std::to_string(ce.range_->getEnd().getColumn()) + ") ] ";
  }

  os << "[" << type << "] " << location << ce.descrip_;
  return os;
}

Tagged::Tagged(char c, const Position& p) : c_(c), p_(p), r_(p_, p_) {}

char Tagged::getC() const { return c_; }

Position Tagged::getPosition() const { return p_; }

Range Tagged::getRange() const { return r_; }