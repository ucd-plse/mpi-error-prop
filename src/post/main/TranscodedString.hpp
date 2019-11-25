#ifndef TRANSCODED_STRING_GUARD
#define TRANSCODED_STRING_GUARD 1

#include <xercesc/util/XMLString.hpp>


class TranscodedString {
public:
  TranscodedString(const char []);
  ~TranscodedString();
  operator const XMLCh *() const;

private:
  XMLCh *transcoded;
};


inline
TranscodedString::TranscodedString(const char toTranscode[])
  : transcoded(XERCES_CPP_NAMESPACE_QUALIFIER XMLString::transcode(toTranscode))
{
}


inline
TranscodedString::~TranscodedString()
{
  XERCES_CPP_NAMESPACE_QUALIFIER XMLString::release(&transcoded);
}


inline
TranscodedString::operator const XMLCh *() const
{
  return transcoded;
}


#endif // !TRANSCODED_STRING_GUARD
