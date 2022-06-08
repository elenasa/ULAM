/**                                        -*- mode:C++ -*-
 * NodeBlockClass.h - Basic Node for handling Classes for ULAM
 *
 * Copyright (C) 2014-2019 The Regents of the University of New Mexico.
 * Copyright (C) 2014-2021 Ackleyshack LLC.
 * Copyright (C) 2020-2021 The Living Computation Foundation.
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
  \author Elena S. Ackley.
  \author David H. Ackley.
  \date (C) 2014-2021 All rights reserved.
  \gpl
*/


#ifndef NODEBLOCKCLASS_H
#define NODEBLOCKCLASS_H

#include "NodeBlockContext.h"
#include "NodeBlockFunctionDefinition.h"
#include "NodeList.h"
#include "Symbol.h"
#include "SymbolTableOfFunctions.h"

namespace MFM{

  class NodeBlockClass : public NodeBlockContext
  {
  public:

    NodeBlockClass(NodeBlock * prevBlockNode, CompilerState & state);

    NodeBlockClass(const NodeBlockClass& ref);

    virtual ~NodeBlockClass();

    virtual Node * instantiate();

    bool isEmpty();

    void setEmpty();

    virtual void updateLineage(NNO pno);

    virtual bool findNodeNo(NNO n, Node *& foundNode);

    virtual void checkAbstractInstanceErrors();

    virtual void setNodeLocation(Locator loc);

    virtual void resetNodeLocations(Locator loc);

    virtual void print(File * fp);

    virtual void printPostfix(File * fp);

    void printPostfixDataMembersParseTree(File * fp, UTI cuti); //helper for recursion NodeVarDecDM

    void printPostfixDataMembersSymbols(File * fp, s32 slot, u32 startpos, UTI cuti);

    void noteBaseClassTypeAndName(UTI nuti, u32 baseitem, bool sharedbase, s32 totalsize, u32& accumsize);

    void noteDataMembersParseTree(UTI cuti, s32 totalsize);

    virtual const char * getName();

    virtual u32 getNameId();

    virtual const std::string prettyNodeName();

    UTI getNodeType(); //not virtual!!

    virtual bool isAClassBlock();

    void setDataMembersParseTree(UTI cuti, NodeBlockClass & fromClassBlock);
    void resetDataMembersParseTree(UTI cuti, NodeBlockClass & fromClassBlock);
    void setDataMembersSymbolTable(UTI cuti, NodeBlockClass & fromClassBlock);

    SymbolTable * getFunctionSymbolTablePtr();

    void setMemberFunctionsSymbolTable(UTI cuti, NodeBlockClass& fromClassBlock);

    bool isBaseClassBlockReady(UTI cuti, UTI baseuti);

    virtual bool hasStringDataMembers();

    virtual UTI checkAndLabelType(Node * thisparentnode);

    bool checkParameterNodeTypes();

    void addParameterNode(Node * nodeArg);

    Node * getParameterNode(u32 n);

    u32 getNumberOfParameterNodes();

    bool checkArgumentNodeTypes();

    void addArgumentNode(Node * nodeArg);

    Node * getArgumentNode(u32 n);

    u32 getNumberOfArgumentNodes();

    NodeList * getListOfArgumentNodes();

    virtual void countNavHzyNoutiNodes(u32& ncnt, u32& hcnt, u32& nocnt);

    u32 getLocalsFilescopeType();

    bool hasCustomArray();

    void checkCustomArrayTypeFunctions(UTI cuti);

    UTI getCustomArrayTypeFromGetFunction(UTI cuti);

    u32 getCustomArrayIndexTypeFromGetFunction(UTI cuti, Node * rnode, UTI& idxuti, bool& hasHazyArgs);

    bool hasCustomArrayLengthofFunction(UTI cuti);

    virtual bool buildDefaultValue(u32 wlen, BV8K& dvref); //starts here, called by SymbolClass

    virtual bool buildDefaultValueForClassConstantDefs();

