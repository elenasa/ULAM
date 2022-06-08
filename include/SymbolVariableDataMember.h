/**                                        -*- mode:C++ -*-
 * SymbolVariableDataMember.h -  Data Member Variable Symbol handling for ULAM
 *
 * Copyright (C) 2014-2019 The Regents of the University of New Mexico.
 * Copyright (C) 2014-2019 Ackleyshack LLC.
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
  \file SymbolVariableDataMember.h - Data Member Variable Symbol handling for ULAM
  \author Elena S. Ackley.
  \author David H. Ackley.
  \date (C) 2014-2019   All rights reserved.
  \gpl
*/


#ifndef SYMBOLVARIABLEDATAMEMBER_H
#define SYMBOLVARIABLEDATAMEMBER_H

#include "SymbolVariable.h"

namespace MFM{

  class SymbolVariableDataMember : public SymbolVariable
  {
  public:
    SymbolVariableDataMember(const Token& id, UTI utype, u32 slot, CompilerState& state);

    SymbolVariableDataMember(const SymbolVariableDataMember& sref);

    SymbolVariableDataMember(const SymbolVariableDataMember& sref, bool keepType);

    virtual ~SymbolVariableDataMember();

    virtual Symbol * clone();

    virtual Symbol * cloneKeepsType();

    virtual u32 getDataMemberUnpackedSlotIndex();

    virtual s32 getBaseArrayIndex();

    virtual const std::string getMangledPrefix();

    virtual const std::string getMangledName();

    virtual u32 getPosOffset();

    virtual bool isPosOffsetReliable();

    void setPosOffset(u32 offsetIntoAtom);

    virtual void printPostfixValuesOfVariableDeclarations(File * fp, s32 slot, u32 startpos, ULAMCLASSTYPE classtype);

    virtual void setStructuredComment();

  protected:

  private:
    u32 m_posOffset; //relative
    u32 m_dataMemberUnpackedSlotIndex;  //untrusted
  };

}

#endif //SYMBOLVARIABLEDATAMEMBER_H
