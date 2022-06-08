/**                                        -*- mode:C++ -*-
 * SymbolModelParameterValue.h - Handling Model Parameter Symbols for ULAM
 *
 * Copyright (C) 2015-2017 The Regents of the University of New Mexico.
 * Copyright (C) 2015-2017 Ackleyshack LLC.
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
  \file SymbolModelParameterValue.h - Handling Model Parameter Symbols for ULAM
  \author Elena S. Ackley.
  \author David H. Ackley.
  \date (C) 2015-2017 All rights reserved.
  \gpl
*/

#ifndef SYMBOLMODELPARAMETERVALUE_H
#define SYMBOLMODELPARAMETERVALUE_H

#include "SymbolWithValue.h"

namespace MFM{

  class CompilerState;  //forward

  //distinguish between Symbols
  class SymbolModelParameterValue : public SymbolWithValue
  {
  public:
    SymbolModelParameterValue(const Token& id, UTI utype, CompilerState& state);
    SymbolModelParameterValue(const SymbolModelParameterValue& sref);
    SymbolModelParameterValue(const SymbolModelParameterValue& sref, bool keepType);
    virtual ~SymbolModelParameterValue();

    virtual Symbol * clone();
    virtual Symbol * cloneKeepsType();

    virtual bool isConstant();

    virtual bool isModelParameter();

    virtual const std::string getMangledPrefix();

    virtual void printPostfixValuesOfVariableDeclarations(File * fp, s32 slot, u32 startpos, ULAMCLASSTYPE classtype);

    virtual void setStructuredComment();

  protected:

  private:
  };
} //MFM

#endif //SYMBOLMODELPARAMETERVALUE_H
