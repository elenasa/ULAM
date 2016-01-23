/**                                        -*- mode:C++ -*-
 * SymbolTable.h -  Basic handling of Table of Symbols for ULAM
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
  \file SymbolTable.h -  Basic handling of Table of Symbols for ULAM
  \author Elenas S. Ackley.
  \author David H. Ackley.
  \date (C) 2014-2016 All rights reserved.
  \gpl
*/


#ifndef SYMBOLTABLE_H
#define SYMBOLTABLE_H

#include <map>
#include <vector>
#include "Symbol.h"
#include "itype.h"
#include "File.h"
#include "FileManager.h"
#include "MapClassMemberDesc.h"
#include "TargetMap.h"
#include "UlamTypeClass.h"

namespace MFM{

  struct CompilerState; //forward
  class Node; //forward
  class NodeBlockClass; //forward
  class NodeBlock; //forward

  class SymbolTable
  {
  public:

    SymbolTable(CompilerState& state);
    SymbolTable(const SymbolTable& ref);
    ~SymbolTable();

    void clearTheTable();
    bool isInTable(u32 id, Symbol * & symptrref);
    void addToTable(u32 id, Symbol * s);
    void replaceInTable(u32 oldid, u32 newid, Symbol * s);
    void replaceInTable(Symbol * oldsym, Symbol * newsym);
    bool removeFromTable(u32 id, Symbol *& rtnsymptr);

    Symbol * getSymbolPtr(u32 id);

    u32 getTableSize();

    u32 getNumberOfConstantSymbolsInTable(bool argsOnly);

    //Table of Variable Data Members:

    u32 getTotalSymbolSize();

    s32 getTotalVariableSymbolsBitSize();

    s32 getMaxVariableSymbolsBitSize();  //for quark union

    //void packBitsForTableOfVariableDataMembers();  //after type labeling, before code gen

    s32 findPosOfUlamTypeInTable(UTI utype, UTI& insidecuti);

    void genCodeForTableOfVariableDataMembers(File * fp, ULAMCLASSTYPE classtype);  //(unused)

    void genModelParameterImmediateDefinitionsForTableOfVariableDataMembers(File *fp);

    void genCodeBuiltInFunctionHasOverTableOfVariableDataMember(File * fp);

    //void genCodeBuiltInFunctionHasPosOverTableOfVariableDataMember(File * fp); (unused)

    void genCodeBuiltInFunctionBuildDefaultsOverTableOfVariableDataMember(File * fp, UTI cuti);

    void addClassMemberDescriptionsToMap(UTI classType, ClassMemberMap& classmembers);

    void addClassMemberFunctionDescriptionsToMap(UTI classType, ClassMemberMap& classmembers);

    void printPostfixValuesForTableOfVariableDataMembers(File * fp, s32 slot, u32 startpos, ULAMCLASSTYPE classtype);


    //Table Of Functions:

    bool checkTableOfFunctions();

    void linkToParentNodesAcrossTableOfFunctions(NodeBlockClass * p);

    void updatePrevBlockPtrAcrossTableOfFunctions(NodeBlockClass * p);

    bool findNodeNoAcrossTableOfFunctions(NNO n, Node*& foundNode);

    void labelTableOfFunctions();

    void countNavNodesAcrossTableOfFunctions(u32& ncnt, u32& hcnt, u32& nocnt);

    bool checkCustomArrayTypeFuncs();

    UTI getCustomArrayReturnTypeGetFunction();

    u32 getCustomArrayIndexTypeGetFunction(Node * rnode, UTI& idxuti, bool& hasHazyArgs);

    u32 countNativeFuncDeclsForTableOfFunctions();

    void calcMaxDepthForTableOfFunctions();

    void calcMaxIndexForVirtualTableOfFunctions(s32& maxidx);

    void checkAbstractInstanceErrorsAcrossTableOfFunctions();

    void genCodeForTableOfFunctions(File * fp, bool declOnly, ULAMCLASSTYPE classtype);


    // TableOfClasses:
    void getTargets(TargetMap& classtargets);

    void getClassMembers(ClassMemberMap& classmembers);

    void initializeElementDefaultsForEval(UlamValue& uvsite);

    void testForTableOfClasses(File * fp);

    void printPostfixForTableOfClasses(File * fp);

    void buildDefaultQuarksFromTableOfClasses();

    void printForDebugForTableOfClasses(File * fp);

    bool statusNonreadyClassArgumentsInTableOfClasses();

    bool fullyInstantiateTableOfClasses();

    void updateLineageForTableOfClasses(); //done incrementally for stubs

    void checkCustomArraysForTableOfClasses();

    void checkDuplicateFunctionsForTableOfClasses();

    void calcMaxDepthOfFunctionsForTableOfClasses();

    bool calcMaxIndexOfVirtualFunctionsForTableOfClasses();

    void checkAbstractInstanceErrorsForTableOfClasses();

    bool labelTableOfClasses();

    void countNavNodesAcrossTableOfClasses(u32& navcount, u32& hzycount, u32& unsetcount);

    bool setBitSizeOfTableOfClasses();

    void printBitSizeOfTableOfClasses();

    void packBitsForTableOfClasses();

    void generateIncludesForTableOfClasses(File * fp);

    void generateForwardDefsForTableOfClasses(File * fp);

    void generateTestInstancesForTableOfClasses(File * fp);

    void genCodeForTableOfClasses(FileManager * fm);

    UTI findClassNodeNoForTableOfClasses(NNO n);

  protected:
    std::map<u32, Symbol* > m_idToSymbolPtr;

  private:
    CompilerState & m_state;
    s32 calcVariableSymbolTypeSize(UTI ut);
    bool variableSymbolWithCountableSize(Symbol * sym);

    bool isAnonymousClass(UTI cuti);
  };

}

#endif //end SYMBOL_H
