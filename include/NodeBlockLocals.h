/**                                        -*- mode:C++ -*-
 * NodeBlockLocals.h - Node for handling Local Defs for ULAM
 *
 * Copyright (C) 2014-2020 The Regents of the University of New Mexico.
 * Copyright (C) 2014-2020 Ackleyshack LLC.
 * Copyright (C) 2020 The Living Computation Foundation.
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
  \file NodeBlockLocals.h - Node for handling Local Defs for ULAM
  \author Elena S. Ackley.
  \author David H. Ackley.
  \date (C) 2014-2020 All rights reserved.
  \gpl
*/


#ifndef NODEBLOCKLOCALS_H
#define NODEBLOCKLOCALS_H

#include "NodeBlockContext.h"


namespace MFM{

  class NodeBlockLocals : public NodeBlockContext
  {
  public:

    NodeBlockLocals(NodeBlock * prevBlockNode, CompilerState & state);

    NodeBlockLocals(const NodeBlockLocals& ref);

    virtual ~NodeBlockLocals();

    virtual Node * instantiate();

    virtual void updateLineage(NNO pno);

    virtual void printPostfix(File * fp);

    virtual const char * getName();

    virtual const std::string prettyNodeName();

    virtual bool isAClassBlock();

    virtual UTI checkAndLabelType();

    virtual void calcMaxDepth(u32& depth, u32& maxdepth, s32 base);

    virtual void genCode(File * fp, UVPass& uvpass);

    virtual void cloneAndAppendNode(std::vector<Node *> & cloneVec);

    virtual void addTargetDescriptionToInfoMap(TargetMap& classtargets, u32 scid);
    virtual void addMemberDescriptionsToInfoMap(ClassMemberMap& classmembers);

    virtual void generateTestInstance(File * fp, bool runtest);

    bool assignRegistrationNumberToLocalsBlock(u32 n);
    u32 getRegistrationNumberForLocalsBlock();

  protected:


  private:
    // registration number saved here since no SymbolClass until made into temp class for code gen
    u32 m_registryNumberLocalsSafe; //ulam-4

  };

}

#endif //end NODEBLOCKLOCALS_H
