#ifndef PATH_GUARD
#define PATH_GUARD 1

/*!
 * @author Cindy Rubio Gonzalez
 */

#include <list>
#include <string>
#include <fstream>
#include "Message.hpp"

class ProgramPoint;

using namespace std;


class Path {
  
public:
  
  Path(MsgRef msg);
  ~Path();
  void add(MsgRef msg, bool slice);
  void printPath(std::ostream& out);
  void printSlice(std::ostream& out);
  void printReport(std::ostream& out, bool slice=true);
  int size();
  bool isComplete();
  void setComplete(bool c);
  
private:
  struct MessageList : public std::list<MsgRef> {
	void appendIfChanged(MsgRef message);
  };
  void filter(MessageList &list);;

  MessageList pathMsgs;
  MessageList sliceMsgs;
  bool complete;


}; // class Path


#endif  // PATH_GUARD

/* Yo, Emacs!
   ;;; Local Variables: ***
   ;;; tab-width: 4 ***
   ;;; End: ***
 */
