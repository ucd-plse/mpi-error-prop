#ifndef ERROR_PROPAGATION_GUARD
#define ERROR_PROPAGATION_GUARD 1

/*!
 * @author Cindy Rubio Gonzalez
 */

#include "wali/SemElem.hpp"
#include "wali/WeightFactory.hpp"
#include <string>
#include "fdd.h"
#include <map>
#include <wali/MergeFn.hpp>
#include "wali/SemElemPair.hpp"
#include <vector>
#include "WeightInfo.hpp"
#include <queue>
#include <fstream>
#include "Message.hpp"


using namespace std;

using wali::SemElem;
using wali::SemElemPair;
using wali::sem_elem_t;
using wali::merge_fn_t;

class ErrorPropagation : public wali::SemElem, public wali::WeightFactory {

public:
  static void initialize();

  ErrorPropagation();
  ErrorPropagation( bdd b);
  ErrorPropagation( bdd b, int l, string f, bool t, string fn );
  ErrorPropagation( sem_elem_t s);
  ErrorPropagation( sem_elem_t s, string f);
  
  
  virtual ~ErrorPropagation();
  
  sem_elem_t one() const;
  
  sem_elem_t zero() const;
  
  // zero is the annihilator for extend
  sem_elem_t extend( SemElem* rhs );
  
  // zero is neutral for combine
  sem_elem_t combine( SemElem* rhs );
  
  bool equal( SemElem* rhs ) const;
  
  std::ostream & print( std::ostream & o ) const;
  
  // For WeightFactory
  sem_elem_t getWeight( std::string s );
  virtual sem_elem_t getWeight( queue<WeightInfo>& weightInfos, int line, string file );

  string getFunction();
  bool hasError(const int var, const int error);
  bool hasConstants(const int var);
  vector<int> getVariables(const int target);
  vector<int> getConstants(const int target);
  bool isError(int target);
  bool isConstant(int target);
  MsgRef getMessage(int target, int source, int error, bool& slice, bool& stop);

  bdd BDD;  

protected:
  int line_number;
  string file_name;
  bool is_trusted;
  string function_name;
  
  MsgRef receivesErrorMsg(int target, int source, int error, bool& slice, bool& stop);
  MsgRef mayHaveErrorMsg(int target, int error);

  };

#endif	// ERROR_PROPAGATION_GUARD

/*
 * $Log $
 */

/* Yo, Emacs!
;;; Local Variables: ***
;;; tab-width: 4 ***
;;; End: ***
*/

