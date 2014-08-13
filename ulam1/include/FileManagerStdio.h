/* -*- c++ -*- */

#ifndef FILEMANAGERSTDIO_H
#define FILEMANAGERSTDIO_H


#include "FileManager.h"

namespace MFM
{

  class FileManagerStdio : public FileManager
    {
    public:
            
      FileManagerStdio(std::string directoryPath = ".");

      ~FileManagerStdio();

      /** First, obtain a complete pathname, which is equal to path if path
       begins with a '/', or is equal to directoryPath+'/'+path otherwise.
       Then, use fopen to attempt to open the file in the appropriate mode.
       If fopen fails, return NULL with errno still set.  Otherwise, return
       a new instance of FileStdio, appropriately initialized based on the
       successful fopen.
      */
      virtual File * open(std::string path, enum Mode mode);

      
    private:
      std::string m_dirPath;
  };
}

    
#endif  /* FILEMANAGERSTDIO_H */
