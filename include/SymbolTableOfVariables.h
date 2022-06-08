/**                                        -*- mode:C++ -*-
 * SymbolTableOfVariables.h - Handling of Table of Variable, Typedef and Constant Symbols for ULAM
 *
 * Copyright (C) 2014-2021 The Regents of the University of New Mexico.
 * Copyright (C) 2014-2021 Ackleyshack LLC.
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
  \author Elena S. Ackley.
  \author David H. Ackley.
  \date (C) 2014-2021 All rights reserved.
  \gpl
*/
#ifndef SYMBOLTABLEOFVARIABLES_H
#define SYMBOLTABLEOFVARIABLES_H

#include "SymbolTable.h"
#include "UlamValue.h"
#include <set>

namespace MFM{

  class SymbolTableOfVariables : public SymbolTable
  {
  public:

    SymbolTableOfVariables(CompilerState& state);
    SymbolTableOfVariables(const SymbolTableOfVariables& ref);
    virtual ~SymbolTableOfVariables();

    u32 getNumberOfConstantSymbolsInTable(bool argsOnly);

    bool hasUlamTypeSymbolsInTable(ULAMTYPE etyparg);

    bool hasADataMemberStringInitValueInClass(UTI cuti);

    u32 fixAllStringSymbolsInTable(); //returns number of strings fixed

    u32 findTypedefSymbolNameIdByTypeInTable(UTI type);

    u32 getAllRemainingCulamGeneratedTypedefSymbolsInTable(std::map<u32, Symbol*>& mapref);

    //Table of Variable Data Members:

    virtual u32 getTotalSymbolSize();

    s32 getTotalVariableSymbolsBitSize(std::set<UTI>& seensetref);

    s32 getMaxVariableSymbolsBitSize(std::set<UTI>& seensetref);  //for quark union

    //void packBitsForTableOfVariableDataMembers();  //after type labeling, before code gen

    void genModelParameterImmediateDefinitionsForTableOfVariableDataMembers(File *fp);

    void printPostfixValuesForTableOfVariableDataMembers(File * fp, s32 slot, u32 startpos, ULAMCLASSTYPE classtype);

  protected:

  private:
    s32 calcVariableSymbolTypeSize(UTI ut, std::set<UTI>& seensetref);
    bool variableSymbolWithCountableSize(Symbol * sym);

  };

}

#endif //end SYMBOLTABLEOFVARIABLES_H
