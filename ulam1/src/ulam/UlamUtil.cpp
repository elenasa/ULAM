#include "UlamUtil.h"
#include <sstream>  /* For ostringstream */
#include <cctype>   /* For isgraph */
#include <time.h>   /* For nanosleep */

namespace MFM
{
  u32 DigitCount(u32 num, u32 base)
  {
    u32 count = 0;
    while(num)
    {
      count++;
      num /= base;
    }
    return count;
  }

  std::string ToLeximited(u32 len)
  {
    u32 digits = DigitCount(len,10);

    std::ostringstream os;
    if (digits < 9)
      os << digits << len;
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

  void Sleep(u32 seconds, u64 nanos)
  {
    struct timespec tspec;
    tspec.tv_sec = seconds;
    tspec.tv_nsec = nanos;

    nanosleep(&tspec, NULL);
  }



}
