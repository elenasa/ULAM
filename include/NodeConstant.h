/**                                        -*- mode:C++ -*-
 * NodeConstant.h - Node handling NamedConstants for ULAM
 *
 * Copyright (C) 2015-2019 The Regents of the University of New Mexico.
 * Copyright (C) 2015-2019 Ackleyshack LLC.
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
  \author Elena S. Ackley.
  \author David H. Ackley.
  \date (C) 2015-2019 All rights reserved.
  \gpl
*/


#ifndef NODECONSTANT_H
#define NODECONSTANT_H

#include "NodeBlock.h"
#include "NodeTerminal.h"
#include "SymbolWithValue.h"
#include "Token.h"
#include "NodeTypeDescriptor.h"

namespace MFM{

  class NodeConstant : public NodeTerminal
  {
  public:

    NodeConstant(const Token& tok, SymbolWithValue * symptr, NodeTypeDescriptor * typedesc, CompilerState & state);

    NodeConstant(const NodeConstant& ref);

    virtual ~NodeConstant();

    virtual Node * instantiate();

    virtual void updateLineage(NNO pno);

    virtual bool findNodeNo(NNO n, Node *& foundNode);

    virtual void printPostfix(File * fp);

    virtual const char * getName();

    virtual const std::string prettyNodeName();

    virtual bool getSymbolPtr(Symbol *& symptrref);

    void setupBlockNo();

    virtual bool hasASymbolDataMember();

    virtual bool isReadyConstant();

    virtual void setClassType(UTI cuti); //noop

    virtual bool getConstantValue(BV8K& bval);

    virtual FORECAST safeToCastTo(UTI newType);

    virtual UTI checkAndLabelType();

    virtual bool assignClassArgValueInStubCopy();

    virtual EvalStatus eval();

    virtual EvalStatus evalToStoreInto();

    virtual void genCode(File * fp, UVPass& uvpass);

    virtual void genCodeToStoreInto(File * fp, UVPass& uvpass);

  protected:
    const Token m_token;
    NodeTypeDescriptor * m_nodeTypeDesc; //can be NULL
    SymbolWithValue * m_constSymbol;
    bool m_ready;
    UTI m_constType;

    virtual void checkForSymbol();
    bool updateConstant();

    void setBlockNo(NNO n);
    NNO getBlockNo() const;

    void setBlock(NodeBlock * ptr);
    NodeBlock * getBlock();

  private:
    NNO m_currBlockNo;
    NodeBlock * m_currBlockPtr; //could be NULL
    SymbolTmpVar * m_tmpvarSymbol;

    bool replaceOurselves(Symbol * symptr);
    UTI checkUsedBeforeDeclared();
    UlamValue makeUlamValuePtr();

  };

}

#endif //end NODECONSTANT_H
