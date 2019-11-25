#ifndef MERGE_ERROR_GUARD
#define MERGE_ERROR_GUARD 1

/*!
 * @author Cindy Rubio Gonzalez
 */

#include <string>
#include "fdd.h"
#include <wali/MergeFn.hpp>
#include <wali/MergeFnFactory.hpp>

using namespace std;

using wali::SemElem;
using wali::sem_elem_t;
using wali::merge_fn_t; 


class MergeError : public wali::MergeFnFactory, public wali::MergeFn {
public:
  MergeError(string f);
  MergeError(string f, sem_elem_t w);
  
  // For MergeFactory
  merge_fn_t getMergeFn(std::string s);
  
  // For MergeFn
  SemElem* apply_f(SemElem* w1, SemElem* w2);
  sem_elem_t apply_f(sem_elem_t w1, sem_elem_t w2);
  
private:
  string function;
  sem_elem_t weight;
  
};

#endif	// MERGE_ERROR_GUARD

/*
 * $Log $
 */

/* Yo, Emacs!
;;; Local Variables: ***
;;; tab-width: 4 ***
;;; End: ***
*/

