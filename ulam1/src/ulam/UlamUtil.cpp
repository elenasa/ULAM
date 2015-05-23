#include "UlamUtil.h"
#include "Util.h"
#include <sstream>  /* For ostringstream */
#include <cctype>   /* For isgraph */
#include <time.h>   /* For nanosleep */
#include <assert.h>

namespace MFM
{
  //note: DigitCount is defined in MFM Util.cpp

  std::string ToLeximitedNumber(s32 num)
  {
    //handles negative numbers
    //returns 'n' before a negative num, e.g., -3 -> n13
    //returns '10' when 'num' is zero
    //returns 'n10' when 'num' is the biggest negative number
    bool useneg = false;
    u32 digits;
    if(num == 0)
      digits = 1;
    else if(num < 0)
      {
	// use 'n10' as a special code meaning 'max negative number'
        if (num == S32_MIN)
          {
            num = 0;
            digits = 1;
          }
        else
          {
            num = -num;
            digits = DigitCount((u32) num, 10);
          }
	useneg = true;
      }
    else //>0
      {
	digits = DigitCount((u32) num, 10);
      }

    std::ostringstream os;

    if(useneg)
      os << "n";

    if (digits < 9)
      os << digits;
    else
      os << 9 << ToLeximited(digits);

    os << num;
    return os.str();
  } //ToLeximitedNumber (signed)


  std::string ToLeximitedNumber(u32 num)
  {
    //returns 10 when 'num' is zero
    u32 digits = (num == 0 ? 1 : DigitCount(num, 10));

    std::ostringstream os;
    if (digits < 9)
      os << digits << num;
    else
      os << 9 << ToLeximited(digits) << num;

    return os.str();
  } //ToLeximitedNumber (unsigned)


  std::string ToLeximited(u32 len)
  {
    u32 digits = DigitCount(len,10);

    std::ostringstream os;
    if (len < 9)
      os << len;
    else
      os << 9 << ToLeximited(digits) << len;
    return os.str();
  }

  std::string ToLeximited(const std::string & source)
  {
    std::ostringstream os;
    os << ToLeximited(source.length()) << source;
    return os.str();
  }

  std::string HexEscape(const std::string & source)
  {
    std::istringstream is(source);
    std::ostringstream os;
    os << std::hex;

    u8 ch;
    while (is >> ch)
    {
      if (isgraph(ch) && ch != '%')
      {
        os << ch;
      }
      else
      {
        os << '%';
        if (ch < 16) os << '0';
        os << (u32) ch;
      }
    }

    return os.str();
  }

}
