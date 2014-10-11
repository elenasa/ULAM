/**                                        -*- mode:C++ -*-
 * NodeTerminalIdent.h - Node handling Terminal Identifiers for ULAM
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
  \file NodeTerminalIdent.h - Node handling Terminal Identifiers for ULAM
  \author Elenas S. Ackley.
  \author David H. Ackley.
  \date (C) 2014 All rights reserved.
  \gpl
*/


#ifndef NODETERMINALIDENT_H
#define NODETERMINALIDENT_H

#include "NodeTerminal.h"
#include "Token.h"
#include "SymbolVariable.h"
#include "UlamType.h"

namespace MFM{
  
  class NodeTerminalIdent : public NodeTerminal
  {
  public:
    
    NodeTerminalIdent(Token tok, SymbolVariable * symptr, CompilerState & state);
    ~NodeTerminalIdent();

    virtual void printPostfix(File * fp);

    virtual UTI checkAndLabelType();

    virtual EvalStatus eval();

    virtual EvalStatus evalToStoreInto();

    virtual bool getSymbolPtr(Symbol *& symptrref);

    virtual bool installSymbolTypedef(Token atok, s32 bitsize, s32 arraysize, Symbol *& asymptr);

    virtual bool installSymbolVariable(Token atok, s32 bitsize, s32 arraysize, Symbol *& asymptr);

    virtual const char * getName();

    virtual const std::string prettyNodeName();

    virtual void genCode(File * fp, UlamValue& uvpass);

    virtual void genCodeToStoreInto(File * fp, UlamValue& uvpass);

  private:
    SymbolVariable * m_varSymbol;
    SymbolVariable *  makeSymbol(UTI aut);
    UlamValue makeUlamValuePtr();

  };

}

#endif //end NODETERMINALIDENT_H
