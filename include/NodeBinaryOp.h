/**                                        -*- mode:C++ -*-
 * NodeBinaryOp.h - Basic Node for handling Binary Operations for ULAM
 *
 * Copyright (C) 2014-2017 The Regents of the University of New Mexico.
 * Copyright (C) 2014-2021 Ackleyshack LLC.
 * Copyright (C) 2020-2021 The Living Computation Foundation
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
  \author Elena S. Ackley.
  \author David H. Ackley.
  \date (C) 2014-2021 All rights reserved.
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

    virtual void checkAbstractInstanceErrors();

    virtual void resetNodeLocations(Locator loc);

    virtual void print(File * fp);

    virtual void printPostfix(File * fp);

    virtual void printOp(File * fp);

    virtual const std::string methodNameForCodeGen() = 0;

    virtual bool isAConstant();

    virtual bool isReadyConstant();

    virtual bool isFunctionCall();

    virtual FORECAST safeToCastTo(UTI newType);

    virtual UTI checkAndLabelType(Node * thisparentnode);

    virtual void countNavHzyNoutiNodes(u32& ncnt, u32& hcnt, u32& nocnt);

    virtual UTI constantFold(Node * parentnode);

    virtual EvalStatus eval();

    virtual void calcMaxDepth(u32& depth, u32& maxdepth, s32 base);

    virtual void genCode(File * fp, UVPass& uvpass);

    virtual void genCodeToStoreInto(File * fp, UVPass& uvpass);

  protected:
    Node * m_nodeLeft;
    Node * m_nodeRight;

    virtual bool doBinaryOperation(s32 lslot, s32 rslot, u32 slots) = 0;
    virtual bool doBinaryOperationImmediate(s32 lslot, s32 rslot, u32 slots);
    virtual bool doBinaryOperationArray(s32 lslot, s32 rslot, u32 slots);

    virtual UlamValue makeImmediateBinaryOp(UTI type, u32 ldata, u32 rdata, u32 len) = 0;
    virtual UlamValue makeImmediateLongBinaryOp(UTI type, u64 ldata, u64 rdata, u32 len) = 0;
    virtual void appendBinaryOp(UlamValue& refUV, u32 ldata, u32 rdata, u32 pos, u32 len) = 0;

    virtual TBOOL buildandreplaceOperatorOverloadFuncCallNode(Node * parentnode);
    virtual Node * buildOperatorOverloadFuncCallNode(bool& hazyArg);

    virtual UTI calcNodeType(UTI lt, UTI rt) = 0;
    virtual s32 resultBitsize(UTI lt, UTI rt); //op specific

    void resultBitsizeCalc(UTI lt, UTI rt, s32& lbs, s32&rbs, s32&lwordsize);
    void calcBitsizeForResult(UTI uti, s32& bs, s32&wordsize, ULAMTYPE& typEnum); //helper

    void resultBitsizeCalcInBits(UTI lt, UTI rt, s32& lbs, s32&rbs, s32&lwordsize);
    void calcBitsizeForResultInBits(UTI uti, s32& bs, s32&wordsize); //helper


    virtual UTI castThyselfToResultType(UTI rt, UTI lt, UTI newType, Node *& parentnoderef);
    //common helpers for calcNodeType:
    virtual bool checkSafeToCastTo(UTI fromType, UTI& newType);

    bool checkForPrimitiveNotVoidTypes(UTI lt, UTI rt);
    bool checkForPrimitiveTypes(UTI lt, UTI rt, bool quietly);
    bool checkNotVoidTypes(UTI lt, UTI rt, bool quietly);
    bool checkForNumericTypes(UTI lt, UTI rt);
    bool checkScalarTypesOnly(UTI lt, UTI rt, bool quietly=false);
  };

}

#endif //NODEBINARYOP_H
