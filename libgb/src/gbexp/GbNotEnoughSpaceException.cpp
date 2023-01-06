#include "gbexp/GbNotEnoughSpaceException.h"

namespace Gb {


GbNotEnoughSpaceException::GbNotEnoughSpaceException(const char* nameOfSourceFile__,
                                   int lineNum__,
                                   const std::string& source__,
                                   const std::string& problem__)
  : TException(nameOfSourceFile__,
                   lineNum__,
                   source__),
    problem_(problem__) { };

const char* GbNotEnoughSpaceException::what() const throw() {
  return problem_.c_str();
}

std::string GbNotEnoughSpaceException::problem() const {
  return problem_;
}


}; 
