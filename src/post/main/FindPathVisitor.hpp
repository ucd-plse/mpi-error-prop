#ifndef FIND_PATH_VISITOR_GUARD
#define FIND_PATH_VISITOR_GUARD 1

/*!
 * @author Nick Kidd
 * modified by Cindy Rubio Gonzalez
 */

#include "wali/witness/Visitor.hpp"
#include "ErrorPropagation.hpp"

class Path;


class FindPathVisitor : public wali::witness::Visitor {
  
public:
  
  FindPathVisitor(int v, int s, int e, Path& p, bool& f);
  
  ~FindPathVisitor();
  
  /*!
   * @return true to continue visiting children, false to stop.
   */
  virtual bool visit( wali::witness::Witness * w );
  
  /*!
   * @return true to continue visiting children, false to stop.
   */
  virtual bool visitExtend( wali::witness::WitnessExtend * w );

  /*!
   * @return true to continue visiting children, false to stop.
   */
  virtual bool visitCombine( wali::witness::WitnessCombine * w );
  
  /*!
   * @return true to continue visiting children, false to stop.
   */
  virtual bool visitRule( wali::witness::WitnessRule * w );
  
  /*!
   * @return true to continue visiting children, false to stop.
   */
  virtual bool visitTrans( wali::witness::WitnessTrans * w);
  
  virtual bool visitMerge( wali::witness::WitnessMerge * w );
  
  ErrorPropagation* witnessToWeight( wali::witness::Witness* w );
  
  int target;
  int source;
  int error;
  Path& path;
  bool stop;

}; // class FindPathVisitor 


#endif  // FIND_PATH_VISITOR_GUARD

/* Yo, Emacs!
   ;;; Local Variables: ***
   ;;; tab-width: 4 ***
   ;;; End: ***
 */
