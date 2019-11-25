#ifndef MAIN_GUARD
#define MAIN_GUARD

#include "Message.hpp"

extern bool verbose;
extern bool debugging;
extern bool return_values;
extern bool info;
extern bool hexastore;
extern bool csv;
extern bool dereference;
extern bool iserr;
extern bool handled;
extern bool input_output;
extern bool predicates;
extern bool option_assignment;

extern Message::Formatters selected_formatter;

enum PrintDirection { None, Backward, Forward };
extern PrintDirection print;

#endif // !MAIN_GUARD
