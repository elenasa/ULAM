/**                                        -*- mode:C++ -*-
 * SymbolTypedef.h -  Basic handling of Typedef Symbols for ULAM
 *
 * Copyright (C) 2014-2017 The Regents of the University of New Mexico.
 * Copyright (C) 2014-2021 Ackleyshack LLC.
 * Copyright (C) 2021 The Living Computation Foundation.
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
  \file SymbolTypedef.h -  Basic handling of Typedef Symbols for ULAM
  \author Elena S. Ackley.
  \author David H. Ackley.
  \date (C) 2014-2021 All rights reserved.
  \gpl
*/


#ifndef SYMBOLTYPEDEF_H
#define SYMBOLTYPEDEF_H

#include "UlamValue.h"
#include "Symbol.h"

namespace MFM{

  class CompilerState;  //forward

  //distinguish between Symbols
  class SymbolTypedef : public Symbol
  {
  public:
    SymbolTypedef(const Token& id, UTI utype, UTI scalaruti, CompilerState& state);
    SymbolTypedef(const SymbolTypedef& sref);
    SymbolTypedef(const SymbolTypedef& sref, bool keeptype);
    ~SymbolTypedef();

    virtual Symbol * clone();
    virtual Symbol * cloneKeepsType();

    virtual bool isTypedef();

    UTI getScalarUTI();

    virtual const std::string getMangledPrefix();

    virtual void printPostfixValuesOfVariableDeclarations(File * fp, s32 slot, u32 startpos, ULAMCLASSTYPE classtype);

    virtual void setStructuredComment();

    virtual bool isCulamGeneratedTypedef();
    void setCulamGeneratedTypedef();
    void clearCulamGeneratedTypedef();
    virtual bool isCulamGeneratedTypedefAliased();
    void setCulamGeneratedTypedefAliased();
  protected:

  private:
    UTI m_scalarUTI; //when utype is an array
    bool m_culamgenerated;
    bool m_culamgeneratedaliased; //good as deleted, but better.
  };

}

#endif //end SYMBOLTYPEDEF_H
