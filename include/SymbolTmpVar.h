/**                                        -*- mode:C++ -*-
 * SymbolTmpVar.h -  Basic handling of TmpVar Symbols for ULAM Code Gen
 *
 * Copyright (C) 2016-2020 The Regents of the University of New Mexico.
 * Copyright (C) 2016-2020 Ackleyshack LLC.
 * Copyright (C) 2020 The Living Computation Foundation.
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
  \file SymbolTmpVar.h -  Basic handling of TmpVar Symbols for ULAM Code Gen
  \author Elena S. Ackley.
  \author David H. Ackley.
  \date (C) 2016-2020 All rights reserved.
  \gpl
*/


#ifndef SYMBOLTMPREF_H
#define SYMBOLTMPREF_H

#include "Symbol.h"
#include "UVPass.h"

namespace MFM{

  class CompilerState;  //forward

  //used to hold the tmpvar name for a genCodeToStoreInto of an array item
  // e.g. member select, function call.
  class SymbolTmpVar : public Symbol
  {
  public:
    SymbolTmpVar(const Token& id, UTI utype, u32 offset, CompilerState& state);
    ~SymbolTmpVar();

    virtual Symbol * clone();

    virtual bool isTmpVarSymbol();

    virtual const std::string getMangledPrefix();

    virtual const std::string getMangledName();

    virtual u32 getPosOffset();

    virtual bool isPosOffsetReliable();

    void setDivinedByConstantClass();

    bool divinedByConstantClass();

    void setBaseClassRef();

    bool isBaseClassRef();

    void setBaseClassRegNum();

    bool isBaseClassRegNum();

  protected:

  private:
    u32 m_posOffsetCopy; //array item of data member
    bool m_divinedByConstantClass;
    bool m_baseclassref; //ulam-5
    bool m_baseclassregnum; //ulam-5

  };

}

#endif //end SYMBOLTMPREF_H
