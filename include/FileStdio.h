/**                                        -*- mode:C++ -*-
 * FileStdio.h - Basic File handling of Standard IO for ULAM
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
  \file FileStdio.h - Basic File handling of Standard IO for ULAM
  \author Elena S. Ackley.
  \author David H. Ackley.
  \date (C) 2014-2017 All rights reserved.
  \gpl
*/

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
