/**                                        -*- mode:C++ -*-
 * SymbolVariableStack.h -  Stack Variable Symbol handling for ULAM
 *
 * Copyright (C) 2014 The Regents of the University of New Mexico.
 * Copyright (C) 2014 Ackleyshack LLC.
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
  \author Elenas S. Ackley.
  \author David H. Ackley.
  \date (C) 2014 All rights reserved.
  \gpl
*/

#ifndef SYMBOLVARIABLESTACK_H
#define SYMBOLVARIABLESTACK_H

#include "UlamValue.h"
#include "SymbolVariable.h"

namespace MFM{

  class SymbolVariableStack : public SymbolVariable
  {
  public:
    SymbolVariableStack(Token id, UTI utype, PACKFIT packed, s32 slot, CompilerState& state);

    SymbolVariableStack(const SymbolVariableStack& sref);

    virtual ~SymbolVariableStack();

    virtual Symbol * clone();

    virtual s32 getStackFrameSlotIndex();

    virtual void setStackFrameSlotIndex(s32 slot);

    virtual s32 getBaseArrayIndex();

    virtual const std::string getMangledPrefix();

    virtual void generateCodedVariableDeclarations(File * fp, ULAMCLASSTYPE classtype);

  protected:

  private:
    s32 m_stackFrameSlotIndex;
  };

}

#endif //end SYMBOLVARIABLESTACK_H
