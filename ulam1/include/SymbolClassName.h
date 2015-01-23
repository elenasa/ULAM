/**                                        -*- mode:C++ -*-
 * SymbolClassName.h -  Class Symbol "Template" for ULAM
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
  \file SymbolClassName.h -  Class Symbol "Template" for ULAM
  \author Elenas S. Ackley.
  \author David H. Ackley.
  \date (C) 2015 All rights reserved.
  \gpl
*/

#ifndef SYMBOLCLASSNAME_H
#define SYMBOLCLASSNAME_H

#include "SymbolClass.h"
#include "SymbolConstantValue.h"

namespace MFM{

  class SymbolTable;  //forward

  class SymbolClassName : public SymbolClass
  {
  public:
    SymbolClassName(u32 id, UTI utype, NodeBlockClass * classblock, CompilerState& state);
    ~SymbolClassName();

    void addParameterSymbol(SymbolConstantValue * argSym);
    u32 getNumberOfParameters();
    u32 getTotalSizeOfParameters();
    Symbol * getParameterSymbolPtr(u32 n);

    bool isClassInstanceInTable(UTI uti, SymbolClass * & symptrref);
    void addClassInstanceToTable(UTI uti, SymbolClass * symptr);

   protected:

  private:
    //ordered class parameters
    std::vector<SymbolConstantValue *> m_parameterSymbols;  // like named constants; symbols owned by m_ST.
    SymbolTable m_classInstanceST; //owner of class symbols; indexed by UTI (not name id)
  };

}

#endif //end SYMBOLCLASSNAME_H
