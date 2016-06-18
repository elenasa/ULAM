/**                                        -*- mode:C++ -*-
 * SymbolWithValue.h - Basic handling of symbols with values for ULAM
 *
 * Copyright (C) 2015-2106 The Regents of the University of New Mexico.
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
  \file SymbolWithValue.h - Basic handling of symbols with values for ULAM
  \author Elenas S. Ackley.
  \author David H. Ackley.
  \date (C) 2015-2016 All rights reserved.
  \gpl
*/

#ifndef SYMBOLWITHVALUE_H
#define SYMBOLWITHVALUE_H

#include "Symbol.h"

namespace MFM{

  class CompilerState;  //forward

  //distinguish between Symbols
  class SymbolWithValue : public Symbol
  {
  public:
    SymbolWithValue(Token id, UTI utype, CompilerState& state);
    SymbolWithValue(const SymbolWithValue& sref);
    SymbolWithValue(const SymbolWithValue& sref, bool keepType);
    virtual ~SymbolWithValue();

    virtual Symbol * clone() = 0;
    virtual Symbol * cloneKeepsType() = 0;

    virtual u32 getPosOffset();

    virtual bool isConstant() = 0;

    bool isClassParameter();
    void setClassParameterFlag();

    bool isClassArgument();
    void setClassArgumentFlag();

    virtual bool isReady();

    bool getLexValue(std::string& vstr);
    bool getValue(u32& val);
    bool getValue(u64& val);
    bool getValue(BV8K& val);
    void setValue(const BV8K& val);

    bool hasInitValue();
    bool getInitValue(u32& val);
    bool getInitValue(u64& val);
    bool getInitValue(BV8K& val);
    void setInitValue(const BV8K& val);
    bool isInitValueReady(); //new
    void setHasInitValue(); //new

    bool foldConstantExpression();

    void printPostfixValue(File * fp);

    virtual const std::string getMangledPrefix() = 0;

    virtual void printPostfixValuesOfVariableDeclarations(File * fp, s32 slot, u32 startpos, ULAMCLASSTYPE classtype) = 0;

    void changeConstantId(u32 fmid, u32 toid); //for premature class instances

  protected:

  private:
    bool m_isReady;
    bool m_hasInitVal;
    bool m_isReadyInitVal;
    bool m_classParameter; //constant has default but no value, look at instance
    bool m_classArgument; //constant has value but no default, look at template parameter

    BV8K m_constantValue;
    BV8K m_initialValue;

    void printPostfixValueScalar(File * fp);
    void printPostfixValueArray(File * fp);
  };
} //MFM

#endif //end SYMBOLWITHVALUE_H
