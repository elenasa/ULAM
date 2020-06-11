/**                                        -*- mode:C++ -*-
 * VirtualTable.h - Table of virtual functions for ULAM
 *
 * Copyright (C) 2015-2019 The Regents of the University of New Mexico.
 * Copyright (C) 2015-2019 Ackleyshack LLC.
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
  \file VirtualTable.h -  Table of virtual functions for ULAM
  \author Elena S. Ackley.
  \author David H. Ackley.
  \date (C) 2015-2019 All rights reserved.
  \gpl
*/

#ifndef VIRTUALTABLE_H
#define VIRTUALTABLE_H

#include <vector>
#include "SymbolFunction.h"

namespace MFM
{
  class SymbolFunction; //forward
  struct VTEntry
  {
    SymbolFunction * m_funcPtr;
    UTI m_ofClassUTI;
    UTI m_origClassUTI;
    u32 m_origvidx;
    bool m_isPure;
  };

  // key is virtual index in originating class + start index of originating base class
  typedef std::vector<struct VTEntry> VT;

}

#endif  /* VIRTUALTABLE_H */
