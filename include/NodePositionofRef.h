/**                                        -*- mode:C++ -*-
 * NodePositionofRef.h - Node handling positionof operator on references for ULAM
 *
 * Copyright (C) 2022-2024 Ackleyshack LLC.
 * Copyright (C) 2022-2024 The Living Computation Foundation.
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
  \file NodePositionofRef.h - Node handling positionof operator on references for ULAM
  \author Elena S. Ackley.
  \author David H. Ackley.
  \date (C) 2022-2024  All rights reserved.
  \gpl
*/


#ifndef NODEPOSITIONOFREF_H
#define NODEPOSITIONOFREF_H

#include "Node.h"

namespace MFM{

  class NodePositionofRef : public Node
  {
  public:

    NodePositionofRef(Node * memberNode, CompilerState & state);

    NodePositionofRef(const NodePositionofRef& ref);

    virtual ~NodePositionofRef();

    virtual Node * instantiate();

    virtual void updateLineage(NNO pno);

    virtual bool exchangeKids(Node * oldnptr, Node * newnptr);

    virtual bool findNodeNo(NNO n, Node *& foundNode);

    virtual void printPostfix(File * fp);

    virtual const char * getName();

    virtual u32 getNameId();

    virtual const std::string prettyNodeName();

    virtual bool isAConstant();

    virtual FORECAST safeToCastTo(UTI newType);

    virtual UTI checkAndLabelType(Node * thisparentnode);

    virtual TBOOL checkVarUsedBeforeDeclared(u32 id, NNO declblockno);

    virtual void countNavHzyNoutiNodes(u32& ncnt, u32& hcnt, u32& nocnt);

    virtual EvalStatus eval();

    virtual void genCode(File * fp, UVPass& uvpass);

    virtual void genCodeToStoreInto(File * fp, UVPass& uvpass);

  private:
    Node * m_nodeOf; //useful when type is not available at parse

  };

} //MFM

#endif //end NODEPOSITIONOFREF_H
