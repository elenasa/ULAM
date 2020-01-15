/**                                        -*- mode:C++ -*-
 * SymbolConstantValue.h - Handling of Named Constant Symbols for ULAM
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
  \file SymbolConstantValue.h - Handling of Named Constant Symbols for ULAM
  \author Elena S. Ackley.
  \author David H. Ackley.
  \date (C) 2015-2017 All rights reserved.
  \gpl
*/

#ifndef SYMBOLCONSTANTVALUE_H
#define SYMBOLCONSTANTVALUE_H

#include "SymbolWithValue.h"

namespace MFM{

  class CompilerState;  //forward

  //distinguish between Symbols
  class SymbolConstantValue : public SymbolWithValue
  {
  public:
    SymbolConstantValue(const Token& id, UTI utype, CompilerState& state);
    SymbolConstantValue(const SymbolConstantValue& sref);
    SymbolConstantValue(const SymbolConstantValue& sref, bool keepType);
    virtual ~SymbolConstantValue();

    virtual Symbol * clone();
    virtual Symbol * cloneKeepsType();

    virtual bool isConstant();

    virtual const std::string getMangledPrefix();

    virtual const std::string getMangledName();

    const std::string getCompleteConstantMangledName();

    virtual void printPostfixValuesOfVariableDeclarations(File * fp, s32 slot, u32 startpos, ULAMCLASSTYPE classtype);

    virtual void setStructuredComment();

    void setConstantStackFrameAbsoluteSlotIndex(u32 slot);
    u32 getConstantStackFrameAbsoluteSlotIndex();

  protected:

  private:
    u32 m_constantStackFrameAbsoluteSlotIndex;

  };
} //MFM

#endif //end SYMBOLCONSTANTVALUE_H
