/**                                        -*- mode:C++ -*-
 * NodeVarDecl.h -  Basic Node handling Variable Declarations for ULAM
 *
 * Copyright (C) 2014-2018 The Regents of the University of New Mexico.
 * Copyright (C) 2014-2021 Ackleyshack LLC.
 * Copyright (C) 2020-2021 The Living Computation Foundation
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
  \author Elena S. Ackley.
  \author David H. Ackley.
  \date (C) 2014-2021 All rights reserved.
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

    virtual void resetNodeNo(NNO no); //for constant folding

    virtual bool findNodeNo(NNO n, Node *& foundNode);

    virtual void checkAbstractInstanceErrors();

    virtual void printPostfix(File * f);

    virtual const char * getName();

    virtual u32 getNameId();

    const std::string getMangledName();

    virtual u32 getTypeNameId();

    virtual UTI getTypeDescriptorGivenType();

    virtual ALT getTypeDescriptorRefType();

    virtual const std::string prettyNodeName();

    virtual bool hasASymbol();

    virtual u32 getSymbolId();

    virtual bool getSymbolValue(BV8K& bv);

    virtual bool cloneSymbol(Symbol *& symptrref);

    virtual bool getSymbolPtr(const Symbol *& symptrref);

    bool isAConstantFunctionParameter();

    virtual void setInitExpr(Node * node);

    bool hasInitExpr();

    virtual bool foldArrayInitExpression();

    bool buildDefaultValueForClassConstantInitialization();

    virtual FORECAST safeToCastTo(UTI newType);

    virtual bool checkReferenceCompatibility(UTI uti, Node * parentnode);

    virtual UTI checkAndLabelType(Node * thisparentnode);

    virtual NNO getBlockNo();

    virtual NodeBlock * getBlock();

    virtual void calcMaxDepth(u32& depth, u32& maxdepth, s32 base);

    virtual void printUnresolvedLocalVariables(u32 fid);

    virtual void countNavHzyNoutiNodes(u32& ncnt, u32& hcnt, u32& nocnt);

    virtual EvalStatus eval();

    virtual EvalStatus evalToStoreInto();

    virtual void genCode(File * fp, UVPass& uvpass);

    virtual void genCodeConstantArrayInitialization(File * fp);

    virtual void generateBuiltinConstantClassOrArrayInitializationFunction(File * fp, bool declOnly);

    virtual void generateUlamClassInfo(File * fp, bool declOnly, u32& dmcount);

    virtual void addMemberDescriptionToInfoMap(UTI classType, ClassMemberMap& classmembers);

  protected:
    SymbolVariable * m_varSymbol;
    u32 m_vid; // to instantiate

    Node * m_nodeInitExpr;
    NodeTypeDescriptor * m_nodeTypeDesc; //can be NULL

    virtual void checkForSymbol();
    virtual void clearSymbolPtr();

    bool getNodeTypeDescriptorPtr(const NodeTypeDescriptor *& nodetypedescref);

    virtual void printTypeAndName(File * fp);
    virtual bool checkSafeToCastTo(UTI fromType, UTI& newType);
    EvalStatus evalInitExpr();

    virtual UlamValue makeUlamValuePtr(); //for locals

  private:
    NNO m_currBlockNo;
    NodeBlock * m_currBlockPtr;

    void setBlock(NodeBlock * ptr);

    void setupStackWithPrimitiveForEval(u32 slots);
    void setupStackWithClassForEval(u32 slots);

  };

}

#endif //end NODEVARDECL_H
