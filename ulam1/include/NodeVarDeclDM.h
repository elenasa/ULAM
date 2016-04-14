/**                                        -*- mode:C++ -*-
 * NodeVarDeclDM.h -  Data Member Variable Declarations for ULAM
 *
 * Copyright (C) 2015-2016 The Regents of the University of New Mexico.
 * Copyright (C) 2015-2016 Ackleyshack LLC.
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
  \file NodeVarDeclDM.h -  Data Member Variable Declarations for ULAM
  \author Elenas S. Ackley.
  \author David H. Ackley.
  \date (C) 2015-2016 All rights reserved.
  \gpl
*/


#ifndef NODEVARDECLDM_H
#define NODEVARDECLDM_H

#include "NodeVarDecl.h"

namespace MFM{

  class NodeVarDeclDM : public NodeVarDecl
  {
  public:

    NodeVarDeclDM(SymbolVariable * sym, NodeTypeDescriptor * nodetype, CompilerState & state);

    NodeVarDeclDM(const NodeVarDeclDM& ref);

    virtual ~NodeVarDeclDM();

    virtual Node * instantiate();

    virtual void updateLineage(NNO pno);

    virtual bool findNodeNo(NNO n, Node *& foundNode);

    virtual void printPostfix(File * f);

    virtual const char * getName();

    virtual const std::string prettyNodeName();

    virtual FORECAST safeToCastTo(UTI newType);

    virtual UTI checkAndLabelType();

    virtual void countNavHzyNoutiNodes(u32& ncnt, u32& hcnt, u32& nocnt);

    virtual bool buildDefaultQuarkValue(u32& dqref);

    virtual void setInitExpr(Node * node); //was setConstantExpr

    virtual bool foldInitExpression(); //was foldConstantExpression

    virtual void packBitsInOrderOfDeclaration(u32& offset);

    virtual void printUnresolvedVariableDataMembers();

    virtual void printUnresolvedLocalVariables();

    virtual EvalStatus eval();

    virtual EvalStatus evalToStoreInto();

    virtual void genCode(File * fp, UlamValue& uvpass);

    virtual void generateUlamClassInfo(File * fp, bool declOnly, u32& dmcount);

  protected:

  private:

    bool updateConstant(u64 & newconst);
    bool updateConstant32(u64 & newconst);
    bool updateConstant64(u64 & newconst);

    bool foldDefaultQuark(u32 dq);
    void genCodedBitFieldTypedef(File * fp, UlamValue& uvpass);

  };
} //MFM

#endif //end NODEVARDECLDM_H
