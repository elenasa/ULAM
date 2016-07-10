#include <errno.h>
#include <iostream>
#include "FileString.h"

#define EOF (-1)

namespace MFM {

  FileString::FileString(std::string & strptr, enum Mode mode): m_str(strptr), m_mode(mode)
  {
    m_at = 0;  //at the beginning, open
  }


  // The File destructor automatically invoke close()
  FileString::~FileString()
  {
    close();
  }


  s32 FileString::read()
  {
    if(m_mode != READ)
      {
	errno = EACCES;
	return -EACCES;  // mode not for writing
      }

    if(m_at == std::string::npos)
      {
    	errno = EPERM; // operation not permitted
    	return -EPERM;
      }

    if( m_at >= m_str.length()) return -1;  //-1

    //contents of reference to char in string;
    // a \377 will be returned as 255, not EOF (-1)
    return (unsigned char) m_str[m_at++];
  }


  s32 FileString::write(s32 c)
  {
    if(m_mode == READ)
      {
	errno = EACCES;
	return -EACCES;  // mode not for writing
      }

    if(m_at == std::string::npos)
      {
    	errno = EPERM; // operation not permitted
    	return -EPERM;
      }

    m_str.append(1, c);     //expands string size as needed;

    return 0;
  }


  s32 FileString::close()
  {
    s32 r = EOF;  //-1

    //avoid closing if already closed
    if(m_at != std::string::npos)
      {
	m_at = std::string::npos;
	r = 0;
      }

    return r;
  }


}
