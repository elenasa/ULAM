/**                                        -*- mode:C++ -*-
 * NodeUnaryOp.h -  Basic Node handling Unary Operations for ULAM
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
  \file NodeUnaryOp.h -  Basic Node handling Unary Operations for ULAM
  \author Elenas S. Ackley.
  \author David H. Ackley.
  \date (C) 2014-2015 All rights reserved.
  \gpl
*/


#ifndef NODEUNARYOP_H
#define NODEUNARYOP_H

#include "Node.h"

namespace MFM{

  class NodeUnaryOp : public Node
  {
  public:

    NodeUnaryOp(Node * n, CompilerState & state);

    NodeUnaryOp(const NodeUnaryOp& ref);

    virtual ~NodeUnaryOp();

    virtual void updateLineage(NNO pno);

    virtual bool exchangeKids(Node * oldnptr, Node * newnptr);

    virtual bool findNodeNo(NNO n, Node *& foundNode);

    virtual void checkAbstractInstanceErrors();

    virtual void print(File * fp);

    virtual void printPostfix(File * fp);

    virtual void printOp(File * fp);

    virtual bool isAConstant();

    virtual bool isReadyConstant();

    virtual FORECAST safeToCastTo(UTI newType);

    virtual UTI checkAndLabelType();

    virtual void countNavHzyNoutiNodes(u32& ncnt, u32& hcnt, u32& nocnt);

    virtual UTI constantFold();

    virtual const std::string methodNameForCodeGen();

    virtual bool assignClassArgValueInStubCopy();

    virtual EvalStatus eval();

    virtual void calcMaxDepth(u32& depth, u32& maxdepth, s32 base);

    virtual void genCode(File * fp, UlamValue& uvpass);

    virtual void genCodeToStoreInto(File * fp, UlamValue& uvpass);

  protected:

    Node * m_node;

    virtual UTI calcNodeType(UTI uti) = 0;

    bool checkSafeToCastTo(UTI& newType);
    s32 resultBitsize(UTI uti);
    bool checkForPrimitiveType(UTI uti);
    bool checkNotVoidType(UTI uti);
    bool checkForNumericType(UTI uti);

    virtual bool doUnaryOperation(s32 slot, u32 nslots);
    virtual bool doUnaryOperationImmediate(s32 slot, u32 nslots);
    virtual UlamValue makeImmediateUnaryOp(UTI type, u32 data, u32 len) = 0;

  };

}

#endif //end NODEUNARYOP_H
