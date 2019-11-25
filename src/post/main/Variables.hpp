#ifndef VARIABLES_GUARD
#define VARIABLES_GUARD 1

#include <memory>
#include <string>
#include <vector>
#include <map>
#include <set>

#include "Variable.hpp"

using namespace std;

class Variables {
public:
  Variables(const string &errorCodes);
  void add(const string &id, const string &name);
  void add(const string &id);
  int getIndex(const string &v);
  const string &getId(unsigned int i);
  string getRealName(unsigned int i);
  void print(std::ostream& out);

  void addGlobal(const string &v);
  bool isGlobal(const string &v);
  bool isGlobal(unsigned int i);

  void addLocal(const string &v);
  int numLocals() const;

  void addPointer(const string &v);
  bool isPointer(const string &v);
  bool isPointer(unsigned int i);

private:
  vector<Variable> details;
  map<string, int> vars;
  set<string> globals;
  set<string> locals;
  set<string> pointers;
};


extern std::auto_ptr<Variables> variables;
extern std::vector<std::string> errorNames;
extern std::vector<std::string> tentativeErrorNames;


////////////////////////////////////////////////////////////////////////


inline void Variables::add(const std::string &id)
{
  add(id, id);
}


#endif //VARIABLES_GUARD
