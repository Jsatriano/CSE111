// $Id: ubigint.cpp,v 1.4 2022-03-21 16:02:26-07 - - $

#include <cctype>
#include <cstdlib>
#include <exception>
#include <stack>
#include <stdexcept>
#include <vector>
using namespace std;

#include "debug.h"
#include "ubigint.h"

ostream& operator<< (ostream& out, const vector<uint8_t>& vec) {
   for (auto itor = vec.rbegin(); itor != vec.rend(); ++itor) {
      out << int (*itor);
   }
   return out;
}

ostream& operator<< (ostream& out, const ubigint& that) { 
   return out << "ubigint(" << that.uvalue << ")";
}

ubigint::ubigint (unsigned long that) {
   while(that > 0) {
      uvalue.push_back(that % 10);
      that = that / 10;
   }
   DEBUGF ('~', this << " -> " << uvalue);
}

ubigint::ubigint (const string& that): uvalue(0) {
   DEBUGF ('~', "that = \"" << that << "\"");
   for (char digit: that) {
      if (not isdigit (digit)) {
         throw invalid_argument ("ubigint::ubigint(" + that + ")");
      }
      uvalue.insert(0, digit - '0');
      //uvalue.push_back(digit - '0');
      //uvalue = uvalue * 10 + digit - '0';
   }
   //reverse(uvalue.begin(), uvalue.end());
}

ubigint ubigint::operator+ (const ubigint& that) const {
   DEBUGF ('u', *this << "+" << that);
   ubigint result;
   int x = 0;
   uint8_t value = 0;
   uint8_t carry = 0;
   int y = 0;
   vector<uint8_t> c;
   
   // Checks edge case if either is 0
   if (this->uvalue.size() == 0 or that.uvalue.size() == 0) {
      this->uvalue.size() != 0 ? y = this->uvalue.size() : y = that.uvalue.size();
      this->uvalue.size() != 0 ? c = this->uvalue : c = that.uvalue;
      for (int k = 0; k < y; k++) {
         result.uvalue.push_back(c.at(k));
      }
      return result;
   }

   this->uvalue.size() < that.uvalue.size() ? x = this->uvalue.size() : x = that.uvalue.size();
   for (int i = 0; i < x; i++) {
      value = this->uvalue.at(i) + that.uvalue.at(i) + carry;
      if (value > 9) {
         value = value % 10;
         carry = 1;
      }
      else {
         carry = 0;
      }
      result.uvalue.push_back(value);
   }
   x = (this->uvalue.size() - that.uvalue.size());
   if (x < 0) {
      for (int j = this->uvalue.size(); j < that.uvalue.size(); j++) {
         value = that.ubigint.uvalue.at(j) + carry;
         carry = 0;
         result.uvalue.push_back(value);
      }
   }
   else if (x > 0) {
      for (int j = that->uvalue.size(); j < this->uvalue.size(); j++) {
         value = this->uvalue.at(j) + carry;
         carry = 0;
         result.uvalue.push_back(value);
      }
   }
   DEBUGF ('u', result);

   int d = result.uvalue.size() - 1;
   while (result.uvalue.at(d) == 0){
      result.uvalue.pop_back();
      d--;
   }
   return result;
}

ubigint ubigint::operator- (const ubigint& that) const {
   if (*this < that) throw domain_error ("ubigint::operator-(a<b)");
   ubigint result;
   uint8_t value = 0;
   uint8_t carry = 0;
   vector<uint8_t> c;
   int y;
   int x = that.uvalue.size();

   if (this->uvalue.size() == 0 or that.uvalue.size() == 0) {
      this->uvalue.size() != 0 ? y = this->uvalue.size() : y = that.uvalue.size();
      this->uvalue.size() != 0 ? c = this->uvalue : c = that.uvalue;
      for (int k = 0; k < y; k++) {
         result.uvalue.push_back(c.at(k));
      }
      return result;
   }

   for (int i = 0; i < x; i++) {
      value = this->uvalue.at(i) - that.uvalue.at(i) - carry;
      if (value < 0) {
         value = value + 10;
         carry = 1;
      }
      else {
          carry = 0;
      }
      result.uvalue.push_back(value);
   }
   if (this->uvalue.size() > x) {
      for (int j = x; j < this->uvalue.size(); j++) {
         value = this->uvalue.at(j) - carry;
         carry = 0;
         result.uvalue.push_back(value);
      }
   }
   
   while (result.uvalue.at(d) == 0){
      result.uvalue.pop_back();
      d--;
   }
   return result;
}

ubigint ubigint::operator* (const ubigint& that) const {
   return ubigint (uvalue * that.uvalue);
}

void ubigint::multiply_by_2() {
   uvalue *= 2;
}

void ubigint::divide_by_2() {
   uvalue /= 2;
}


struct quo_rem { ubigint quotient; ubigint remainder; };
quo_rem udivide (const ubigint& dividend, const ubigint& divisor_) {
   // NOTE: udivide is a non-member function.
   ubigint divisor {divisor_};
   ubigint zero {0};
   if (divisor == zero) throw domain_error ("udivide by zero");
   ubigint power_of_2 {1};
   ubigint quotient {0};
   ubigint remainder {dividend}; // left operand, dividend
   while (divisor < remainder) {
      divisor.multiply_by_2();
      power_of_2.multiply_by_2();
   }
   while (power_of_2 > zero) {
      if (divisor <= remainder) {
         remainder = remainder - divisor;
         quotient = quotient + power_of_2;
      }
      divisor.divide_by_2();
      power_of_2.divide_by_2();
   }
   DEBUGF ('/', "quotient = " << quotient);
   DEBUGF ('/', "remainder = " << remainder);
   return {.quotient = quotient, .remainder = remainder};
}

ubigint ubigint::operator/ (const ubigint& that) const {
   return udivide (*this, that).quotient;
}

ubigint ubigint::operator% (const ubigint& that) const {
   return udivide (*this, that).remainder;
}

bool ubigint::operator== (const ubigint& that) const {
   if (this->uvalue.size() != that.uvaluesize()) {
      return false;
   }
   else {
      for (int i = 0; i < this->uvalue.size(); i++) {
         if (this->uvalue.at(i) != that.uvalue.at(i)) {
            return false;
         }
      }
   }
   return true;
}

bool ubigint::operator< (const ubigint& that) const {
   if ((int a = this->uvalue.size()) != (int b = that.uvalue.size())) {
      if (a < b) {
         return true;
      }
      else {
         return false;
      }
   }
   else {
      for (int i = this->uvalue.size(); i > 0; i--) {
         if (this->uvalue.at(i) > that.uvalue.at(i)) {
            return false;
         }
         else if (this->uvalue.at(i) < that.uvalue.at(i)){
            return true;
         }
      }
      return false;
   }
}

void ubigint::print() const {
   DEBUGF ('p', this << " -> " << *this);
   cout << uvalue;
}

