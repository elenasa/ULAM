/**                                        -*- mode:C++ -*-
 * SymbolTmpRef.h -  Basic handling of TmpRef Symbols for ULAM Code Gen
 *
 * Copyright (C) 2016 The Regents of the University of New Mexico.
 * Copyright (C) 2016 Ackleyshack LLC.
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
  \file SymbolTmpRef.h -  Basic handling of TmpRef Symbols for ULAM Code Gen
  \author Elenas S. Ackley.
  \author David H. Ackley.
  \date (C) 2016 All rights reserved.
  \gpl
*/


#ifndef SYMBOLTMPREF_H
#define SYMBOLTMPREF_H

#include "Symbol.h"

namespace MFM{

  class CompilerState;  //forward

  //used to hold the tmpvar name for a genCodeToStoreInto of an array item
  // e.g. member select, function call.
  class SymbolTmpRef : public Symbol
  {
  public:
    SymbolTmpRef(const Token& id, UTI utype, u32 offset, CompilerState& state);
    ~SymbolTmpRef();

    virtual Symbol * clone();

    virtual bool isTmpRefSymbol();

    virtual const std::string getMangledPrefix();

    virtual const std::string getMangledName();

    virtual u32 getPosOffset();

  protected:

  private:
    u32 m_posOffsetCopy; //array item of data member
  };

}

#endif //end SYMBOLTMPREF_H
