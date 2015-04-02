/**                                        -*- mode:C++ -*-
 * SymbolTable.h -  Basic handling of Table of Symbols for ULAM
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
  \file SymbolTable.h -  Basic handling of Table of Symbols for ULAM
  \author Elenas S. Ackley.
  \author David H. Ackley.
  \date (C) 2014-2015 All rights reserved.
  \gpl
*/


#ifndef SYMBOLTABLE_H
#define SYMBOLTABLE_H

#include <map>
#include "Symbol.h"
#include "itype.h"
#include "File.h"
#include "FileManager.h"
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
    bool mergeTables(NodeBlock * toTable);

    Symbol * getSymbolPtr(u32 id);

    u32 getTableSize();


    //Table of Variable Data Members:

    u32 getTotalSymbolSize();

    s32 getTotalVariableSymbolsBitSize();

    s32 getMaxVariableSymbolsBitSize();  //for quark union

    //void packBitsForTableOfVariableDataMembers();  //after type labeling, before code gen

    s32 findPosOfUlamTypeInTable(UTI utype);

    void genCodeForTableOfVariableDataMembers(File * fp, ULAMCLASSTYPE classtype);  //(unused)

    void genCodeBuiltInFunctionsOverTableOfVariableDataMember(File * fp, bool declOnly, ULAMCLASSTYPE classtype);

    void printPostfixValuesForTableOfVariableDataMembers(File * fp, s32 slot, u32 startpos, ULAMCLASSTYPE classtype);


    //Table Of Functions:

    bool checkTableOfFunctions();

    void linkToParentNodesAcrossTableOfFunctions(NodeBlockClass * p);

    bool findNodeNoAcrossTableOfFunctions(NNO n, Node*& foundNode);

    void labelTableOfFunctions();

    u32 countNavNodesAcrossTableOfFunctions();

    bool checkCustomArrayTypeFuncs();

    u32 countNativeFuncDeclsForTableOfFunctions();

    void calcMaxDepthForTableOfFunctions();

    void genCodeForTableOfFunctions(File * fp, bool declOnly, ULAMCLASSTYPE classtype);


    // TableOfClasses:
    void getTargets(TargetMap& classtargets);

    void testForTableOfClasses(File * fp);

    void printPostfixForTableOfClasses(File * fp);

    void printForDebugForTableOfClasses(File * fp);

    bool statusUnknownConstantExpressionsInTableOfClasses();

    bool statusNonreadyClassArgumentsInTableOfClasses();

    bool fullyInstantiateTableOfClasses();

    void updateLineageForTableOfClasses(); //done incrementally for stubs

    void checkCustomArraysForTableOfClasses();

    void checkDuplicateFunctionsForTableOfClasses();

    void calcMaxDepthOfFunctionsForTableOfClasses();

    bool labelTableOfClasses();

    u32 countNavNodesAcrossTableOfClasses();

    bool setBitSizeOfTableOfClasses();

    void printBitSizeOfTableOfClasses();

    void packBitsForTableOfClasses();

    void generateIncludesForTableOfClasses(File * fp);

    void generateForwardDefsForTableOfClasses(File * fp);

    void generateTestInstancesForTableOfClasses(File * fp);

    void genCodeForTableOfClasses(FileManager * fm);

  protected:
    std::map<u32, Symbol* > m_idToSymbolPtr;

  private:
    CompilerState & m_state;
    s32 calcVariableSymbolTypeSize(UTI ut);
    bool variableSymbolWithCountableSize(Symbol * sym);
  };

}

#endif //end SYMBOL_H
