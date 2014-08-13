/* -*- c++ -*- */

#ifndef FILESTRING_H
#define FILESTRING_H

#include <string>
#include "File.h"

// for the Mode enum
#include "FileManager.h"

namespace MFM
{
  // it helps me to think of this class as a basic iterator
  // the string points to the string owned by the FilemanagerString map

  class FileString : public File
    {
    public:
      FileString(std::string & strptr, enum Mode mode);
      
      ~FileString();

      /**  Reads the next unread byte stored in the string inside the
	   FileString (assuming appropriate open mode, non-error conditions,
	   etc), returning -1 (EOF) if the end of the string has been reached.
      */
      virtual s32 read();
      

      /**  Concatenates the given byte onto the string inside the FileString
	   (assuming appropriate open mode, non-error conditions, etc)
      */
      virtual s32 write(s32);


      virtual s32 close();
     

    private:      
      std::string & m_str;   
      enum Mode m_mode; 
      size_t m_at;
  };
}

#endif  /* FILESTRING_H */
    
