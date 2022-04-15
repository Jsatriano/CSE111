/*************************************************
# Justin Satriano    (jsatrian)
# Sriharsha Madala   (srmadala)
#*************************************************/

// $Id: scanner.h,v 1.1 2021-12-28 13:54:01-08 - - $

#ifndef SCANNER_H
#define SCANNER_H

#include <iostream>
#include <utility>
using namespace std;

#include "debug.h"

enum class tsymbol {SCANEOF, NUMBER, OPERATOR};

struct token {
   tsymbol symbol;
   string lexinfo;
   token (tsymbol sym, const string& lex = string()):
          symbol(sym), lexinfo(lex){
   }
};

class scanner {
   private:
      istream& instream;
      int nextchar {instream.get()};
      bool good() const { return nextchar != EOF; }
      char get();
   public:
      scanner (istream& instream_ = cin): instream(instream_) {}
      token scan();
};

ostream& operator<< (ostream&, tsymbol);
ostream& operator<< (ostream&, const token&);

#endif

