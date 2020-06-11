/**                                        -*- mode:C++ -*-
 * Locator.h - Basic Location handling of Tokens for ULAM
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
  \file Locator.h - Basic Location handling of Tokens for ULAM
  \author Elena S. Ackley.
  \author David H. Ackley.
  \date (C) 2014-2017 All rights reserved.
  \gpl
*/


#ifndef LOCATOR_H
#define LOCATOR_H

#include "itype.h"

namespace MFM{

class Locator
{
 public:
  Locator(u32 id=0);
  ~Locator();

  u32 getPathIndex() const;
  u32 getFullPathIndex() const;
  u16 getLineNo() const;
  u16 getByteNo() const;

  void setPathIndex(u32 idx);
  void setFullPathIndex(u32 idx);

  void updateLineByteNo(s32 c);

  bool hasNeverBeenRead() const;

 private:
  u32 m_pathIdx;
  u32 m_fullPathIdx;
  u16 m_lineNo;
  u16 m_byteNo;
};

}

#endif  /* LOCATOR_H */
