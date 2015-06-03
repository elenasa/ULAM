#include <stdio.h>
#include "File.h"

namespace MFM {

  File::File(): m_lastRead(0), m_haveLastRead(false) {}

  File::~File(){}

  s32 File::peek()
  {
    if(! m_haveLastRead)
      {
	m_lastRead = read();
	m_haveLastRead = true;
      }
    return m_lastRead;
  }

  s32 File::consume()
  {
    s32 ret = peek();
    m_haveLastRead = false;
    return ret;
  }

  s32 File::write(const char * data)
  {
    s32 i = 0;
    s32 r = 0;
    while ( data[i] != '\0')
      {
	r = write((s32) data[i]);
	if(r != 0) break;
	i++;
      }
    return r;
  }

  s32 File::write_decimal(const s32 data)
    {
      char tmp[32+3];
      sprintf(tmp,"%d",data);
      return write(tmp);
    }

  s32 File::write_decimal_long(const s64 data)
    {
      char tmp[64+3];
      sprintf(tmp,"%ld",data);
      return write(tmp);
    }

} //end MFM
