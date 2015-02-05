/**                                        -*- mode:C++ -*-
 * NodeSquareBracket.h - Basic Node for handling
 *                               Array Subscripts for ULAM
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
  \file NodeSquareBracket.h - Basic Node for handling Array Subscripts for ULAM
  \author Elenas S. Ackley.
  \author David H. Ackley.
  \date (C) 2014-2015 All rights reserved.
  \gpl
*/


#ifndef NODEBINARYOPSQUAREBRACKET_H
#define NODEBINARYOPSQUAREBRACKET_H

#include "NodeBinaryOp.h"

namespace MFM{

  class NodeSquareBracket : public NodeBinaryOp
  {
  public:

    NodeSquareBracket(Node * left, Node * right, CompilerState & state);
    NodeSquareBracket(const NodeSquareBracket& ref);
    virtual ~NodeSquareBracket();

    virtual Node * instantiate();

    virtual void updateLineage(Node * p);

    virtual void printOp(File * fp);

    virtual const char * getName();

    virtual const std::string prettyNodeName();

    virtual const std::string methodNameForCodeGen();

    virtual UTI checkAndLabelType();

    virtual void countNavNodes(u32& cnt);

    virtual EvalStatus eval();

    virtual EvalStatus evalToStoreInto();

    virtual bool getSymbolPtr(Symbol *& symptrref);

    virtual bool installSymbolTypedef(Token atok, s32 bitsize, s32 arraysize, UTI classInstanceIdx, Symbol *& asymptr);

    virtual bool installSymbolConstantValue(Token atok, s32 bitsize, s32 arraysize, Symbol *& asymptr);

    virtual bool installSymbolVariable(Token atok, s32 bitsize, s32 arraysize, UTI classInstanceIdx, Symbol *& asymptr);

    //helper method to install symbol
    bool getArraysizeInBracket(s32 & rtnArraySize);

    virtual void genCode(File * fp, UlamValue& uvpass);

    virtual void genCodeToStoreInto(File * fp, UlamValue& uvpass);

  protected:

  private:
    virtual void doBinaryOperation(s32 lslot, s32 rslot, u32 slots){}
    virtual UlamValue makeImmediateBinaryOp(UTI type, u32 ldata, u32 rdata, u32 len);
    virtual void appendBinaryOp(UlamValue& refUV, u32 ldata, u32 rdata, u32 pos, u32 len){}

    virtual UTI calcNodeType(UTI lt, UTI rt); //stub
  };

}

#endif //end NODEBINARYOPSQUAREBRACKET_H
