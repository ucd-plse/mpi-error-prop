#ifndef INCLUDE_XERCES_INITIALIZER_H
#define INCLUDE_XERCES_INITIALIZER_H

#include <xercesc/util/PlatformUtils.hpp>


class XercesInitializer {
public:
  XercesInitializer();
  ~XercesInitializer();
};


XercesInitializer::XercesInitializer()
{
  XERCES_CPP_NAMESPACE_QUALIFIER XMLPlatformUtils::Initialize();
}


XercesInitializer::~XercesInitializer()
{
  XERCES_CPP_NAMESPACE_QUALIFIER XMLPlatformUtils::Terminate();
}


#endif // !INCLUDE_XERCES_INITIALIZER_H
