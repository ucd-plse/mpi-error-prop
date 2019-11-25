/*!
 * @author Cindy Rubio Gonzalez
 */

#include <sstream>
#include <algorithm>
#include "Main.hpp"
#include "Path.hpp"
#include "ProgramPoint.hpp"


// Following the convention of the flags,
// selected_formatter is global from Main.hpp

Path::Path(MsgRef msg)
  : complete(false)
{
  pathMsgs.push_back(msg);
  sliceMsgs.push_back(msg);
}


Path::~Path() {}


void Path::add(MsgRef msg, bool slice) {
  pathMsgs.appendIfChanged(msg);
  if (slice) {
    sliceMsgs.appendIfChanged(msg);
  }
  return;
}

void Path::printPath(std::ostream& out) {
  if (print == Forward) {
    list<MsgRef>::reverse_iterator it = pathMsgs.rbegin();
    for(; it != pathMsgs.rend(); it++) {
      string msg = (*it)->format();   
      if (!msg.empty()) {
        out << msg;
      }
    }
  }
  else {
	list<MsgRef>::iterator it = pathMsgs.begin();
	for(; it != pathMsgs.end(); it++) {
          string msg = (*it)->format();   
          if (!msg.empty()) {
            out << msg;
          }
          if (selected_formatter != Message::Formatters::TRACES) {
            out << endl;
          }
	}
  }
  return;
}


void Path::printSlice(std::ostream& out) {
  if (print == Forward) {
	list<MsgRef>::reverse_iterator it = sliceMsgs.rbegin();
    for(; it != sliceMsgs.rend(); it++) {
      string msg = (*it)->format();   
      if (!msg.empty()) {
        out << msg << endl;
      }
    }
  }
  else {
	list<MsgRef>::iterator it = sliceMsgs.begin();
	for(; it != sliceMsgs.end(); it++) {
	  out << (*it)->format();
	  
	  if (*it != sliceMsgs.back())
		out << endl;
	}
  }
  return;
}


void Path::printReport(std::ostream& out, bool slice) {
  Message::Types frontTy = pathMsgs.front()->type;
  if (selected_formatter == Message::Formatters::STANDARD &&
      frontTy != Message::Types::RETURN_FN &&
      frontTy != Message::Types::DEREFERENCE &&
      frontTy != Message::Types::HANDLED &&
      frontTy != Message::Types::ISERR &&
      frontTy != Message::Types::ISERRWARN &&
      frontTy != Message::Types::OPERAND) {
    out << "Error codes: ";
    out << pathMsgs.front()->getErrorCodeStr(true);
    out << endl << endl;
  }

  filter(pathMsgs);
  printPath(out);

  if (slice && selected_formatter == Message::Formatters::STANDARD) {
    out << endl;
    filter(sliceMsgs);
    printSlice(out);
    out << endl;
  }

  if (selected_formatter == Message::Formatters::STANDARD) {
    out << "====" << endl;
  }
  return;
}


int Path::size() {
  return pathMsgs.size();
}


bool Path::isComplete() {
  return complete;
}


void Path::setComplete(bool c) {
  complete = c;
  return;
}


void Path::MessageList::appendIfChanged(MsgRef message) {
  if (empty() || (*back() != *message)) {
    push_back(message);
  }
}

// Predicate for list erase in Path::filter
bool removeMsg(MsgRef msg ) {
  if (msg->target.find("__cil") != string::npos) {
    return true;
  }
  return false;
}

void Path::filter(MessageList &list) {
  // Update line number of message to match __cil var message
  // Not entirely sure why the line number fix needs to be made,
  // but it should match previous filter.
  // If there is a bug, could be here :)
  // --- (Daniel)
  int line_update = -1;
  for (MessageList::reverse_iterator it = list.rbegin(); it != list.rend(); ++it) {
    if ((*it)->target.find("__cil") != string::npos && line_update == -1) {
      // Set update line number
      line_update = (*it)->line_number;
    } else if ((*it)->target.find("__cil") == string::npos && line_update != -1) {
      (*it)->line_number = line_update;
      line_update = -1;
    }
  }

  // Two loops, scalvage what clarity we can in lieu of small efficiency gains.

  // Remove __cil messages
  // Determines membership by removeMsg()
  list.erase(remove_if(++(list.begin()), list.end(), removeMsg), list.end()); 
}


// void Path::MessageList::filter() {
// 
//   // Special handling for unsaved errors
//   if (front().find("saved", 0) != string::npos) {
// 
// 	// saving first message
// 	MessageList::iterator it = begin();
// 	string::size_type p1 = it->find(':', 0);
// 	string::size_type p2 = it->find(':', p1+1);
// 	string file = it->substr(0, p1);
// 	string line = it->substr(p1+1, p2-p1-1);
// 	string msg = it->substr(p2+1, it->size()-p2-1);
// 	pop_front();
// 
// 	MessageList temp;
// 
// 	// Deciding which messages to keep
// 	for(it = begin(); it != end(); it++) {
// 	  if (it->find("__cil") == string::npos) {
// 		temp.push_back(*it);
// 	  }
// 	  else {
// 		p1 = it->find(':', 0);
// 		p2 = it->find(':', p1+1);
// 		line = (it->substr(p1+1, p2-p1-1)).c_str();
// 	  }
// 	}
// 	
// 	clear();   
// 
// 	// Copying back to pathMsgs
// 	
// 	// first new message
// 	string newMessage = file + ':' + line + ':' + msg;
// 	push_back(newMessage);
// 
// 	for(it = temp.begin(); it != temp.end(); it++) {
// 	  push_back(*it);
// 	}
// 
//   } // end if	
//   return;
// }



/* Yo, Emacs!
   ;;; Local Variables: ***
   ;;; tab-width: 4 ***
   ;;; End: ***
 */
