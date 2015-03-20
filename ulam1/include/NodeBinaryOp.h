/**                                        -*- mode:C++ -*-
 * NodeBinaryOp.h - Basic Node for handling Binary Operations for ULAM
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
  \file NodeBinaryOp.h - Basic Node for handling Binary Operations for ULAM
  \author Elenas S. Ackley.
  \author David H. Ackley.
  \date (C) 2014-2015 All rights reserved.
  \gpl
*/

#ifndef NODEBINARYOP_H
#define NODEBINARYOP_H

#include <assert.h>
#include "Node.h"
#include "NodeCast.h"

namespace MFM{

  class NodeBinaryOp : public Node
  {
  public:

    NodeBinaryOp(Node * left, Node * right, CompilerState & state);
    NodeBinaryOp(const NodeBinaryOp& ref);
    virtual ~NodeBinaryOp();

    virtual void updateLineage(NNO pno);

    virtual bool exchangeKids(Node * oldnptr, Node * newnptr);

    virtual bool findNodeNo(NNO n, Node *& foundNode);

    virtual void print(File * fp);

    virtual void printPostfix(File * fp);

    virtual void printOp(File * fp);

    virtual const std::string methodNameForCodeGen() = 0;

    virtual bool isAConstant();

    virtual bool isReadyConstant();

    virtual UTI checkAndLabelType();

    virtual void countNavNodes(u32& cnt);

    virtual UTI constantFold();

    virtual bool assignClassArgValueInStubCopy();

    virtual EvalStatus eval();

    virtual void genCode(File * fp, UlamValue& uvpass);

    virtual void genCodeToStoreInto(File * fp, UlamValue& uvpass);

  protected:
    Node * m_nodeLeft;
    Node * m_nodeRight;

    virtual void doBinaryOperation(s32 lslot, s32 rslot, u32 slots) = 0;

    virtual void doBinaryOperationImmediate(s32 lslot, s32 rslot, u32 slots);
    virtual void doBinaryOperationArray(s32 lslot, s32 rslot, u32 slots);

    virtual UlamValue makeImmediateBinaryOp(UTI type, u32 ldata, u32 rdata, u32 len) = 0;
    virtual void appendBinaryOp(UlamValue& refUV, u32 ldata, u32 rdata, u32 pos, u32 len) = 0;

    virtual UTI calcNodeType(UTI lt, UTI rt) = 0;

  };

}

#endif //NODEBINARYOP_H
