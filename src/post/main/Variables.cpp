#include "Variables.hpp"
#include <cassert>
#include <cstdlib>
#include <iostream>
#include <fstream>
#include "fdd.h"
#include "Main.hpp"

using namespace std;
vector<string> errorNames; //has all errors include tentative errors
vector<string> tentativeErrorNames;

auto_ptr<Variables> variables;


Variables::Variables(const string &errorCodes) {

  // Adding OK and uninitialized
  add("OK");
  add("UNINITIALIZED");

  // Reading error names from file
  ifstream inFile(errorCodes.c_str());
  string name;
  int value;

  if (!inFile) {
    std::cerr << "Unable to open " << errorCodes << '\n';
    exit(1);
  }

  while(inFile >> name >> value) {
    errorNames.push_back(name);
    add(name);

    if (!hexastore) {
      tentativeErrorNames.push_back("TENTATIVE_" + name);
      add("TENTATIVE_" + name);
    }
  }

  for(unsigned int i = 0; i < tentativeErrorNames.size(); i++) {  //we want the tentative errors to be at the end of all errors.
    errorNames.push_back(tentativeErrorNames[i]);
  }
 
}


void Variables::add(const string &id, const string &name) {
  if (!id.empty() && vars.find(id) == vars.end()) {
    Variable detail(id, name.empty() ? id : name);
    vars[id] = details.size();
    details.push_back(detail);
  }
  return;
}


void Variables::addGlobal(const string &v) {
  if (!v.empty()) {
    globals.insert(v);
  }
  return;
}


bool Variables::isGlobal(const string &v) {
  return globals.find(v) != globals.end();
}


bool Variables::isGlobal(unsigned int i) {
  return isGlobal(getId(i));
}


void Variables::addLocal(const string &v) {
  assert(!v.empty());
  assert(locals.find(v) == locals.end());
  locals.insert(v);
}


int Variables::numLocals() const {
  return locals.size();
}


void Variables::addPointer(const string &v) {
  if (!v.empty()) {
    pointers.insert(v);
  }
  return;
}


bool Variables::isPointer(const string &v) {
  return pointers.find(v) != pointers.end();
}


bool Variables::isPointer(unsigned int i) {
  return isPointer(getId(i));
}


int Variables::getIndex(const string &v) {
  
  if (vars.find(v) != vars.end()) {
    return vars[v];
  }
  // if not found, return -1 and do not add it to the map
  return -1;
}


const string &Variables::getId(unsigned int i) {
  return details.at(i).id;
}


string Variables::getRealName(unsigned int i) {
  const string &name = details.at(i).name;
  int pos = name.find('#');
  return name.substr(pos+1); /*get from *+1 to end*/    
}


void Variables::print( std::ostream& out) {
  for (unsigned int i = 0; i < details.size(); ++i)
    out << i << ' ' << getId(i) << std::endl;
}
