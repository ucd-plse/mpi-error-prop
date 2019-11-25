#include "CompressRules.hpp"
#include "CountRules.hpp"
#include "InitializeCounters.hpp"
#include "PrintRules.hpp"
#include "StrX.hpp"
#include "XercesInitializer.hpp"

#include <xercesc/sax2/SAX2XMLReader.hpp>
#include <xercesc/sax2/XMLReaderFactory.hpp>
#include <xercesc/util/OutOfMemoryException.hpp>

#include <iostream>
#include <memory>


static void parseFile(const char filename[], DefaultHandler &handler)
{
  const auto_ptr<SAX2XMLReader> parser(XMLReaderFactory::createXMLReader());

  parser->setContentHandler(&handler);
  parser->setErrorHandler(&handler);
  parser->parse(filename);
}


int main(int argc, char *argv[])
{
  if (argc != 2)
    {
      cerr << "Usage: " << argv[0] << " original.wpds >compressed.wpds\n";
      return 1;
    }

  const char * const filename = argv[1];

  try
    {
      const XercesInitializer xercesInitializer;

      InitializeCounters initializer;
      parseFile(filename, initializer);

      CountRules counter;
      parseFile(filename, counter);

      CompressRules compressor;
      parseFile(filename, compressor);

      PrintRules printer(cout);
      parseFile(filename, printer);
    }
  catch (const OutOfMemoryException &bad)
    {
      cerr << "out of memory: " << StrX(bad.getMessage()) << '\n';
      return 2;
    }
  catch (const XMLException& bad)
    {
      cerr << "XML error: " << StrX(bad.getMessage()) << '\n';
      return 3;
    }
  catch (const SAXException& bad)
    {
      cerr << "SAX error: " << StrX(bad.getMessage()) << '\n';
      return 4;
    }

  return 0;
}
