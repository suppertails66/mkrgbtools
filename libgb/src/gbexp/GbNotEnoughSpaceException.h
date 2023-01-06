#ifndef GBNOTENOUGHSPACEEXCEPTION_H
#define GBNOTENOUGHSPACEEXCEPTION_H


#include "exception/TException.h"
#include <string>

namespace Gb {


class GbNotEnoughSpaceException : public BlackT::TException {
public:
  GbNotEnoughSpaceException(const char* nameOfSourceFile__,
                   int lineNum__,
                   const std::string& source__,
                   const std::string& problem__);
  
  const char* what() const throw();
  
  std::string problem() const;
protected:

  std::string problem_;
  
};


};


#endif 
