/**                                        -*- mode:C++ -*-
 * NodeConstant.h - Node handling NamedConstants for ULAM
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
  \file NodeConstant.h - Node handling Named Constants for ULAM
  \author Elenas S. Ackley.
  \author David H. Ackley.
  \date (C) 2015 All rights reserved.
  \gpl
*/


#ifndef NODECONSTANT_H
#define NODECONSTANT_H

#include "NodeTerminal.h"
#include "Token.h"
#include "SymbolConstantValue.h"
#include "UlamType.h"
#include "NodeBlock.h"

namespace MFM{

  class NodeConstant : public NodeTerminal
  {
  public:

    NodeConstant(Token tok, SymbolConstantValue * symptr, CompilerState & state);
    NodeConstant(const NodeConstant& ref);
    virtual ~NodeConstant();

    virtual Node * instantiate();

    virtual void printPostfix(File * fp);

    virtual const char * getName();

    virtual const std::string prettyNodeName();

    virtual void constantFold(Token tok);

    virtual bool getSymbolPtr(Symbol *& symptrref);

    virtual UTI checkAndLabelType();

    NNO getBlockNo();
    NodeBlock * getBlock();

    virtual EvalStatus eval();

    virtual void genCode(File * fp, UlamValue& uvpass);

  private:
    Token m_token;
    SymbolConstantValue * m_constSymbol;
    bool m_ready;
    NNO m_currBlockNo;
    bool updateConstant();
  };

}

#endif //end NODECONSTANT_H
