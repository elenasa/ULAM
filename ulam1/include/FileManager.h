/* -*- c++ -*- */

#ifndef FILEMANAGER_H
#define FILEMANAGER_H

#include <string>
#include "File.h"


namespace MFM
{

  /** enum Mode is a member of FileManager and defines at least the
   constants READ, WRITE, APPEND, and EXTEND.  (EXTEND is like APPEND
   except APPEND will attempt to create the file if it doesn't already
   exist, while EXTEND fails unless the file does already exist.)
  */
  enum Mode { READ = 0, WRITE, APPEND, EXTEND };

  class FileManager
  {
  public:
    FileManager();
    virtual ~FileManager();
    
    /** Attempt to open the path in the given mode.  Return a
	newly-allocated instance of File, appropriately initialized, on
	success; return NULL with errno set appropriately on failure.  Note
	that on a non-NULL return, the return value becomes the caller's
	responsibility to delete.
    */
    virtual File * open(std::string path, Mode mode) = 0;

  private:

  };
}

#endif  /* FILEMANAGER_H */
