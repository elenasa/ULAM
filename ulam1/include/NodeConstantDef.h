/**                                        -*- mode:C++ -*-
 * NodeConstantDef.h - Node handling Constant Definition for ULAM
 *
 * Copyright (C) 2015 The Regents of the University of New Mexico.
 * Copyright (C) 2015 Ackleyshack LLC.
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
  \file NodeConstantDef.h - Node handling Constant Definition for ULAM
  \author Elenas S. Ackley.
  \author David H. Ackley.
  \date (C) 2015 All rights reserved.
  \gpl
*/


#ifndef NODECONSTANTDEF_H
#define NODECONSTANTDEF_H

#include "Node.h"
#include "NodeBlock.h"
#include "SymbolConstantValue.h"
#include "NodeTypeDescriptor.h"

namespace MFM{

  class NodeConstantDef : public Node
  {
  public:

    NodeConstantDef(SymbolConstantValue * symptr, NodeTypeDescriptor * nodetype, CompilerState & state);
    NodeConstantDef(const NodeConstantDef& ref);

    virtual ~NodeConstantDef();

    virtual Node * instantiate();

    virtual void updateLineage(NNO pno);

    virtual bool exchangeKids(Node * oldnptr, Node * newnptr);

    virtual bool findNodeNo(NNO n, Node *& foundNode);

    virtual void printPostfix(File * f);

    virtual const char * getName();

    virtual const std::string prettyNodeName();

    virtual bool getSymbolPtr(Symbol *& symptrref);

    void setSymbolPtr(SymbolConstantValue * cvsymptr);

    u32 getSymbolId();

    virtual UTI checkAndLabelType();

    virtual void countNavNodes(u32& cnt);

    NNO getBlockNo();

    void setBlockNo(NNO n);

    NodeBlock * getBlock();

    void setConstantExpr(Node * node);

    bool foldConstantExpression();

    void fixPendingArgumentNode();

    virtual bool assignClassArgValueInStubCopy();

    virtual EvalStatus eval();

    virtual void packBitsInOrderOfDeclaration(u32& offset);

    virtual void genCode(File * fp, UlamValue& uvpass);

    virtual void generateUlamClassInfo(File * fp, bool declOnly, u32& dmcount);

  private:
    SymbolConstantValue * m_constSymbol;
    Node * m_nodeExpr;
    NNO m_currBlockNo;
    u32 m_cid; //to instantiate
    NodeTypeDescriptor * m_nodeTypeDesc; //can be NULL

    bool updateConstant(u32 & newconst);
  };

} //MFM

#endif //NODECONSTANTDEF_H
