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

    os << ToLeximitedHeader(digits) << num;
    return os.str();
  } //ToLeximitedNumber (signed)


  std::string ToLeximitedNumber(u32 num)
  {
    //returns 10 when 'num' is zero
    u32 digits = (num == 0 ? 1 : DigitCount(num, 10));

    std::ostringstream os;
    os << ToLeximitedHeader(digits) << num;

    return os.str();
  } //ToLeximitedNumber (unsigned)


  std::string ToLeximitedHeader(u32 len)
  {
    std::ostringstream os;
    if (len < 9)
      os << len;
    else
      os << 9 << ToLeximitedHeader(DigitCount(len,10)) << len;
    return os.str();
  }

  //support for 64-bit
  std::string ToLeximitedNumber64(s64 num)
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
        if (num == S64_MIN)
          {
            num = 0;
            digits = 1;
          }
        else
          {
            num = -num;
            digits = DigitCount64((u64) num, 10);
          }
	useneg = true;
      }
    else //>0
      {
	digits = DigitCount64((u64) num, 10);
      }

    std::ostringstream os;

    if(useneg)
      os << "n";

    os << ToLeximitedHeader(digits) << num;
    return os.str();
  } //ToLeximitedNumber64 (signed)

  std::string ToLeximitedNumber64(u64 num)
  {
    //returns 10 when 'num' is zero
    u32 digits = (num == 0 ? 1 : DigitCount64(num, 10));

    std::ostringstream os;
    os << ToLeximitedHeader(digits) << num;

    return os.str();
  } //ToLeximitedNumber64 (unsigned)

  std::string ToLeximited(const std::string & source)
  {
    std::ostringstream os;
    os << ToLeximitedHeader(source.length()) << source;
    return os.str();
  }

  std::string HexEscape(const std::string & source)
  {
    std::istringstream is(source);
    std::ostringstream os;
    os << std::hex;
    is >> std::noskipws;

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

  std::string ToSignedDecimal(const s64 data)
  {
    // We must avoid printfs involving 64 bit quantities,
    // due to ANSI C99 limitations, so we do this instead.
    std::stringstream os;
    //os << std::dec << data;
    u32 svalhi = (u32) _ShiftFromBitNumber64((u64) data, 32);
    u32 svallo = (u32) (_GetMask64(0, 32) & data);
    os << "HexU64(" << std::hex << "0x" << svalhi << ", " << std::hex << "0x" << svallo << ")";
    return os.str();
  }

  std::string ToUnsignedDecimal(const u64 data)
  {
    // We must avoid printfs involving 64 bit quantities,
    // due to ANSI C99 limitations, so we do this instead.
    std::stringstream os;
    //os << std::dec << data;
    u32 uvalhi = (u32) _ShiftFromBitNumber64(data, 32);
    u32 uvallo = (u32) (_GetMask64(0, 32) & data);
    os << "HexU64(" << std::hex << "0x" << uvalhi << ", " << std::hex << "0x" << uvallo << ")";
    return os.str();
  }

}
