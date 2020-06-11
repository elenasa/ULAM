/**                                        -*- mode:C++ -*-
 * BaseClassTable.h - Table of base classes for ULAM
 *
 * Copyright (C) 2019 The Regents of the University of New Mexico.
 * Copyright (C) 2019 Ackleyshack LLC.
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
  \file BaseClassTable.h -  Table of base classes for ULAM
  \author Elena S. Ackley.
  \author David H. Ackley.
  \date (C) 2019 All rights reserved.
  \gpl
*/

#ifndef BASECLASSTABLE_H
#define BASECLASSTABLE_H

#include <vector>
#include <map>
#include "Constants.h"

namespace MFM
{
  struct BaseClassEntry
  {
    UTI m_base;
    u32 m_numbaseshared; //rounds out the struct size
    s32 m_basepos; //UNKNOWN < 0
  };

  // key is base class index in this class
  typedef std::vector<struct BaseClassEntry> BasesTable;

  // key is base class type, value is the index into BasesTable
  typedef std::map<UTI, u32> BasesTableTypeMap;

}

#endif  /* BASECLASSTABLE_H */
