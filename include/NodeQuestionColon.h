/**                                        -*- mode:C++ -*-
 * NodeQuestionColon.h - Question Colon Expression Node for ULAM
 *
 * Copyright (C) 2017-2019 The Regents of the University of New Mexico.
 * Copyright (C) 2017-2019 Ackleyshack LLC.
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
  \file NodeQuestionColon.h - Question Colon Expression Node for ULAM
  \author Elena S. Ackley.
  \author David H. Ackley.
  \date (C) 2017-2019 All rights reserved.
  \gpl
*/


#ifndef NODEQUESTIONCOLON_H
#define NODEQUESTIONCOLON_H

#include "NodeBinaryOp.h"

namespace MFM{

  class NodeQuestionColon : public NodeBinaryOp
  {
  public:

    NodeQuestionColon(Node * condNode, Node * trueNode, Node * falseNode, CompilerState & state);
    NodeQuestionColon(const NodeQuestionColon& ref);

    virtual ~NodeQuestionColon();

    virtual Node * instantiate();

    virtual void updateLineage(NNO pno);

    virtual bool exchangeKids(Node * oldnptr, Node * newnptr);

    virtual bool findNodeNo(NNO n, Node *& foundNode);

    virtual void checkAbstractInstanceErrors();

    virtual void resetNodeLocations(Locator loc);

    virtual void print(File * fp);

    virtual void printPostfix(File * fp);

    virtual void printOp(File * fp);

    virtual const std::string methodNameForCodeGen();

    virtual const char * getName();

    virtual const std::string prettyNodeName();

    virtual bool isAConstant();

    virtual bool isTernaryExpression();

    virtual UTI checkAndLabelType();

    virtual void calcMaxDepth(u32& depth, u32& maxdepth, s32 base);

    virtual void countNavHzyNoutiNodes(u32& ncnt, u32& hcnt, u32& nocnt);

    virtual UTI constantFold();

    virtual EvalStatus eval();

    virtual EvalStatus evalToStoreInto();

    virtual void genCode(File * fp, UVPass& uvpass);

    virtual void genCodeToStoreInto(File * fp, UVPass& uvpass);

  protected:

    virtual bool doBinaryOperation(s32 lslot, s32 rslot, u32 slots);
    virtual UlamValue makeImmediateBinaryOp(UTI type, u32 ldata, u32 rdata, u32 len);
    virtual UlamValue makeImmediateLongBinaryOp(UTI type, u64 ldata, u64 rdata, u32 len);
    virtual void appendBinaryOp(UlamValue& refUV, u32 ldata, u32 rdata, u32 pos, u32 len);

    virtual UTI calcNodeType(UTI lt, UTI rt);

  private:
    Node * m_nodeCondition;

    void genCodeConditionalExpression(File * fp, UVPass& uvpass);
    void genCodeToStoreIntoExpression(File * fp, UVPass& uvpass, s32 tmpVarNum);
  };

}

#endif //end NODEQUESTIONCOLON_H
