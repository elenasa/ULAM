/**                                        -*- mode:C++ -*-
 * ClassContext.h - Data for Class Context changes for ULAM
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
  \file ClassContext.h - Data for Class Context changes for ULAM
  \author Elenas S. Ackley.
  \author David H. Ackley.
  \date (C) 2015 All rights reserved.
  \gpl
*/


#ifndef CLASSCONTEXT_H
#define CLASSCONTEXT_H

#include "itype.h"
#include "NodeBlock.h"
#include "NodeBlockClass.h"
#include "Constants.h"

namespace MFM{

class ClassContext
{
 public:
  ClassContext();
  ClassContext(u32 id, UTI idx, NodeBlock * nb, NodeBlockClass * nbc, bool usememberblock, NodeBlockClass * mbc);
  ~ClassContext();

  /** for context switching */
  u32 getCompileThisId();

  UTI getCompileThisIdx();

  /** use when changing m_compileThisIdx to keep id and idx in sync */
  void setCompileThisIdx(u32 id, UTI idx);

  NodeBlock * getCurrentBlock();
  void setCurrentBlock(NodeBlock * nb);

  NodeBlockClass * getClassBlock();
  void setClassBlock(NodeBlockClass * nbc);

  bool useMemberBlock();
  void useMemberBlock(bool use);

  NodeBlockClass * getCurrentMemberClassBlock();
  void setCurrentMemberClassBlock(NodeBlockClass * nbc);

 private:
    u32 m_compileThisId;                 // the subject of this compilation; id into m_pool
    UTI m_compileThisIdx;                // the subject of this compilation; various class instances
    NodeBlock *      m_currentBlock;     //replaces m_stackOfBlocks
    NodeBlockClass * m_classBlock;       //holds current class ST with function defs
    bool m_useMemberBlock;               // used during parsing and c&l member select expression
    NodeBlockClass * m_currentMemberClassBlock;
};

} //MFM

#endif  /* LOCATOR_H */
