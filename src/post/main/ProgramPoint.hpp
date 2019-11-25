#ifndef PROGRAM_POINT_GUARD
#define PROGRAM_POINT_GUARD 1

#include <iosfwd>
#include <string>
#include <vector>
#include <map>
#include "ErrorPropagation.hpp"
#include "Main.hpp"
#include "wali/wfa/WFA.hpp"
#include "wali/wfa/State.hpp"
#include "wali/witness/Witness.hpp"
#include "Variables.hpp"
#include "fdd.h"
#include "wali/SemElem.hpp"

using namespace std;


/*Some external variables to be used by derived classes of this class*/
extern bdd bddErrors;
extern bdd bddNonTentativeErrors;
extern bdd bddTentativeErrors;
extern bool report_tentative;
extern bool report_temps;


class ProgramPoint {

public:
  ProgramPoint(string f, int l, string c, string m, wali::Key k) :
    file(f), line(l), caller(c), msgcode(m), fromStkKey(k), weight(NULL) {}

  virtual ~ProgramPoint() {}

  void calculateWeight(const wali::wfa::WFA& result, const map<wali::Key, wali::wfa::ITrans*>& mapTrans);
  virtual void process() = 0;

  friend std::ostream& operator << (std::ostream &, const ProgramPoint &);

 protected:
  int getError(bdd &targetToErrors);
  //string getDroppedErrors(bdd targetToErrors, int const target, int const error);
  vector<string> getErrors(bdd targetToErrors, int const target);

  string file;
  int line;
  string caller;
  string msgcode;
  wali::Key fromStkKey;
  const ErrorPropagation* weight;
  wali::witness::Witness* witness;

private:
  wali::sem_elem_t weight_for_fromStkKey;

};

#endif //PROGRAM_POINT_GUARD
