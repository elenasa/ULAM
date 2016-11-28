/**                                        -*- mode:C++ -*-
 * ClassContext.h - Data for Class Context changes for ULAM
 *
 * Copyright (C) 2014-2016 The Regents of the University of New Mexico.
 * Copyright (C) 2014-2016 Ackleyshack LLC.
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
  \date (C) 2014-2016 All rights reserved.
  \gpl
*/


#ifndef CLASSCONTEXT_H
#define CLASSCONTEXT_H

#include "itype.h"
#include "NodeBlock.h"
#include "NodeBlockClass.h"
#include "NodeBlockContext.h"
#include "Constants.h"
#include <string.h>
namespace MFM{

class ClassContext
{
 public:
  ClassContext();
  ClassContext(u32 id, UTI idx, NodeBlock * nb, NodeBlockContext * nbc, bool usememberblock, NodeBlockClass * mbc);
  ~ClassContext();

  /** for context switching */
  u32 getCompileThisId();

  UTI getCompileThisIdx();

  NodeBlock * getCurrentBlock();
  void setCurrentBlock(NodeBlock * nb);

  NodeBlockContext * getContextBlock();
  void setContextBlock(NodeBlockContext * nbc);

  bool useMemberBlock();
  void useMemberBlock(bool use);

  NodeBlockClass * getCurrentMemberClassBlock();
  void setCurrentMemberClassBlock(NodeBlockClass * nbc);

  std::string getClassContextAsString(); //debugging

 private:
    u32 m_compileThisId;                 // the subject of this compilation; id into m_pool
    UTI m_compileThisIdx;                // the subject of this compilation; various class instances
    NodeBlock * m_currentBlock;     // replaces m_stackOfBlocks
    NodeBlockContext * m_contextBlock;   // holds current class blockST w func defs; or localdef blockST
    bool m_useMemberBlock;               // used during parsing and c&l member select expression
    NodeBlockClass * m_currentMemberClassBlock;
};

} //MFM

#endif  /* LOCATOR_H */
