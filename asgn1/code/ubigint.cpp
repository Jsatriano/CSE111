/*************************************************
# Justin Satriano    (jsatrian)
# Sriharsha Madala   (srmadala)
#*************************************************/

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

bool sign = false;

ostream& operator<< (ostream& out, const vector<uint8_t>& vec) {
   int counter = 0;
   if(sign == true) {
      counter += 1;
   }
   for (auto itor = vec.rbegin(); itor != vec.rend(); ++itor) {
         out << int (*itor);
         counter += 1;
         if(counter == 69) {
            cout << "\\" << endl;
            counter = 0;
         }
   }
   return out;
}

ostream& operator<< (ostream& out, const ubigint& that) { 
   string str;
   for(unsigned int i = 0; i < that.uvalue.size(); i += 1) {
      cout << sign << endl;
      if(sign == true && i == 70) {
         str.append("\\\n");
      }
      else if(i > 0 && i % 71 == 0) {
         str.append("\\\n");
      }
      str.append(1, static_cast<char>(that.uvalue.at(i) + '0'));
   }
   return out << str;
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
      uvalue.insert(uvalue.begin(), digit - '0'); // check to make sure
      if (this->uvalue.size() == 1) {
         if (this->uvalue.at(0) == 0) {
            this->uvalue.clear();
         }
      }
   }
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
      y = (this->uvalue.size() != 0 ? this->uvalue.size() : that.uvalue.size());
      c = this->uvalue.size() != 0 ? this->uvalue : that.uvalue;
      for (int k = 0; k < y; k++) {
         result.uvalue.push_back(c.at(k));
      }
      return result;
   }

   x = this->uvalue.size() < that.uvalue.size() ? this->uvalue.size() : that.uvalue.size();
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
      for (unsigned int j = this->uvalue.size(); j < that.uvalue.size(); j++) {
         value = that.uvalue.at(j) + carry;
         if (value > 9) {
            value = value % 10;
            carry = 1;
         }
         else {
            carry = 0;
         }
         result.uvalue.push_back(value);
      }
   }
   else if (x > 0) {
      for (unsigned int j = that.uvalue.size(); j < this->uvalue.size(); j++) {
         value = this->uvalue.at(j) + carry;
         if (value > 9) {
            value = value % 10;
            carry = 1;
         }
         else {
            carry = 0;
         }
         result.uvalue.push_back(value);
      }
   }
   DEBUGF ('u', result);
   if(carry > 0) {
      result.uvalue.push_back(1);
   }

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
   int value = 0;
   uint8_t carry = 0;
   vector<uint8_t> c;
   int y;
   unsigned int x = that.uvalue.size();

   if (this->uvalue.size() == 0 or that.uvalue.size() == 0) {
      y = this->uvalue.size() != 0 ? this->uvalue.size() : that.uvalue.size();
      c = this->uvalue.size() != 0 ? this->uvalue : that.uvalue;
      for (int k = 0; k < y; k++) {
         result.uvalue.push_back(c.at(k));
      }
      return result;
   }

   for (unsigned int i = 0; i < x; i++) {
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
      for (unsigned int j = x; j < this->uvalue.size(); j++) {
         value = this->uvalue.at(j) - carry;
         if(value < 0) {
            value = value + 10;
            carry = 1;
         }
         else {
            carry = 0;
         }
         result.uvalue.push_back(value);
      }
   }

   int d = result.uvalue.size() - 1;
   while (result.uvalue.at(d) == 0){
      result.uvalue.pop_back();
      d--;
      if (d < 0) {
         break;
      }
   }
   return result;
}

ubigint ubigint::operator* (const ubigint& that) const {
   ubigint result;
   uint8_t value = 0;
   uint8_t carry = 0;
   int s = (that.uvalue.size() * this->uvalue.size());
   unsigned int u = 0;
   int v = 0;

   if (this->uvalue.size() == 0 or that.uvalue.size() == 0) {
      return result;
   }

   this->uvalue.size() > that.uvalue.size() ? u = that.uvalue.size() : u = this->uvalue.size();
   u == that.uvalue.size() ? v = this->uvalue.size() : v = that.uvalue.size();
   for (int k = 0; k < s; k++) {
      result.uvalue.push_back(0);
   }
   unsigned int i = 0;
   int j = 0;
   for (i = 0; i < u; i++) {
      for (j = 0; j < v; j++) {
         if(this->uvalue.size() < that.uvalue.size()) {
            value = result.uvalue.at(i+j) + (this->uvalue.at(i) * that.uvalue.at(j)) + carry;
         }
         else {
            value = result.uvalue.at(i+j) + (that.uvalue.at(i) * this->uvalue.at(j)) + carry;
         }
         carry = value / 10;
         result.uvalue.erase(result.uvalue.begin() + j + i);
         result.uvalue.insert(result.uvalue.begin() + j + i, (value%10));
      }
      if (carry != 0) {
         result.uvalue.insert(result.uvalue.begin() + j + i, carry);
         carry = 0;
      }
   }
   int d = result.uvalue.size() - 1;
   while (result.uvalue.at(d) == 0){
      result.uvalue.pop_back();
      d--;
   }
   return result;
}

void ubigint::multiply_by_2() {
   uint8_t carry = 0;
   uint8_t value = 0;
   vector<uint8_t> c;
   for (unsigned int i = 0; i < this->uvalue.size(); i++) {
      value = (this->uvalue.at(i)*2) + carry;
      carry = (value/10);
      c.push_back(value%10);
   }
   this->uvalue.swap(c);
   if (carry != 0) {
      this->uvalue.push_back(carry);
      carry = 0;
   }
}

void ubigint::divide_by_2() {
   uint8_t value = 0;
   vector<uint8_t> c;
   for (unsigned int i = 0; i < this->uvalue.size(); i++) {
      value = (this->uvalue.at(i)/2);
      if (i != this->uvalue.size() - 1) {
         if ((this->uvalue.at(i+1)%2) != 0) {
            value = value + 5;
         }
      }
      c.push_back(value);
   }

   this->uvalue.swap(c);
   int d = this->uvalue.size()-1;
   while (this->uvalue.at(d) == 0){
      this->uvalue.pop_back();
      d--;
      if (this->uvalue.size() == 0) {
         break;
      }
   }
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
   if (this->uvalue.size() != that.uvalue.size()) {
      return false;
   }
   else {
      for (unsigned int i = 0; i < this->uvalue.size(); i++) {
         if (this->uvalue.at(i) != that.uvalue.at(i)) {
            return false;
         }
      }
   }
   return true;
}

bool ubigint::operator< (const ubigint& that) const {
   int a = this->uvalue.size();
   int b = that.uvalue.size();
   if (a != b) {
      if (a < b) {
         return true;
      }
      else {
         return false;
      }
   }
   else {
      for (int i = this->uvalue.size() - 1; i >= 0; i--) {
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

void ubigint::print(bool neg) const {
   DEBUGF ('p', this << " -> " << *this);
   if(neg == true) {
      sign = true;
   }
   cout << uvalue;
}

