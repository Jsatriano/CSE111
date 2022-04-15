/*************************************************
# Justin Satriano    (jsatrian)
# Sriharsha Madala   (srmadala)
#*************************************************/

// $Id: util.h,v 1.1 2021-12-28 13:54:01-08 - - $

//
// util -
//    A utility class to provide various services
//    not conveniently included in other modules.
//

#ifndef UTIL_H
#define UTIL_H

#include <iomanip>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <vector>
using namespace std;

#include "debug.h"

//
// ydc_error -
//    Indicate a problem where processing should be abandoned and
//    the main function should take control.
//

class ydc_error: public runtime_error {
   public:
      explicit ydc_error (const string& what): runtime_error (what) {
      }
};

//
// octal -
//    Convert integer to octal string.
//

const string octal (long number);


//
// main -
//    Keep track of execname and exit status.  Must be initialized
//    as the first thing done inside main.  Main should call:
//       main::execname (argv[0]);
//    before anything else.
//

class exec {
   private:
      static string execname_;
      static int status_;
      static void execname (const string& argv0);
      friend int main (int, char**);
   public:
      static void status (int status);
      static const string& execname() {return execname_; }
      static int status() {return status_; }
};

//
// complain -
//    Used for starting error messages.  Sets the exit status to
//    EXIT_FAILURE, writes the program name to cerr, and then
//    returns the cerr ostream.  Example:
//       complain() << filename << ": some problem" << endl;
//

ostream& note();
ostream& error();

#endif

