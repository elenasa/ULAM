/**                                        -*- mode:C++ -*-
 * NodeVarDecl.h -  Basic Node handling Variable Declarations for ULAM
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
  \file NodeVarDecl.h -  Basic Node handling Variable Declarations for ULAM
  \author Elenas S. Ackley.
  \author David H. Ackley.
  \date (C) 2014-2015 All rights reserved.
  \gpl
*/


#ifndef NODEVARDECL_H
#define NODEVARDECL_H

#include "Node.h"
#include "SymbolVariable.h"
#include "NodeBlock.h"
#include "NodeTypeDescriptor.h"

namespace MFM{

  class NodeVarDecl : public Node
  {
  public:

    NodeVarDecl(SymbolVariable * sym, NodeTypeDescriptor * nodetype, CompilerState & state);

    NodeVarDecl(const NodeVarDecl& ref);

    virtual ~NodeVarDecl();

    virtual Node * instantiate();

    virtual void updateLineage(NNO pno);

    virtual bool exchangeKids(Node * oldnptr, Node * newnptr);

    virtual bool findNodeNo(NNO n, Node *& foundNode);

    virtual void checkAbstractInstanceErrors();

    virtual void printPostfix(File * f);

    virtual const char * getName();

    virtual const std::string prettyNodeName();

    virtual bool getSymbolPtr(Symbol *& symptrref);

    virtual void setInitExpr(Node * node);

    bool hasInitExpr();

    virtual bool foldInitExpression();

    virtual FORECAST safeToCastTo(UTI newType);

    virtual UTI checkAndLabelType();

    virtual NNO getBlockNo();

    virtual NodeBlock * getBlock();

    virtual void packBitsInOrderOfDeclaration(u32& offset);

    virtual void calcMaxDepth(u32& depth, u32& maxdepth, s32 base);

    virtual void countNavHzyNoutiNodes(u32& ncnt, u32& hcnt, u32& nocnt);

    virtual EvalStatus eval();

    virtual EvalStatus evalToStoreInto();

    virtual void genCode(File * fp, UlamValue& uvpass);

    virtual void generateUlamClassInfo(File * fp, bool declOnly, u32& dmcount);

  protected:
    SymbolVariable * m_varSymbol;
    u32 m_vid; // to instantiate

    Node * m_nodeInitExpr;

    virtual void checkForSymbol();
    virtual void printTypeAndName(File * fp);

    EvalStatus evalInitExpr();

  private:
    NNO m_currBlockNo;
    NodeTypeDescriptor * m_nodeTypeDesc; //can be NULL

    UlamValue makeUlamValuePtr(); //for locals
  };

}

#endif //end NODEVARDECL_H
