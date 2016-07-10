#include <errno.h>
#include "FileStdio.h"


namespace MFM {
      
  FileStdio::FileStdio(FILE * fp, enum Mode mode): m_fp(fp), m_mode(mode) {}


  FileStdio::~FileStdio() 
  {
    close();
  } 

      
  s32 FileStdio::read()
  {
    if( m_mode != READ) return -EACCES;
    if(m_fp == NULL)  return -EPERM;

    //mustn't cast to unsigned char; need EOF to be EOF here
    return fgetc(m_fp);
  }


  s32 FileStdio::write(s32 c)
  {
    s32 r;
    if (m_mode == READ) return -EACCES; 
    if( m_fp == NULL)   return -EPERM;

    if((r = fputc(c, m_fp)) == c)
      r = 0;  //success

    return r;
  }


  s32 FileStdio::close()
  {
    s32 rtn = EOF;  //-1

    //avoid closing a closed file
    if(m_fp != NULL)
      {
	fclose(m_fp);
	rtn = 0;
	m_fp = NULL;
      }

    return rtn;
  }
      
}
