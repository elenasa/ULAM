/**                                        -*- mode:C++ -*-
 * SymbolTable.h -  Basic handling of Table of Symbols for ULAM
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
  \file SymbolTable.h -  Basic handling of Table of Symbols for ULAM
  \author Elenas S. Ackley.
  \author David H. Ackley.
  \date (C) 2014 All rights reserved.
  \gpl
*/


#ifndef SYMBOLTABLE_H
#define SYMBOLTABLE_H

#include <map>
#include "Symbol.h"
#include "itype.h"
#include "File.h"

namespace MFM{

  class SymbolTable
  {
  public:

    SymbolTable();
    ~SymbolTable();

    bool isInTable(u32 id, Symbol * & symptrref);
    void addToTable(u32 id, Symbol * s);

    Symbol * getSymbolPtr(u32 id);

    void labelTableOfFunctions();

    void genCodeForTableOfFunctions(File * fp);

    u32 getTableSize();

    u32 getTotalSymbolSize();

  protected:
    std::map<u32, Symbol* > m_idToSymbolPtr;

  private:



  };

}

#endif //end SYMBOL_H
