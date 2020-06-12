/**                                        -*- mode:C++ -*-
 * FileMangerStdio.h - Basic File Management of Standard IO for ULAM
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
  \file  FileManagerStdio.h - Basic File Management of Standard IO for ULAM
  \author Elena S. Ackley.
  \author David H. Ackley.
  \date (C) 2014-2017 All rights reserved.
  \gpl
*/


#ifndef FILEMANAGERSTDIO_H
#define FILEMANAGERSTDIO_H


#include "FileManager.h"
#include <vector>

namespace MFM
{

  class FileManagerStdio : public FileManager
    {
    public:

      FileManagerStdio(std::string directoryPath = ".");

      ~FileManagerStdio();

      /** Attempt to open the given path.  Operates exactly like
          open(const std::string, Mode, std::string&) except the
          resulting path is discarded.

          \sa open(const std::string path, Mode mode, std::string & resultpath)
      */
      virtual File * open(const std::string path, Mode mode)
      {
        std::string ignored;
        return open(path, mode, ignored);
      }

      /** Attempt to open the given path, and record the resulting
          path used if the open ultimately succeeds.  The given path
          is opened either directly (if it is an absolute path), or by
          prefixing it with a directory path (if it is a relative
          path).  For relative paths, the following directories are
          considered in order: (1) The directoryPath supplied in the
          constructor (default "."), (2) Any directories added via
          addReadDir, in the order they were added.

          The first resulting filename that can be opened in the
          appropriate mode is used.  If none work, return NULL with
          errno set to the (last or only) fopen error.  Otherwise,
          return a new instance of FileStdio, appropriately
          initialized based on the successful fopen.

          \sa open(const std::string path, Mode mode)
      */
      virtual File * open(const std::string path, Mode mode, std::string & resultpath);

      void addReadDir(std::string readDir);

    private:
      std::string m_dirPath;

      /**
         Additional directories to search during open for read with a
         relative path.
       */

      std::vector<std::string> m_otherReadDirs;
  };
}


#endif  /* FILEMANAGERSTDIO_H */
