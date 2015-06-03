/**                                        -*- mode:C++ -*-
 * SymbolVariable.h -  Basic handling of Variable Symbols for ULAM
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
  \file SymbolVariable.h -  Basic handling of Variable Symbols for ULAM
  \author Elenas S. Ackley.
  \author David H. Ackley.
  \date (C) 2014-2015 All rights reserved.
  \gpl
*/

#ifndef SYMBOLVARIABLE_H
#define SYMBOLVARIABLE_H

#include "UlamValue.h"
#include "Symbol.h"

namespace MFM{

  class CompilerState;  //forward

  //distinguishes between SymbolFunction, SymbolTypedef, SymbolConstantValue
  class SymbolVariable : public Symbol
  {
  public:
    SymbolVariable(Token id, UTI utype, PACKFIT packed, CompilerState& state);

    SymbolVariable(const SymbolVariable& sref);

    virtual ~SymbolVariable();

    virtual s32 getStackFrameSlotIndex();

    virtual void setStackFrameSlotIndex(s32 slot);

    virtual u32 getDataMemberSlotIndex();

    virtual s32 getBaseArrayIndex() = 0;

    u32 getPosOffset();

    void setPosOffset(u32 offsetIntoAtom);

    PACKFIT isPacked();
    void setPacked(PACKFIT p);

    virtual void generateCodedVariableDeclarations(File * fp, ULAMCLASSTYPE classtype) = 0;

  protected:
    u32 m_posOffset;
    PACKFIT m_packed;

  private:

  };

}

#endif //end SYMBOLVARIABLE_H
