/**                                        -*- mode:C++ -*-
 * NodeTerminal.h - Basic Node handling Terminals for ULAM
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
  \file NodeTerminal.h - Basic Node handling Terminals for ULAM
  \author Elenas S. Ackley.
  \author David H. Ackley.
  \date (C) 2014-2015 All rights reserved.
  \gpl
*/


#ifndef NODETERMINAL_H
#define NODETERMINAL_H

#include "Node.h"
#include "NodeIdent.h"
#include "Token.h"


namespace MFM{

  class NodeTerminal : public Node
  {
  public:

    NodeTerminal(CompilerState & state); //for NodeConstant
    NodeTerminal(Token tok, CompilerState & state);
    NodeTerminal(s64 val, UTI utype, CompilerState & state);
    NodeTerminal(u64 val, UTI utype, CompilerState & state);
    NodeTerminal(const NodeTerminal& ref);
    NodeTerminal(const NodeIdent& ref); //passthru for NodeConstant

    virtual ~NodeTerminal();

    virtual Node * instantiate();

    virtual void printPostfix(File * f);

    virtual const char * getName();

    virtual const std::string prettyNodeName();

    virtual void constantFoldAToken(Token tok);

    virtual bool isAConstant();

    virtual bool isReadyConstant();

    virtual bool safeToCastTo(UTI newType);

    virtual UTI checkAndLabelType();

    virtual bool fitsInBits(UTI fituti);

    virtual bool isNegativeConstant();

    virtual bool isWordSizeConstant();

    virtual EvalStatus eval();

    virtual void genCode(File * fp, UlamValue& uvpass);

    virtual void genCodeToStoreInto(File * fp, UlamValue& uvpass);

    /** reads into a tmp BitVector */
    virtual void genCodeReadIntoATmpVar(File * fp, UlamValue & uvpass);

  private:
    virtual bool setConstantValue(Token tok);
    virtual UTI setConstantTypeForNode(Token tok);
    bool fitsInBits32(UTI fituti);
    bool fitsInBits64(UTI fituti);

  protected:
    virtual EvalStatus makeTerminalValue(UlamValue& uvarg); //used both by eval and gencode
    union {
      s64 sval;
      u64 uval;
    } m_constant;

  };

}

#endif //end NODETERMINAL_H
