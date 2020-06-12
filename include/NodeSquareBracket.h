/**                                        -*- mode:C++ -*-
 * NodeSquareBracket.h - Basic Node for handling
 *                               Array Subscripts for ULAM
 *
 * Copyright (C) 2014-2018 The Regents of the University of New Mexico.
 * Copyright (C) 2014-2018 Ackleyshack LLC.
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
  \author Elena S. Ackley.
  \author David H. Ackley.
  \date (C) 2014-2018  All rights reserved.
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

    virtual void updateLineage(NNO pno);

    virtual bool findNodeNo(NNO n, Node *& foundNode);

    virtual void printOp(File * fp);

    virtual const char * getName();

    virtual const std::string prettyNodeName();

    virtual const std::string methodNameForCodeGen();

    virtual bool isArrayItem();

    virtual UTI checkAndLabelType();

    virtual bool getConstantValue(BV8K& bval);

    virtual bool trimToTheElement(Node ** fromleftnode, Node *& rtnnodeptr);

    virtual bool assignClassArgValueInStubCopy();

    virtual EvalStatus eval();

    virtual EvalStatus evalToStoreInto();

    virtual bool getSymbolPtr(Symbol *& symptrref);

    virtual bool getStorageSymbolPtr(Symbol *& symptrref);

    virtual bool installSymbolTypedef(TypeArgs& args, Symbol *& asymptr);
    virtual bool installSymbolConstantValue(TypeArgs& args, Symbol *& asymptr);
    virtual bool installSymbolModelParameterValue(TypeArgs& args, Symbol*& asymptr);
    virtual bool installSymbolVariable(TypeArgs& args,  Symbol *& asymptr);

    //helper method to install symbol; also called by Resolver for unknown arraysize
    bool getArraysizeInBracket(s32 & rtnArraySize, UTI& sizetype);

    virtual void genCode(File * fp, UVPass& uvpass);

    virtual void genCodeToStoreInto(File * fp, UVPass& uvpass);

  protected:

    virtual Node * buildOperatorOverloadFuncCallNode();

  private:
    bool m_isCustomArray;
    SymbolTmpVar * m_tmpvarSymbol;

    Node * buildArefFuncCallNode();

    EvalStatus evalAUserStringByte();

    virtual bool doBinaryOperation(s32 lslot, s32 rslot, u32 slots);
    virtual UlamValue makeImmediateBinaryOp(UTI type, u32 ldata, u32 rdata, u32 len);
    virtual UlamValue makeImmediateLongBinaryOp(UTI type, u64 ldata, u64 rdata, u32 len);
    virtual void appendBinaryOp(UlamValue& refUV, u32 ldata, u32 rdata, u32 pos, u32 len){}

    virtual UTI calcNodeType(UTI lt, UTI rt); //stub

    void genCodeAUserStringByte(File * fp, UVPass& uvpass);

    bool getConstantArrayItemValue(BV8K& bvitem);
  };

}

#endif //NODEBINARYOPSQUAREBRACKET_H
