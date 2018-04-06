/**                                        -*- mode:C++ -*-
 * NodeConstantClass.h - Node handling Named Constant classes for ULAM
 *
 * Copyright (C) 2018 The Regents of the University of New Mexico.
 * Copyright (C) 2018 Ackleyshack LLC.
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
  \file NodeConstantClass.h - Node handling Named Constant classes for ULAM
  \author Elenas S. Ackley.
  \author David H. Ackley.
  \date (C) 2018 All rights reserved.
  \gpl
*/


#ifndef NODECONSTANTCLASS_H
#define NODECONSTANTCLASS_H

#include "NodeBlock.h"
#include "Node.h"
#include "SymbolWithValue.h"
#include "Token.h"
#include "NodeTypeDescriptor.h"

namespace MFM{

  class NodeConstantClass : public Node
  {
  public:

    NodeConstantClass(const Token& tok, SymbolWithValue * symptr, NodeTypeDescriptor * typedesc, CompilerState & state);

    NodeConstantClass(const NodeConstantClass& ref);

    virtual ~NodeConstantClass();

    virtual Node * instantiate();

    virtual void updateLineage(NNO pno);

    virtual bool findNodeNo(NNO n, Node *& foundNode);

    virtual void printPostfix(File * fp);

    virtual const char * getName();

    virtual const std::string prettyNodeName();

    virtual bool getSymbolPtr(Symbol *& symptrref);

    virtual bool hasASymbolDataMember();

    virtual bool isReadyConstant();

    virtual bool isAConstant();

    virtual bool isAConstantClass();

    virtual void setClassType(UTI cuti); //noop

    virtual FORECAST safeToCastTo(UTI newType);

    virtual UTI checkAndLabelType();

    virtual bool assignClassArgValueInStubCopy();

    virtual bool getConstantValue(BV8K& bval);

    virtual bool initDataMembersConstantValue(BV8K& bvref, BV8K& bvmask);

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

  private:
    NNO m_currBlockNo;
    NodeBlock * m_currBlockPtr; //could be NULL
    SymbolTmpVar * m_tmpvarSymbol;

    void setupBlockNo();
    void setBlockNo(NNO n);
    NNO getBlockNo() const;
    NodeBlock * getBlock();
    void setBlock(NodeBlock * ptr);

    UTI checkUsedBeforeDeclared();

    UlamValue makeUlamValuePtr();
    void makeUVPassForCodeGen(UVPass& uvpass);
    void setupStackWithClassForEval();

    bool getClassValue(BV8K& bvtmp);
  };

}

#endif //end NODECONSTANTCLASS_H
