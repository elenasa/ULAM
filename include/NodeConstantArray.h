/**                                        -*- mode:C++ -*-
 * NodeConstantArray.h - Node handling Named Constant arrays for ULAM
 *
 * Copyright (C) 2016-2018 The Regents of the University of New Mexico.
 * Copyright (C) 2016-2024 Ackleyshack LLC.
 * Copyright (C) 2020-2024 The Living Computation Foundation
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
  \file NodeConstantArray.h - Node handling Named Constant arrays for ULAM
  \author Elena S. Ackley.
  \author David H. Ackley.
  \date (C) 2016-2024 All rights reserved.
  \gpl
*/


#ifndef NODECONSTANTARRAY_H
#define NODECONSTANTARRAY_H

#include "NodeBlock.h"
#include "Node.h"
#include "SymbolWithValue.h"
#include "Token.h"
#include "NodeTypeDescriptor.h"

namespace MFM{

  class NodeConstantArray : public Node
  {
  public:

    NodeConstantArray(const Token& tok, SymbolWithValue * symptr, NodeTypeDescriptor * typedesc, CompilerState & state);

    NodeConstantArray(const Token& tok, NNO stblockno, UTI constantType, NodeTypeDescriptor * typedesc, CompilerState & state);

    NodeConstantArray(const NodeConstantArray& ref);

    virtual ~NodeConstantArray();

    virtual Node * instantiate();

    virtual void updateLineage(NNO pno);

    virtual bool findNodeNo(NNO n, Node *& foundNode);

    virtual void printPostfix(File * fp);

    virtual const char * getName();

    virtual const std::string prettyNodeName();

    virtual bool getSymbolPtr(const Symbol *& symptrref);

    virtual bool hasASymbolDataMember();

    virtual bool isReadyConstant();

    virtual bool isAConstant();

    virtual bool isAConstantClassArray();

    virtual FORECAST safeToCastTo(UTI newType);

    virtual UTI checkAndLabelType(Node * thisparentnode);

    virtual TBOOL checkVarUsedBeforeDeclared(u32 id, NNO declblockno);

    virtual bool getConstantValue(BV8K& bval);

    virtual EvalStatus eval();

    virtual EvalStatus evalToStoreInto();

    virtual void genCode(File * fp, UVPass& uvpass);

    virtual void genCodeToStoreInto(File * fp, UVPass& uvpass);

  protected:
    Token m_token;
    NodeTypeDescriptor * m_nodeTypeDesc;
    SymbolWithValue * m_constSymbol;
    UTI m_constType;

    virtual void checkForSymbol();

    virtual void clearSymbolPtr();


  private:
    NNO m_currBlockNo;
    NodeBlock * m_currBlockPtr; //could be NULL

    void setBlockNo(NNO n);
    NNO getBlockNo() const;
    NodeBlock * getBlock();
    void setBlock(NodeBlock * ptr);

    UTI checkUsedBeforeDeclared();

    UlamValue makeUlamValuePtr();
    void makeUVPassForCodeGen(UVPass& uvpass);

    bool getArrayValue(BV8K& bvtmp);
  };

}

#endif //end NODECONSTANTARRAY_H
