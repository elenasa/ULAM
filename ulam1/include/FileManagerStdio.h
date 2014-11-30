/**                                        -*- mode:C++ -*-
 * FileMangerStdio.h - Basic File Management of Standard IO for ULAM
 *
 * Copyright (C) 2014 The Regents of the University of New Mexico.
 * Copyright (C) 2014 Ackleyshack LLC.
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
  \author Elenas S. Ackley.
  \author David H. Ackley.
  \date (C) 2014 All rights reserved.
  \gpl
*/


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
