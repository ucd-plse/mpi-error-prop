#ifndef WEIGHT_INFO_GUARD
#define WEIGHT_INFO_GUARD 1

#include <string>
#include <vector>
using namespace std;

struct WeightInfo {
  string weight1;
  string weight2;
  bool trusted;

  string function;
  string callee; //only used for ErrorPropagationPushRule weights
}; 

#endif
