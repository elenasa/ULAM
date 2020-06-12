/**                                        -*- mode:C++ -*-
 * TargetMap.h - Map of class targets for ULAM
 *
 * Copyright (C) 2015-2017 The Regents of the University of New Mexico.
 * Copyright (C) 2015-2017 Ackleyshack LLC.
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
  \file TargetMap.h -  Map of class targets for ULAM
  \author Elena S. Ackley.
  \author David H. Ackley.
  \date (C) 2015-2017 All rights reserved.
  \gpl
*/

#ifndef TARGETMAP_H
#define TARGETMAP_H

#include <map>
#include <string>
#include "itype.h"
#include "Locator.h"
#include "Constants.h"

namespace MFM
{
  struct TargetDesc
  {
    bool m_hasTest;
    ULAMCLASSTYPE m_classType;
    u32 m_bitsize;
    Locator m_loc;
    std::string m_className;
    std::string m_classSignature;
    std::string m_baseClassSignature;
    std::string m_structuredComment;
  };

  // key is class mangled name
  typedef std::map<std::string, struct TargetDesc> TargetMap;
}

#endif  /* TARGETMAP_H */
