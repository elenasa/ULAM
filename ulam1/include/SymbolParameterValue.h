/**                                        -*- mode:C++ -*-
 * SymbolParameterValue.h - Handling Model Parameter Symbols for ULAM
 *
 * Copyright (C) 2015-2016 The Regents of the University of New Mexico.
 * Copyright (C) 2015-2016 Ackleyshack LLC.
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
  \file SymbolParameterValue.h - Handling Model Parameter Symbols for ULAM
  \author Elenas S. Ackley.
  \author David H. Ackley.
  \date (C) 2015-2016 All rights reserved.
  \gpl
*/

#ifndef SYMBOLPARAMETERVALUE_H
#define SYMBOLPARAMETERVALUE_H

#include "SymbolWithValue.h"

namespace MFM{

  class CompilerState;  //forward

  //distinguish between Symbols
  class SymbolParameterValue : public SymbolWithValue
  {
  public:
    SymbolParameterValue(Token id, UTI utype, CompilerState& state);
    SymbolParameterValue(const SymbolParameterValue& sref);
    SymbolParameterValue(const SymbolParameterValue& sref, bool keepType);
    virtual ~SymbolParameterValue();

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

#endif //SYMBOLPARAMETERVALUE_H
