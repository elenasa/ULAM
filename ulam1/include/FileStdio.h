/* -*- c++ -*- */

#ifndef FILESTDIO_H
#define FILESTDIO_H

#include <stdio.h>
#include "File.h"
#include "FileManager.h"

namespace MFM
{

  class FileStdio : public File
    {
    public:
      FileStdio(FILE * fp, enum Mode mode);
      
      ~FileStdio();

      /** Uses fgetc to read from a FILE* stored inside the FileStdio
	  (assuming appropriate open mode, non-error conditions, etc)
      */
      virtual s32 read();
      
      /** Uses fputc to write to a FILE* stored inside the FileStdio (assuming
	  appropriate open mode, non-error conditions, etc)
      */
      virtual s32 write(s32);

      
      /** Uses fclose to close the FILE* stored inside the FileStdio, if it
	  has not already been closed.
      */
      virtual s32 close();
      
      
    private:
      
      FILE * m_fp;
      enum Mode m_mode; 
  };
}

#endif  /* FILESTDIO_H */
    
