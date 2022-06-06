/**                                        -*- mode:C++ -*-
 * FileManager.h - Basic File Management for ULAM
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
  \file FileManager.h - Basic File Management for ULAM
  \author Elena S. Ackley.
  \author David H. Ackley.
  \date (C) 2014-2017 All rights reserved.
  \gpl
*/

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
    virtual File * open(const std::string path, Mode mode) = 0;

    /** Attempt to open the path in the given mode, recording the
	actual path found.  On success, return a newly-allocated
	instance of File, appropriately initialized, and also set
	resultpath to be path that was opened successfully; on
	failure, return NULL with errno set appropriately, and
	resultpath is unmodified.  Note that on a non-NULL return, the
	return value becomes the caller's responsibility to delete.
    */
    virtual File * open(const std::string path, Mode mode, std::string & resultpath) = 0;

  private:

  };
}

#endif  /* FILEMANAGER_H */
