/*************************************************
# Justin Satriano    (jsatrian)
# Sriharsha Madala   (srmadala)
#*************************************************/

// $Id: bigint.h,v 1.3 2022-01-07 17:55:54-08 - - $

#ifndef BIGINT_H
#define BIGINT_H

#include <exception>
#include <iostream>
#include <limits>
#include <utility>
using namespace std;

#include "debug.h"
#include "relops.h"
#include "ubigint.h"

class bigint {
   friend ostream& operator<< (ostream&, const bigint&);
   private:
      ubigint uvalue {};
      bool is_negative {false};
   public:

      bigint() = default; // Needed or will be suppressed.
      bigint (long);
      bigint (const ubigint&, bool is_negative = false);
      explicit bigint (const string&);

      bigint operator+() const;
      bigint operator-() const;

      bigint operator+ (const bigint&) const;
      bigint operator- (const bigint&) const;
      bigint operator* (const bigint&) const;
      bigint operator/ (const bigint&) const;
      bigint operator% (const bigint&) const;

      bool operator== (const bigint&) const;
      bool operator<  (const bigint&) const;

      void print() const;
};

#endif

