/**                                        -*- mode:C++ -*-
 * NodeTypedef.h - Basic Node handling Typedefs for ULAM
 *
 * Copyright (C) 2014-2015 The Regents of the University of New Mexico.
 * Copyright (C) 2014-2015 Ackleyshack LLC.
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
  \file NodeTypedef.h - Basic Node handling Typedefs for ULAM
  \author Elenas S. Ackley.
  \author David H. Ackley.
  \date (C) 2014-2015 All rights reserved.
  \gpl
*/


#ifndef NODETYPEDEF_H
#define NODETYPEDEF_H

#include "Node.h"
#include "SymbolTypedef.h"
#include "NodeBlock.h"
#include "NodeTypeDescriptor.h"

namespace MFM{

  class NodeTypedef : public Node
  {
  public:

    NodeTypedef(SymbolTypedef * sym, NodeTypeDescriptor * nodetype, CompilerState & state);

    NodeTypedef(const NodeTypedef& ref);

    virtual ~NodeTypedef();

    virtual Node * instantiate();

    virtual void updateLineage(NNO pno);

    virtual bool findNodeNo(NNO n, Node *& foundNode);

    virtual void printPostfix(File * f);

    virtual const char * getName();

    virtual const std::string prettyNodeName();

    virtual bool getSymbolPtr(Symbol *& symptrref);

    virtual UTI checkAndLabelType();

    NNO getBlockNo();

    NodeBlock * getBlock();

    virtual void packBitsInOrderOfDeclaration(u32& offset);

    virtual void countNavHzyNoutiNodes(u32& ncnt, u32& hcnt, u32& nocnt);

    virtual bool buildDefaultQuarkValue(u32& dqref);

    virtual EvalStatus eval();

    virtual void genCode(File * fp, UlamValue& uvpass);

    virtual void generateUlamClassInfo(File * fp, bool declOnly, u32& dmcount);

  protected:
    virtual void checkForSymbol();

  private:
    SymbolTypedef * m_typedefSymbol;
    u32 m_tdid; //to instantiate
    NNO m_currBlockNo;
    NodeTypeDescriptor * m_nodeTypeDesc; //can be NULL
  };

}

#endif //end NODETYPEDEF_H
