/**                                        -*- mode:C++ -*-
 * NodeConstantDef.h - Node handling Constant Definition for ULAM
 *
 * Copyright (C) 2015-2019 The Regents of the University of New Mexico.
 * Copyright (C) 2015-2019 Ackleyshack LLC.
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
  \author Elena S. Ackley.
  \author David H. Ackley.
  \date (C) 2015-2019 All rights reserved.
  \gpl
*/


#ifndef NODECONSTANTDEF_H
#define NODECONSTANTDEF_H

#include "Node.h"
#include "NodeBlock.h"
#include "SymbolWithValue.h"
#include "NodeTypeDescriptor.h"

namespace MFM{

  class NodeConstantDef : public Node
  {
  public:

    NodeConstantDef(SymbolWithValue * symptr, NodeTypeDescriptor * nodetype, CompilerState & state);
    NodeConstantDef(const NodeConstantDef& ref);

    virtual ~NodeConstantDef();

    virtual Node * instantiate();

    virtual void updateLineage(NNO pno);

    virtual bool exchangeKids(Node * oldnptr, Node * newnptr);

    virtual bool findNodeNo(NNO n, Node *& foundNode);

    virtual void checkAbstractInstanceErrors();

    virtual void resetNodeLocations(Locator loc);

    virtual void printPostfix(File * f);

    virtual void noteTypeAndName(UTI cuti, s32 totalsize, u32& accumsize);

    virtual void genTypeAndNameEntryAsComment(File * fp, s32 totalsize, u32& accumsize);

    virtual const char * getName();

    virtual u32 getTypeNameId();

    virtual const std::string prettyNodeName();

    virtual bool getSymbolPtr(Symbol *& symptrref);

    virtual void setSymbolPtr(SymbolWithValue * cvsymptr);

    virtual u32 getSymbolId();

    virtual bool getNodeTypeDescriptorPtr(NodeTypeDescriptor *& nodetypedescref);

    bool setNodeTypeDescriptor(NodeTypeDescriptor * nodetypedesc);

    virtual bool hasDefaultSymbolValue();

    virtual UTI checkAndLabelType();

    virtual void countNavHzyNoutiNodes(u32& ncnt, u32& hcnt, u32& nocnt);

    virtual NNO getBlockNo();

    virtual void setBlockNo(NNO n);

    virtual NodeBlock * getBlock();

    virtual void setConstantExpr(Node * node);

    virtual bool hasConstantExpr();

    virtual bool isReadyConstant();

    virtual UTI foldConstantExpression();

    virtual bool foldArrayInitExpression();

    virtual bool buildDefaultValue(u32 wlen, BV8K& dvref);

    virtual bool buildDefaultValueForClassConstantDefs();

    virtual void fixPendingArgumentNode();

    virtual bool assignClassArgValueInStubCopy();

    bool cloneTypeDescriptorForPendingArgumentNode(NodeConstantDef * templateparamdef);

    virtual EvalStatus eval();

    virtual TBOOL packBitsInOrderOfDeclaration(u32& offset);

    virtual void printUnresolvedVariableDataMembers();

    virtual void printUnresolvedLocalVariables(u32 fid);

    virtual void genCode(File * fp, UVPass& uvpass);

    virtual void genCodeConstantArrayInitialization(File * fp);

    virtual void generateBuiltinConstantClassOrArrayInitializationFunction(File * fp, bool declOnly);

    virtual void cloneAndAppendNode(std::vector<Node *> & cloneVec);

    virtual void generateTestInstance(File * fp, bool runtest);

    virtual void generateUlamClassInfo(File * fp, bool declOnly, u32& dmcount);

    virtual void addMemberDescriptionToInfoMap(UTI classType, ClassMemberMap& classmembers);

  protected:
    SymbolWithValue * m_constSymbol;
    Node * m_nodeExpr;
    NodeTypeDescriptor * m_nodeTypeDesc; //can be NULL
    u32 m_cid; //to instantiate

    virtual void checkForSymbol();

    virtual bool isDataMemberInit();

  private:
    NNO m_currBlockNo;
    NodeBlock * m_currBlockPtr;

    void setBlock(NodeBlock * ptr);

    void setupStackWithPrimitiveForEval(u32 slots);
    void setupStackWithConstantClassForEval(u32 slots);
    void assignConstantSlotIndex(u32& cslotidx);

  };

} //MFM

#endif //NODECONSTANTDEF_H
