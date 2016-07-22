/**                                        -*- mode:C++ -*-
 * SymbolTableOfVariables.h - Handling of Table of Variable, Typedef and Constant Symbols for ULAM
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
  \file SymbolTableOfVariables.h - Handling of Table of Variable, Typedef and Constant Symbols for ULAM
  \author Elenas S. Ackley.
  \author David H. Ackley.
  \date (C) 2014-2016 All rights reserved.
  \gpl
*/


#ifndef SYMBOLTABLEOFVARIABLES_H
#define SYMBOLTABLEOFVARIABLES_H

#include "SymbolTable.h"
#include "MapClassMemberDesc.h"
#include "UlamValue.h"

namespace MFM{

  class SymbolTableOfVariables : public SymbolTable
  {
  public:

    SymbolTableOfVariables(CompilerState& state);
    SymbolTableOfVariables(const SymbolTableOfVariables& ref);
    virtual ~SymbolTableOfVariables();

    u32 getNumberOfConstantSymbolsInTable(bool argsOnly);

    //Table of Variable Data Members:

    virtual u32 getTotalSymbolSize();

    s32 getTotalVariableSymbolsBitSize();

    s32 getMaxVariableSymbolsBitSize();  //for quark union

    void initializeElementDefaultsForEval(UlamValue& uvsite, UTI cuti);

    //void packBitsForTableOfVariableDataMembers();  //after type labeling, before code gen

    s32 findPosOfUlamTypeInTable(UTI utype, UTI& insidecuti);

    void genCodeForTableOfVariableDataMembers(File * fp, ULAMCLASSTYPE classtype);  //(unused)

    void genModelParameterImmediateDefinitionsForTableOfVariableDataMembers(File *fp);

    void genCodeBuiltInFunctionHasOverTableOfVariableDataMember(File * fp);

    //void genCodeBuiltInFunctionHasPosOverTableOfVariableDataMember(File * fp); (unused)

    //void genCodeBuiltInFunctionBuildDefaultsOverTableOfVariableDataMember(File * fp, UTI cuti);

    void addClassMemberDescriptionsToMap(UTI classType, ClassMemberMap& classmembers);

    void printPostfixValuesForTableOfVariableDataMembers(File * fp, s32 slot, u32 startpos, ULAMCLASSTYPE classtype);


  protected:

  private:
    s32 calcVariableSymbolTypeSize(UTI ut);
    bool variableSymbolWithCountableSize(Symbol * sym);

  };

}

#endif //end SYMBOLTABLEOFVARIABLES_H
