/**                                        -*- mode:C++ -*-
 * NodeTerminalProxy.h - Node handling of Unknown Type Sizes for ULAM
 *
 * Copyright (C) 2016-2020 The Regents of the University of New Mexico.
 * Copyright (C) 2016-2024 Ackleyshack LLC.
 * Copyright (C) 2020-2024 The Living Computation Foundation.
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
  \file NodeTerminalProxy.h - Node handling Unknown Type Sizes for ULAM
  \author Elena S. Ackley.
  \author David H. Ackley.
  \date (C) 2016-2024  All rights reserved.
  \gpl
*/


#ifndef NODETERMINALPROXY_H
#define NODETERMINALPROXY_H

#include "NodeTerminal.h"
#include "Token.h"
#include "NodeTypeDescriptor.h"
#include "NodeIdent.h"

namespace MFM{

  class NodeTerminalProxy : public NodeTerminal
  {
  public:

    NodeTerminalProxy(Node * memberNode, UTI memberType, const Token& funcTok, NodeTypeDescriptor * nodetype, CompilerState & state);

    NodeTerminalProxy(const NodeTerminalProxy& ref);

    virtual ~NodeTerminalProxy();

    virtual Node * instantiate();

    virtual void updateLineage(NNO pno);

    virtual bool exchangeKids(Node * oldnptr, Node * newnptr);

    virtual bool findNodeNo(NNO n, Node *& foundNode);

    virtual void printPostfix(File * fp);

    virtual const char * getName();

    virtual u32 getNameId();

    virtual const std::string prettyNodeName();

    virtual TBOOL isAConstant();

    virtual bool isReadyConstant();

    virtual FORECAST safeToCastTo(UTI newType);

    virtual UTI checkAndLabelType(Node * thisparentnode);

    virtual void countNavHzyNoutiNodes(u32& ncnt, u32& hcnt, u32& nocnt);

    virtual EvalStatus eval();

    virtual void genCode(File * fp, UVPass& uvpass);

    virtual void genCodeToStoreInto(File * fp, UVPass& uvpass);

  private:
    Node * m_nodeOf; //useful when type is not available at parse
    UTI m_uti; //lhs type of func
    Token m_funcTok; //minof, maxof or sizeof
    bool m_ready;
    NodeTypeDescriptor * m_nodeTypeDesc; //can be NULL

    virtual bool setConstantValue(const Token& tok);
    virtual UTI setConstantTypeForNode(const Token& tok);
    bool updateProxy();

    bool replaceOurselvesLengthOf(Node * parentnode);
    Node * buildAlengthofFuncCallNode(); //like toInt cast in Node
    Node* constantFoldLengthofConstantString();

    bool replaceOurselvesPositionofRef(Node * parentnode);
    Node * buildApositionofrefNode(); //like toInt cast in Node

    void genCodeForUserStringLength(File * fp, UVPass& uvpass);
    bool checkForClassType();
    bool checkForClassIdOfType();

  };

} //MFM

#endif //end NODETERMINALPROXY_H
