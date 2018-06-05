/**                                        -*- mode:C++ -*-
 * SymbolTableOfClasses.h -  Handling of Table of Class Symbols for ULAM
 *
 * Copyright (C) 2014-2018 The Regents of the University of New Mexico.
 * Copyright (C) 2014-2018 Ackleyshack LLC.
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
  \file SymbolTableOfClasses.h - Handling of Table of Class Symbols for ULAM
  \author Elenas S. Ackley.
  \author David H. Ackley.
  \date (C) 2014-2018 All rights reserved.
  \gpl
*/


#ifndef SYMBOLTABLEOFCLASSES_H
#define SYMBOLTABLEOFCLASSES_H

#include "SymbolTable.h"
#include "MapClassMemberDesc.h"
#include "TargetMap.h"

namespace MFM{

  class SymbolTableOfClasses : public SymbolTable
  {
  public:

    SymbolTableOfClasses(CompilerState& state);
    SymbolTableOfClasses(const SymbolTableOfClasses& ref);
    virtual ~SymbolTableOfClasses();

    // TableOfClasses:
    void getTargets(TargetMap& classtargets);

    void getClassMembers(ClassMemberMap& classmembers);

    void testForTableOfClasses(File * fp);

    void printPostfixForTableOfClasses(File * fp);

    void buildDefaultValuesFromTableOfClasses();

    void printForDebugForTableOfClasses(File * fp);

    void printClassListForDebugForTableOfClasses();

    bool checkForUnknownTypeNamesInTableOfClasses(); //at end of parsing

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

    u32 reportUnknownTypeNamesAcrossTableOfClasses();

    u32 reportTooLongClassNamesAcrossTableOfClasses();

    bool setBitSizeOfTableOfClasses();

    void printBitSizeOfTableOfClasses();

    void packBitsForTableOfClasses();

    void printUnresolvedVariablesForTableOfClasses();

    void generateIncludesForTableOfClasses(File * fp);

    void generateAllIncludesTestMainForTableOfClasses(File * fp);

    void generateForwardDefsForTableOfClasses(File * fp);

    void generateTestInstancesForTableOfClasses(File * fp);

    u32 defineRegistrationNumberForTableOfClasses(); //ulam-4

    void genCodeForTableOfClasses(FileManager * fm);

    UTI findClassNodeNoForTableOfClasses(NNO n);

  protected:

  private:

  };

}

#endif //end SYMBOLTABLEOFCLASSES_H
