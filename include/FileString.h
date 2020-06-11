/**                                        -*- mode:C++ -*-
 * FileString.h - Basic File handling in Strings for ULAM
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
  \file FileString.h - Basic File handling in Strings for ULAM
  \author Elena S. Ackley.
  \author David H. Ackley.
  \date (C) 2014-2017 All rights reserved.
  \gpl
*/

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
