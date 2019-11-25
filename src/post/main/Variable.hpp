#ifndef VARIABLE_GUARD
#define VARIABLE_GUARD 1

#include <string>


struct Variable {
public:
  Variable(const std::string &id, const std::string &name);

  std::string id;
  std::string name;
};


////////////////////////////////////////////////////////////////////////


inline
Variable::Variable(const std::string &id, const std::string &name)
  : id(id),
    name(name)
{
}


#endif //VARIABLE_GUARD
