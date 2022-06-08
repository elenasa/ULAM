/**                                        -*- mode:C++ -*-
 * FileMangerStrings.h - Basic File Management of Strings for ULAM
 *
 * Copyright (C) 2014-2017 The Regents of the University of New Mexico.
 * Copyright (C) 2014-2017 Ackleyshack LLC.
 *
 * This file is part of the ULAM programming language compilation system.
 *
 * The ULAM programming language compilation system is free software:
 * you can redistribute it and/or modify it under the terms of the GNU
 * General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or (at your option)
 * any later version.
 *
 * The ULAM programming language compilation system is distributed in
 * the hope that it will be useful, but WITHOUT ANY WARRANTY; without
 * even the implied warranty of MERCHANTABILITY or FITNESS FOR A
 * PARTICULAR PURPOSE.  See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License
 * along with the ULAM programming language compilation system
 * software.  If not, see <http://www.gnu.org/licenses/>.
 *
 * @license GPL-3.0+ <http://spdx.org/licenses/GPL-3.0+>
 */

/**
  \file FileMangerStrings.h - Basic File Management of Strings for ULAM
  \author Elena S. Ackley.
  \author David H. Ackley.
  \date (C) 2014-2017 All rights reserved.
  \gpl
*/


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
      virtual File * open(const std::string path, Mode mode, std::string & resultpath);

      virtual File * open(const std::string path, Mode mode)
      {
        std::string ignored;
        return open(path, mode, ignored);
      }

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
