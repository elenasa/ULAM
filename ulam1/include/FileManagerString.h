/* -*- c++ -*- */

#ifndef FILEMANAGERSTRING_H
#define FILEMANAGERSTRING_H

#include <map>
#include "FileManager.h"

namespace MFM
{

  class FileManagerString : public FileManager
    {
    public:
            

      /** Create a FileManagerString based on the given directoryPath
	  A FileManagerString maintains an internal map from paths to file
	  contents, representing all the files that have been created in that
	  FileManagerString (via open() calls with mode WRITE or APPEND).
      */
      FileManagerString(std::string directoryPath = ".");

      ~FileManagerString();


      /**  First, obtain a complete pathname, which is equal to path if path
	   begins with a '/', or is equal to directoryPath+'/'+path otherwise.
	   Then, use the internal map to attempt to open the File in the
	   appropriate mode.  If this fails (e.g., mode is READ or EXTEND but
	   the path doesn't exist in the map), return NULL with errno set to an
	   appropriate value.  Otherwise, return a new instance of FileString,
	   appropriately initialized based on the data from the internal map.
      */
      virtual File * open(std::string path, enum Mode mode);

      /**  Convenience routine for creating files in a FileManagerString.  May
        be implemented otherwise, but its actions are equivalent to: Opens
        the given path for WRITE, writes all the bytes of data to the
        resulting FileString, then closes and deletes the FileString.
        Returns true if all succeeded, false if any problem occurred.
      */
      bool add(std::string path, std::string data);

      /**  Convenience routine for getting all the bytes of data into
	   second arg string. Returns true if all succeeded, false if
	   any problem occurred.
      */
      bool get(std::string path, std::string& data);

    private:
      std::string m_dirPath;
      std::map <std::string,std::string> m_pathToContents;
  };
}

    
#endif  /* FILEMANAGERSTRING_H */
