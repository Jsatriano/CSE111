/*************************************************
# Justin Satriano    (jsatrian)
# Sriharsha Madala   (srmadala)
#*************************************************/

// $Id: bigint.cpp,v 1.2 2021-12-28 14:11:26-08 - - $

#include <cstdlib>
#include <exception>
#include <stack>
#include <stdexcept>
#include <vector>
using namespace std;

#include "bigint.h"
#include "debug.h"

bigint::bigint (long that): uvalue (that), is_negative (that < 0) {
   DEBUGF ('~', this << " -> " << uvalue)
}

bigint::bigint (const ubigint& uvalue_, bool is_negative_):
                uvalue(uvalue_), is_negative(is_negative_) {
}

bigint::bigint (const string& that) {
   is_negative = that.size() > 0 and that[0] == '_';
   uvalue = ubigint (that.substr (is_negative ? 1 : 0));
}

bigint bigint::operator+ () const {
   return *this;
}

bigint bigint::operator- () const {
   if(uvalue == 0) {
      return {uvalue, false};
   }
   else {
      return {uvalue, not is_negative};
   }
}


bigint bigint::operator+ (const bigint& that) const {
   ubigint result;
   bool sign;

   if(this->is_negative && !that.is_negative) {
      if(this->uvalue > that.uvalue) {
         result = this->uvalue - that.uvalue;
         sign = true;
      }
      else {
         result = that.uvalue - this->uvalue;
         sign = false;
      }
   }
   else if(!this->is_negative && that.is_negative) {
      if(that.uvalue > this->uvalue) {
         result = that.uvalue - this->uvalue;
         sign = true;
      }
      else {
         result = this->uvalue - that.uvalue;
         sign = false;
      }
   }
   else {
      result = this->uvalue + that.uvalue;
      sign = this->is_negative;
   }

   if(result == 0) {
      sign = false;
   }

   return bigint(result, sign);
}

bigint bigint::operator- (const bigint& that) const {
   ubigint result;
   bool sign;

   if(this->is_negative && !that.is_negative) {
      result = this->uvalue + that.uvalue;
      sign = true;
   }
   else if(!this->is_negative && that.is_negative) {
      result = this->uvalue + that.uvalue;
      sign = false;
   }
   else if(!this->is_negative && !that.is_negative) {
      if(this->uvalue > that.uvalue) {
         result = this->uvalue - that.uvalue;
         sign = false;
      }
      else {
         result = that.uvalue - this->uvalue;
         sign = true;
      }
   }
   else {
      if(this->uvalue > that.uvalue) {
         result = this->uvalue - that.uvalue;
         sign = true;
      }
      else {
         result = that.uvalue - this->uvalue;
         sign = false;
      }
   }

   if(result == 0) {
      sign = false;
   }

   return bigint(result, sign);
}

bigint bigint::operator* (const bigint& that) const {
   bigint result {uvalue * that.uvalue};
   if(this->is_negative and that.is_negative) {
      result.is_negative = false;
   }
   else if(this->is_negative or that.is_negative) {
      result.is_negative = true;
   }
   return result;
}

bigint bigint::operator/ (const bigint& that) const {
   bigint result {uvalue / that.uvalue};
   if(this->is_negative and that.is_negative) {
      result.is_negative = false;
   }
   else if(this->is_negative or that.is_negative) {
      result.is_negative = true;
   }
   return result;
}

bigint bigint::operator% (const bigint& that) const {
   bigint result {uvalue % that.uvalue};
   result.is_negative = this->is_negative;
   return result;
}

bool bigint::operator== (const bigint& that) const {
   return is_negative == that.is_negative and uvalue == that.uvalue;
}

bool bigint::operator< (const bigint& that) const {
   if (is_negative != that.is_negative) return is_negative;
   return is_negative ? uvalue > that.uvalue
                      : uvalue < that.uvalue;
}

void bigint::print() const {
   DEBUGF ('p', this << " -> " << *this);
   bool neg = false;
   if (is_negative) {
      cout << "-";
      neg = true;
   }

   uvalue.print(neg);
}

ostream& operator<< (ostream& out, const bigint& that) {
   return out << "bigint(" << (that.is_negative ? "-" : "+")
              << "," << that.uvalue << ")";
}

