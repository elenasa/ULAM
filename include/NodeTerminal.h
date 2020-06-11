/**                                        -*- mode:C++ -*-
 * NodeTerminal.h - Basic Node handling Terminals for ULAM
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
  \file NodeTerminal.h - Basic Node handling Terminals for ULAM
  \author Elena S. Ackley.
  \author David H. Ackley.
  \date (C) 2014-2017  All rights reserved.
  \gpl
*/


#ifndef NODETERMINAL_H
#define NODETERMINAL_H

#include "Node.h"
#include "Token.h"


namespace MFM{

  class NodeTerminal : public Node
  {
  public:

    NodeTerminal(CompilerState & state); //for NodeConstant
    NodeTerminal(const Token& tok, CompilerState & state);
    NodeTerminal(s64 val, UTI utype, CompilerState & state);
    NodeTerminal(u64 val, UTI utype, CompilerState & state);
    NodeTerminal(const NodeTerminal& ref);

    virtual ~NodeTerminal();

    virtual Node * instantiate();

    virtual void printPostfix(File * f);

    virtual const char * getName();

    virtual const std::string prettyNodeName();

    virtual bool isAConstant();

    virtual bool isReadyConstant();

    virtual FORECAST safeToCastTo(UTI newType);

    virtual UTI checkAndLabelType();

    virtual bool isNegativeConstant();

    virtual bool isWordSizeConstant();

    virtual EvalStatus eval();

    virtual void genCode(File * fp, UVPass& uvpass);

    virtual void genCodeToStoreInto(File * fp, UVPass& uvpass);

    /** reads into a tmp BitVector */
    virtual void genCodeReadIntoATmpVar(File * fp, UVPass & uvpass);

    virtual UTI setConstantTypeForNode(const Token& tok);

  private:
    ULAMTYPE m_etyp;

    virtual bool setConstantValue(const Token& tok);

    bool fitsInBits32(UTI fituti);
    bool fitsInBits64(UTI fituti);

    EvalStatus makeTerminalValue(UlamValue& uvarg, u32 data, UTI uti);
    EvalStatus makeTerminalValueLong(UlamValue& uvarg, u64 data, UTI uti);
    u32 convertForthAndBack(const u32 data, UTI fituti);
    u64 convertForthAndBackLong(const u64 data, UTI fituti);
    bool fitsInBits32compare(UTI fituti);

    u32 getNameId();

  protected:
    virtual EvalStatus makeTerminalValue(UlamValue& uvarg); //used by eval only
    virtual void makeTerminalPassForCodeGen(UVPass& uvpass);

    bool fitsInBits(UTI fituti);

    union {
      s64 sval;
      u64 uval;
    } m_constant;

  };

}

#endif //end NODETERMINAL_H
