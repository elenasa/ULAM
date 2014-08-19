/**                                        -*- mode:C++ -*-
 * SymbolFunctionName.h -  Function Symbol Name handling for ULAM
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
  \file SymbolFunctionName.h -  Function Symbol Name handling for ULAM
  \author Elenas S. Ackley.
  \author David H. Ackley.
  \date (C) 2014 All rights reserved.
  \gpl
*/


#ifndef SYMBOLFUNCTIONNAME_H
#define SYMBOLFUNCTIONNAME_H

#include <map>
#include <vector>
#include "Symbol.h"
#include "SymbolFunction.h"

namespace MFM{


  class SymbolFunctionName : public Symbol
  {
  public:
    SymbolFunctionName(u32 id, UlamType * typetoreturn);
    ~SymbolFunctionName();

    virtual bool isFunction();

    virtual const std::string getMangledPrefix();

    bool overloadFunction(SymbolFunction * fsym, CompilerState * state);

    bool findMatchingFunction(std::vector<UlamType *> argTypes, SymbolFunction *& funcSymbol);

    u32 getDepthSumOfFunctions();

    void labelFunctions();

    void generateCodedFunctions(File * fp);

  protected:

  private:
    std::map<std::string, SymbolFunction *> m_mangledFunctionNames; //mangled func name -> symbol function ptr	 
    bool isDefined(std::string mangledFName, SymbolFunction * & foundSym);


  };

}

#endif //end SYMBOLFUNCTIONNAME_H
