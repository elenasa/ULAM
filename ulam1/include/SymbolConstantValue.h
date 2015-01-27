/**                                        -*- mode:C++ -*-
 * SymbolConstantValue.h - Handling of Named Constant Symbols for ULAM
 *
 * Copyright (C) 2015 The Regents of the University of New Mexico.
 * Copyright (C) 2015 Ackleyshack LLC.
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
  \author Elenas S. Ackley.
  \author David H. Ackley.
  \date (C) 2015 All rights reserved.
  \gpl
*/

#ifndef SYMBOLCONSTANTVALUE_H
#define SYMBOLCONSTANTVALUE_H

#include "Symbol.h"

namespace MFM{

  class CompilerState;  //forward
  class NodeConstantDef;  //forward

  //distinguish between Symbols
  class SymbolConstantValue : public Symbol
  {
  public:
    SymbolConstantValue(u32 id, UTI utype, CompilerState& state);
    SymbolConstantValue(SymbolConstantValue * s);
    ~SymbolConstantValue();

    virtual bool isConstant();

    virtual bool isReady();

    bool isParameter();
    void setParameterFlag();

    bool getValue(s32& val);
    bool getValue(u32& val);
    bool getValue(bool& val);
    void setValue(s32 val);
    void setValue(u32 val);
    void setValue(bool val);

    bool foldConstantExpression();

    virtual const std::string getMangledPrefix();

    virtual void printPostfixValuesOfVariableDeclarations(File * fp, s32 slot, u32 startpos, ULAMCLASSTYPE classtype);

    void changeConstantId(u32 fmid, u32 toid); //for premature class instances

  protected:

  private:
    bool m_isReady;
    bool m_parameter; //i.e. no value, look at instance

    union {
      s32 sval;
      u32 uval;
      bool bval;
    } m_constant;

    NodeConstantDef * m_defnode; //unused
  };

}

#endif //end SYMBOLCONSTANTVALUE_H