    void checkDuplicateFunctionsInClassAndAncestors();

    u32 checkDuplicateFunctions(FSTable& mangledFunctionMap, u32& probcount);

    void checkMatchingFunctions();

    void calcMaxDepthOfFunctions();

    void calcMaxIndexOfVirtualFunctions();

    virtual EvalStatus eval();

    //checks both function and variable symbol names
    virtual bool isIdInScope(u32 id, Symbol * & symptrref);

    virtual u32 getNumberOfSymbolsInTable();

    u32 getNumberOfPotentialClassArgumentSymbols();

    virtual u32 getSizeOfSymbolsInTable();

    virtual s32 getBitSizesOfVariableSymbolsInTable(s32& basebits, s32& mybits, std::set<UTI>& seensetref);

    virtual s32 getMaxBitSizeOfVariableSymbolsInTable(s32& basebits, s32& mybits, std::set<UTI>& seensetref);

    virtual bool isFuncIdInScope(u32 id, Symbol * & symptrref);

    void addFuncIdToScope(u32 id, Symbol * symptr);

    u32 getNumberOfFuncSymbolsInTableHere();

    u32 getNumberOfFuncSymbolsInTable();

    u32 getSizeOfFuncSymbolsInTable();

    void updatePrevBlockPtrOfFuncSymbolsInTable();

    TBOOL packBitsForVariableDataMembers();

    virtual void printUnresolvedVariableDataMembers();

    virtual void printUnresolvedLocalVariables(u32 fid);

    s32 getVirtualMethodMaxIdx();

    void setVirtualMethodMaxIdx(s32 maxidx);

    virtual u32 countNativeFuncDecls();

    void generateCodeForFunctions(File * fp, bool declOnly, ULAMCLASSTYPE classtype);

    virtual void genCode(File * fp, UVPass& uvpass);

    virtual void genCodeExtern(File * fp, bool declOnly);

    void genCodeBody(File * fp, UVPass& uvpass);  //specific for this class

    virtual void genCodeConstantArrayInitialization(File * fp);

    virtual void generateBuiltinConstantClassOrArrayInitializationFunction(File * fp, bool declOnly);

    NodeBlockFunctionDefinition * findTestFunctionNode();

    NodeBlockFunctionDefinition * findCustomArrayLengthofFunctionNode();

    NodeBlockFunctionDefinition * findToIntFunctionNode();

    virtual void addTargetDescriptionToInfoMap(TargetMap& classtargets, u32 scid);
    virtual void addMemberDescriptionsToInfoMap(ClassMemberMap& classmembers);

    virtual void generateTestInstance(File * fp, bool runtest);
    virtual void generateIncludeTestMain(File * fp);

  protected:
    SymbolTableOfFunctions m_functionST;
    s32 m_virtualmethodMaxIdx;

  private:
    bool m_buildingDefaultValueInProgress;
    bool m_bitPackingInProgress;
    bool m_isEmpty; //replaces separate node
    bool m_registeredForTestInstance;

    UTI m_templateClassParentUTI;
    NodeList * m_nodeParameterList; //constants
    NodeList * m_nodeArgumentList;  //template instance

    UTI checkMultipleInheritances();
    void checkTestFunctionReturnType();
    void checkCustomArrayLengthofFunctionReturnType();

    void genCodeHeaderQuark(File * fp);
    void genCodeHeaderElement(File * fp);
    void genCodeHeaderTransient(File * fp);
    void genCodeHeaderLocalsFilescope(File * fp);

    void genCodeDataMemberChartAsComment(File * fp, UTI cuti); //at end of header
    void genBaseClassTypeAndNameEntryAsComment(File * fp, UTI nuti, s32 atpos, u32& accumsize, u32 baseitem, bool dupflag); //for base classes

    void genThisUlamBaseClassAsAHeaderComment(File * fp);

    void genShortNameParameterTypesExtractedForHeaderFile(File * fp);

