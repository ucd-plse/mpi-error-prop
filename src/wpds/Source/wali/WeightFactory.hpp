#ifndef wali_WEIGHT_FACTORY_GUARD
#define wali_WEIGHT_FACTORY_GUARD 1

/**
 * @author Nicholas Kidd
 */

#include "wali/Common.hpp"
#include "wali/SemElem.hpp"
#include <queue>
#include <string>

struct WeightInfo;

namespace wali
{
  class WeightFactory
  {
    public:
      WeightFactory();

      virtual ~WeightFactory();

      virtual wali::sem_elem_t getWeight( std::string s ) = 0;
      // added by Cindy
      virtual wali::sem_elem_t getWeight( std::queue<WeightInfo>& weightInfos, int line, std::string file ) = 0;

  }; // class WeightFactory

} // namespace wali

#endif  // wali_WEIGHT_FACTORY_GUARD

