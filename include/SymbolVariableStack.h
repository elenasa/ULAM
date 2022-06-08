/**                                        -*- mode:C++ -*-
 * SymbolVariableStack.h -  Stack Variable Symbol handling for ULAM
 *
 * Copyright (C) 2014-2017 The Regents of the University of New Mexico.
 * Copyright (C) 2014-2017 Ackleyshack LLC.
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
  \file SymbolVariableStack.h -  Stack Variable Symbol handling for ULAM
  \author Elena S. Ackley.
  \author David H. Ackley.
  \date (C) 2014-2017 All rights reserved.
  \gpl
*/

#ifndef SYMBOLVARIABLESTACK_H
#define SYMBOLVARIABLESTACK_H

#include "SymbolVariable.h"

namespace MFM{

  class SymbolVariableStack : public SymbolVariable
  {
  public:
    SymbolVariableStack(const Token& id, UTI utype, CompilerState& state);

    SymbolVariableStack(const Token& id, UTI utype, s32 slot, CompilerState& state);

    SymbolVariableStack(const SymbolVariableStack& sref);

    SymbolVariableStack(const SymbolVariableStack& sref, bool keepType);

    virtual ~SymbolVariableStack();

    virtual Symbol * clone();

    virtual Symbol * cloneKeepsType();

    virtual s32 getStackFrameSlotIndex();

    virtual void setStackFrameSlotIndex(s32 slot);

    virtual s32 getBaseArrayIndex();

    virtual const std::string getMangledPrefix();

    virtual void printPostfixValuesOfVariableDeclarations(File * fp, s32 slot, u32 startpos, ULAMCLASSTYPE classtype);

    UlamValue getAutoPtrForEval();

    void setAutoPtrForEval(UlamValue ptr);

    UTI getAutoStorageTypeForEval();

    void setAutoStorageTypeForEval(UTI uti);

    virtual bool isFunctionParameter();

    void setFunctionParameter();

    bool isConstantFunctionParameter();

    void setConstantFunctionParameter();

  protected:

  private:
    s32 m_stackFrameSlotIndex;
    UlamValue m_autoPtrForEval;
    UTI m_autoStgTypeForEval;

    bool m_isFunctionParameter;
    bool m_isConstantFunctionParameter;

  };

}

#endif //end SYMBOLVARIABLESTACK_H
