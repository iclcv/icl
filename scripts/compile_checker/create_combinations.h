#ifndef CREATE_CHECK_LIST_H
#define CREATE_CHECK_LIST_H

#include "types.h"
#include <string>
 
string get_make_def(const string &s);

string get_overwrite_def(const string &s);

string create_arg(const string &a, const string &def);

void create_args(string *begin, string *end);

std::vector<arglist> create_combinations();


#endif
