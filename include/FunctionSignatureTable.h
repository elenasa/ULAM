/**                                        -*- mode:C++ -*-
 * FunctionSignatureTable.h - Table of function signatures for ULAM
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
  \file FunctionSignatureTable.h -  Table of function signatures for ULAM
  \author Elena S. Ackley.
  \author David H. Ackley.
  \date (C) 2019 All rights reserved.
  \gpl
*/

#ifndef FUNCTIONSIGNATURETABLE_H
#define FUNCTIONSIGNATURETABLE_H

#include <map>

namespace MFM
{
  struct FSEntry
  {
    UTI m_rtnType; //can never be different!
    UTI m_ofClassUTI; //first seen in this class
    bool m_isVirtual; //not dup when virtual
    bool m_hasProblem;
    u32 m_matchesFound; //used to detect duplicate func signatures
  };

  // key is SF's getMangledNameWithTypes string
  typedef std::map<std::string, struct FSEntry> FSTable;
}

#endif  /* FUNCTIONSIGNATURETABLE_H */