    void genCodeBodyElement(File * fp, UVPass& uvpass);  //specific for this class
    void genCodeBodyQuark(File * fp, UVPass& uvpass);  //specific for this class
    void genCodeBodyTransient(File * fp, UVPass& uvpass);  //specific for this class
    void genCodeBodyLocalsFilescope(File * fp, UVPass& uvpass);  //specific for this class

    void generateCodeForBuiltInClassFunctions(File * fp, bool declOnly, ULAMCLASSTYPE classtype);
    void genCodeBuiltInFunctionIsMethodRelatedInstance(File * fp, bool declOnly, ULAMCLASSTYPE classtype);
    void genCodeBuiltInIsMethodByRegistrationNumber(File * fp, bool declOnly, ULAMCLASSTYPE classtype);
    void generateInternalIsMethodForElement(File * fp, bool declOnly);

    void genCodeBuiltInFunctionGetRelPosMethodRelatedInstance(File * fp, bool declOnly, ULAMCLASSTYPE classtype);
    void genCodeBuiltInFunctionGetRelPosMethodRelatedInstanceByRegistrationNumber(File * fp, bool declOnly, ULAMCLASSTYPE classtype);
    void genCodeBuiltInFunctionGetRelPosRelatedInstanceByRegistrationNumber(File * fp);

    void genCodeBuiltInFunctionNumberOfBases(File * fp, bool declOnly, ULAMCLASSTYPE classtype);
    void genCodeBuiltInFunctionNumberOfDirectBases(File * fp, bool declOnly, ULAMCLASSTYPE classtype);

    void genCodeBuiltInFunctionGetOrderedBaseClass(File * fp, bool declOnly, ULAMCLASSTYPE classtype);
    void generateBuiltInFunctionGetOrderedBaseClassEntry(File * fp); //helper


    void genCodeBuiltInFunctionIsDirectBaseClass(File * fp, bool declOnly, ULAMCLASSTYPE classtype);
    void genCodeBuiltInFunctionIsDirectBaseClassByRegistrationNumber(File * fp); //helper


    void genCodeBuiltInFunctionGetClassLength(File * fp, bool declOnly, ULAMCLASSTYPE classtype);
    void genCodeBuiltInFunctionGetClassRegistrationNumber(File * fp, bool declOnly, ULAMCLASSTYPE classtype);
    void genCodeBuiltInFunctionGetElementType(File * fp, bool declOnly, ULAMCLASSTYPE classtype);
    void genCodeBuiltInFunctionBuildDefaultAtom(File * fp, bool declOnly, ULAMCLASSTYPE classtype);
    bool genCodeBuiltInFunctionBuildingDefaultDataMembers(File * fp);
    void genCodeBuiltInFunctionBuildDefaultQuark(File * fp, bool declOnly, ULAMCLASSTYPE classtype);
    void genCodeBuiltInFunctionBuildDefaultTransient(File * fp, bool declOnly, ULAMCLASSTYPE classtype);

    void genCodeBuiltInVirtualTable(File * fp, bool declOnly, ULAMCLASSTYPE classtype);
    void genCodeBuiltInVirtualTableStartOffsetHelper(File * fp, bool declOnly, ULAMCLASSTYPE classtype);

    void generateInternalTypeAccessorsForElement(File * fp, bool declOnly);
    void generateGetPosForQuark(File * fp, bool declOnly);

    void generateUlamClassInfoFunction(File * fp, bool declOnly, u32& dmcount);
    virtual void generateUlamClassInfo(File * fp, bool declOnly, u32& dmcount);
    void generateUlamClassInfoCount(File * fp, bool declOnly, u32 dmcount);
    void generateUlamClassGetMangledName(File * fp, bool declOnly);
    void generateUlamClassGetMangledNameAsStringIndex(File * fp, bool declOnly);
    void generateUlamClassGetNameAsStringIndex(File * fp, bool declOnly);

    std::string removePunct(std::string str);

    void generateTestInstanceRun(File * fp);
  };

}

#endif //end NODEBLOCKCLASS_H
