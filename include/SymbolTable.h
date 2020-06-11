/**                                        -*- mode:C++ -*-
 * SymbolTable.h -  Basic handling of Table of Symbols for ULAM
 *
 * Copyright (C) 2014-2017 The Regents of the University of New Mexico.
 * Copyright (C) 2014-2017 Ackleyshack LLC.
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
  \author Elena S. Ackley.
  \author David H. Ackley.
  \date (C) 2014-2017 All rights reserved.
  \gpl
*/


#ifndef SYMBOLTABLE_H
#define SYMBOLTABLE_H

#include <map>
#include <vector>
#include "Symbol.h"
#include "itype.h"
#include "File.h"
#include "FileManager.h"

namespace MFM{

  struct CompilerState; //forward
  class Node; //forward
  class NodeBlockClass; //forward
  class NodeBlock; //forward

  class SymbolTable
  {
  public:

    SymbolTable(CompilerState& state);
    SymbolTable(const SymbolTable& ref);
    virtual ~SymbolTable();

    virtual bool isInTable(u32 id, Symbol * & symptrref);
    virtual void addToTable(u32 id, Symbol * s);
    virtual void replaceInTable(u32 oldid, u32 newid, Symbol * s);
    virtual void replaceInTable(Symbol * oldsym, Symbol * newsym);
    virtual bool removeFromTable(u32 id, Symbol *& rtnsymptr);

    virtual Symbol * getSymbolPtr(u32 id);

    virtual u32 getTableSize();

    virtual u32 getTotalSymbolSize();

  protected:
    std::map<u32, Symbol* > m_idToSymbolPtr;
    CompilerState & m_state;

  private:
    void clearTheTable();

  };

}

#endif //end SYMBOLTABLE_H
