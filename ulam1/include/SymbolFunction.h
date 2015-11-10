/**                                        -*- mode:C++ -*-
 * SymbolFunction.h -  Function Symbol handling for ULAM
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
  \file SymbolFunction.h -  Function Symbol handling for ULAM
  \author Elenas S. Ackley.
  \author David H. Ackley.
  \date (C) 2014-2015 All rights reserved.
  \gpl
*/


#ifndef SYMBOLFUNCTION_H
#define SYMBOLFUNCTION_H

#include <vector>
#include "Symbol.h"
#include "UlamTypeClass.h"
#include "Node.h"

namespace MFM{

  class NodeBlockFunctionDefinition;  //forward
  class CompilerState;   //forward

  class SymbolFunction : public Symbol
  {
  public:
    SymbolFunction(Token id, UTI typetoreturn, CompilerState& state);
    SymbolFunction(const SymbolFunction& sref);
    virtual ~SymbolFunction();

    virtual Symbol * clone();

    void addParameterSymbol(Symbol * argSym);
    u32 getNumberOfParameters();
    u32 getTotalParameterSlots();

    Symbol * getParameterSymbolPtr(u32 n);

    void markForVariableArgs(bool m = true);
    bool takesVariableArgs();

    virtual bool isFunction();
    void setFunctionNode(NodeBlockFunctionDefinition * func);
    NodeBlockFunctionDefinition *  getFunctionNode();

    virtual const std::string getMangledPrefix();

    const std::string getMangledNameWithTypes();
    const std::string getMangledNameWithUTIparameters();

    bool checkParameterTypes();

    bool matchingTypesStrictly(std::vector<UTI> argTypes);
    bool matchingTypes(std::vector<UTI> argTypes, std::vector<Node *> constantArg, bool& hasHazyArgs, u32& numUTmatch);

    u32 isNativeFunctionDeclaration();

    void generateFunctionDeclaration(File * fp, bool declOnly, ULAMCLASSTYPE classtype);

    virtual void setStructuredComment();

  protected:


  private:
    std::vector<Symbol *> m_parameterSymbols;  // variable or function can be an args
    NodeBlockFunctionDefinition * m_functionNode;
    bool m_hasVariableArgs;
  };

}

#endif //end SYMBOLFUNCTION_H
