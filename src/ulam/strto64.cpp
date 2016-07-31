#include "strto64.h"
#include <assert.h> /* for assert */
#include <ctype.h> /* for tolower, isspace */
#include "Util.h"

// Old school home cooked conversion routine

namespace MFM {

  struct Strto64 {

    // Main entry point (see strto64 at bottom)
    u64 parse()
    {
      // Quit as soon as problem encountered
      parseSign() && parseBase() && parseDigits() && checkRange();

      return result; // perhaps with error msg flag set
    }

    const char *& ptr;
    const char *& errPtr;
    s32 validBits;
    u32 base;
    u64 result;
    s32 sign;
    bool sawDelimiter;

    Strto64(const char *& p, const char *& e, s32 vb) 
      : ptr(p)
      , errPtr(e)
      , validBits(vb)
      , base(0)
      , result(0)
      , sign(0)  // ==0 no sign or unknown, <0 saw '-', >0 saw '+',
    {
      assert(ptr);
      assert(base == 0 || (base >= 2 && base <= 35));
      assert(validBits >= -64 && validBits <= 64);

      errPtr = 0; // hope for the best
    }

    bool die(const char * msg)
    {
      errPtr = msg;
      return false;
    }

    s32 read() 
    {
      if (*ptr == 0) return -1;
      return (u32) tolower(*ptr++); // don't sign-extend
    }

    void unread() { --ptr; }

    s32 peek() 
    {
      s32 ch = read();
      if (ch >= 0) unread();
      return ch;
    }

    s32 digitVal(s32 ch)
    {
      // EOF is not a digit
      if (ch < 0) return -1;

      s32 explicitBase = base;

      // If we don't know the base yet, decimal is safe
      if (explicitBase == 0)
        explicitBase = 10;

      if (ch < '0') return -1;

      if (explicitBase <= 10 || ch <= '9') {
        if (ch >= '0' + explicitBase) return -1;
        return ch - '0';
      }

      if (ch < 'a' || ch >= 'a' + explicitBase - 10) return -1;

      return ch - 'a' + 10;
    }

    ////
    // PARSING ROUTINES

    bool parseSign()       // leading whitespace and/or sign
    {
      s32 ch;

      while (true) {
        ch = read();
        if (ch < 0) return die("No number found");

        if (isspace(ch)) continue;

        if (ch == '+') {
          if (sign < 0) return die("Both - and + signs found");
          if (sign > 0) return die("Duplicate + signs found");
          sign = 1;
          continue;
        }

        if (ch == '-') {
          if (sign < 0) return die("Duplicate - signs found");
          if (sign > 0) return die("Both + and - signs found");
          sign = -1;
          continue;
        }

        // not ws, signs
        unread();
        break;
      }
      return true;
    }

    bool parseBase()       // leading 0, 0b, 0x, or 1-9
    {
      s32 val = digitVal(peek()); 

      if (val < 0) {
        if (sign < 0) return die("No digits found after -");
        if (sign > 0) return die("No digits found after +");
        return die("No digits found");
      }

      if (val >= 1 && val <= 9) {
        base = 10;
        return true;
      }

      if (val != 0) return die("No radix prefix (0, 0x, 0b, or 1-9) found");

      read(); // discard the '0'

      s32 ch = read();

      if (ch < 0 || ch == 'u') {

        // Whups, the whole number was 0 or 0u.  We check for EOF
        // after the possible 'u', then return false here to
        // shortcircuit downstream, even though the conversion is a
        // success (and the default result is correct).

        if (read() >= 0) return die("Characters after 'u' in number");
        return false;
      }

      if (ch == 'x') {
        base = 16;
        return true;
      }

      if (ch == 'b') {
        base = 2;
        return true;
      }

      unread(); // maybe we'll get hungry later

      // Only possibility left is awful octal.
      if (ch >= '0' && ch <= '7') {
        base = 8;
        return true;
      }

      // Not 0, 0u, 0x, 0b, or 1-9: FU
      return die("Illegal character in number");
    }

    bool parseDigits()     // base now known
    {
      s32 ch;
      while ((ch = read()) >= 0) {
        if (ch == 'u') {
          // 'u' is okay so long as we're at the end
          if (read() >= 0) return die("Characters after 'u' in number");
          return true;
        }

        s32 val = digitVal(ch);
        if (val < 0) {
          switch (base) {
          case 2: return die("Illegal character in binary number");
          case 8: return die("Illegal character in octal number");
          case 10: return die("Illegal character in decimal number");
          case 16: return die("Illegal character in hexadecimal number");
          default: assert(0);
          }
        }
        u64 nextResult = result * base + val;
        if (nextResult / base != result) // ignoring val since < base
          return die("Number too large");
        result = nextResult;
      }
      return true;
    }

    bool checkRange()
    {
      if (validBits == -64) {
        if (sign >= 0) {
          if (result > (u64) S64_MAX) 
            return die("Positive number overflows Int(64)");
          return true;
        } 
        // sign < 0
        if (result > (((u64) S64_MAX) + 1))
          return die("Negative number overflows Int(64)");
        if (result == (((u64) S64_MAX) + 1))
          result = S64_MIN;
        else
          result = -(s64) result;
        return true;
      }

      if (validBits == 64) {
        if (sign >= 0) return true;
        // sign < 0
        if (((s64) result) > 0) 
          return die("Negative number underflows Unsigned(64)");
        result = -(s64) result; // ??
        return true;
      }

      if (validBits < 0) {
        s64 max = (s64) _GetNOnes63(-validBits + 1);
        s64 min = -max - 1;
        if (sign < 0) result *= -1;
        if ((s64) result < min) return die("Negative number below signed minof for size");
        if ((s64) result > max) return die("Positive number above signed maxof for size");
        return true;
      }

      /* 0 <= validBits < 64 */
      {
        u64 max = _GetNOnes63(validBits);
        u64 min = 0;
        assert(0);  // XXX not sure what to do here :(
        if (sign < 0) result *= -1;
        if (result < min) return die("Negative number below minof for size");
        if (result > max) return die("Positive number above maxof for size");
        return true;

      }
    }

  };

  u64 strto64(const char *& ptr, const char *& errPtr, s32 validBits) 
  {
    Strto64 a(ptr, errPtr, validBits);
    return a.parse();
  }

}




