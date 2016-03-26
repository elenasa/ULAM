/**                                        -*- mode:C++ -*-
 * NodeBlockClass.h - Basic Node for handling Classes for ULAM
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
  \file NodeBlockClass.h - Basic Node for handling Classes for ULAM
  \author Elenas S. Ackley.
  \author David H. Ackley.
  \date (C) 2014-2016 All rights reserved.
  \gpl
*/


#ifndef NODEBLOCKCLASS_H
#define NODEBLOCKCLASS_H

#include "NodeBlock.h"
#include "NodeBlockFunctionDefinition.h"
#include "NodeList.h"
#include "Symbol.h"

namespace MFM{

  class NodeBlockClass : public NodeBlock
  {
  public:

    NodeBlockClass(NodeBlock * prevBlockNode, CompilerState & state, NodeStatements * s = NULL);
    NodeBlockClass(const NodeBlockClass& ref);
    virtual ~NodeBlockClass();

    virtual Node * instantiate();

    bool isEmpty();

    void setEmpty();

    virtual void updateLineage(NNO pno);

    virtual bool findNodeNo(NNO n, Node *& foundNode);

    virtual void checkAbstractInstanceErrors();

    virtual void setNodeLocation(Locator loc);

    virtual void print(File * fp);

    virtual void printPostfix(File * fp);

    void printPostfixDataMembersParseTree(File * fp); //helper for recursion NodeVarDecDM

    void printPostfixDataMembersSymbols(File * fp, s32 slot, u32 startpos, ULAMCLASSTYPE classtype);

    virtual const char * getName();

    virtual const std::string prettyNodeName();

    UTI getNodeType();

    virtual bool isAClassBlock();

    NodeBlockClass * getSuperBlockPointer();

    void setSuperBlockPointer(NodeBlockClass *);

    bool isSuperClassLinkReady();

    virtual UTI checkAndLabelType();

    bool checkParameterNodeTypes();

    void addParameterNode(Node * nodeArg);

    Node * getParameterNode(u32 n) const;

    virtual void countNavHzyNoutiNodes(u32& ncnt, u32& hcnt, u32& nocnt);

    bool hasCustomArray();

    void checkCustomArrayTypeFunctions();

    UTI getCustomArrayTypeFromGetFunction();

    u32 getCustomArrayIndexTypeFromGetFunction(Node * rnode, UTI& idxuti, bool& hasHazyArgs);

    bool buildDefaultQuarkValue(u32& dqref); //starts here, called by SymbolClass

    void checkDuplicateFunctions();

    void calcMaxDepthOfFunctions();

    void calcMaxIndexOfVirtualFunctions();

    virtual EvalStatus eval();

    //checks both function and variable symbol names
    virtual bool isIdInScope(u32 id, Symbol * & symptrref);

    virtual u32 getNumberOfSymbolsInTable();

    u32 getNumberOfPotentialClassArgumentSymbols();

    virtual u32 getSizeOfSymbolsInTable();

    virtual s32 getBitSizesOfVariableSymbolsInTable();

    virtual s32 getMaxBitSizeOfVariableSymbolsInTable();

     s32 findUlamTypeInTable(UTI utype, UTI& insidecuti);

    bool isFuncIdInScope(u32 id, Symbol * & symptrref);

    void addFuncIdToScope(u32 id, Symbol * symptr);

    u32 getNumberOfFuncSymbolsInTable();

    u32 getSizeOfFuncSymbolsInTable();

    void updatePrevBlockPtrOfFuncSymbolsInTable();

    void packBitsForVariableDataMembers();

    virtual void printUnresolvedVariableDataMembers();

    virtual void printUnresolvedLocalVariables(u32 fid);

    s32 getVirtualMethodMaxIdx();

    void setVirtualMethodMaxIdx(s32 maxidx);

    virtual u32 countNativeFuncDecls();

    void generateCodeForFunctions(File * fp, bool declOnly, ULAMCLASSTYPE classtype);

    virtual void genCode(File * fp, UlamValue& uvpass);

    virtual void genCodeExtern(File * fp, bool declOnly);

    void genCodeBody(File * fp, UlamValue& uvpass);  //specific for this class

    void initElementDefaultsForEval(UlamValue& uv);

    NodeBlockFunctionDefinition * findTestFunctionNode();

    NodeBlockFunctionDefinition * findToIntFunctionNode();

    virtual void addClassMemberDescriptionsToInfoMap(ClassMemberMap& classmembers);

  protected:
    SymbolTable m_functionST;
    s32 m_virtualmethodMaxIdx;

  private:

    NodeBlockClass * m_superBlockNode;

    bool m_isEmpty; //replaces separate node
    UTI m_templateClassParentUTI;
    NodeList * m_nodeParameterList; //constants

    void genCodeHeaderQuark(File * fp);
    void genCodeHeaderElement(File * fp);

    void genShortNameParameterTypesExtractedForHeaderFile(File * fp);

    void generateCodeForBuiltInClassFunctions(File * fp, bool declOnly, ULAMCLASSTYPE classtype);

    void genCodeBuiltInFunctionHas(File * fp, bool declOnly, ULAMCLASSTYPE classtype);
    void genCodeBuiltInFunctionHasDataMembers(File * fp);
    void genCodeBuiltInFunctionIsMethodQuarkRelated(File * fp, bool declOnly, ULAMCLASSTYPE classtype);
    void genCodeBuiltInFunctionIsRelatedQuarkType(File * fp);

    void genCodeBuiltInFunctionBuildDefaultAtom(File * fp, bool declOnly, ULAMCLASSTYPE classtype);
    void genCodeBuiltInFunctionBuildingDefaultDataMembers(File * fp);
    void genCodeBuiltInFunctionBuildDefaultQuark(File * fp, bool declOnly, ULAMCLASSTYPE classtype);

    void genCodeBuiltInVirtualTable(File * fp, bool declOnly, ULAMCLASSTYPE classtype);

    void generateInternalIsMethodForElement(File * fp, bool declOnly);
    void generateInternalTypeAccessorsForElement(File * fp, bool declOnly);
    void generateGetPosForQuark(File * fp, bool declOnly);

    void generateUlamClassInfoFunction(File * fp, bool declOnly, u32& dmcount);
    virtual void generateUlamClassInfo(File * fp, bool declOnly, u32& dmcount);
    void generateUlamClassInfoCount(File * fp, bool declOnly, u32 dmcount);
    void generateUlamClassGetMangledName(File * fp, bool declOnly);

    std::string removePunct(std::string str);
  };

}

#endif //end NODEBLOCKCLASS_H
