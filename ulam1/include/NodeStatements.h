/**                                        -*- mode:C++ -*-
 * NodeStatements.h - Node linking Statements for ULAM
 *
 * Copyright (C) 2014-2016 The Regents of the University of New Mexico.
 * Copyright (C) 2014-2016 Ackleyshack LLC.
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
  \file NodeStatements.h - Node linking Statements for ULAM
  \author Elenas S. Ackley.
  \author David H. Ackley.
  \date (C) 2014-2016 All rights reserved.
  \gpl
*/


#ifndef NODESTATEMENTS_H
#define NODESTATEMENTS_H

#include "File.h"
#include "Node.h"

namespace MFM{

  class NodeStatements : public Node
  {
  public:

    NodeStatements(Node * s, CompilerState & state);
    NodeStatements(const NodeStatements& ref);
    virtual ~NodeStatements();

    virtual Node * instantiate();

    virtual void updateLineage(NNO pno);

    virtual bool findNodeNo(NNO n, Node *& foundNode);

    virtual void checkAbstractInstanceErrors();

    virtual void print(File * fp);

    virtual void printPostfix(File * fp);

    virtual bool isAConstant();

    virtual bool isFunctionCall();

    virtual bool isExplicitReferenceCast(); //only NodeCast may return true

    virtual UTI checkAndLabelType();

    virtual void countNavHzyNoutiNodes(u32& ncnt, u32& hcnt, u32& nocnt);

    virtual bool buildDefaultQuarkValue(u32& dqref);

    virtual EvalStatus eval();

    virtual void setNextNode(NodeStatements * s);

    virtual const char * getName();

    virtual const std::string prettyNodeName();

    virtual void packBitsInOrderOfDeclaration(u32& offset);

    virtual void printUnresolvedVariableDataMembers();

    virtual void printUnresolvedLocalVariables(u32 fid);

    virtual void calcMaxDepth(u32& depth, u32& maxdepth, s32 base);

    virtual void genCode(File * fp, UlamValue& uvpass);

    virtual void genCodeToStoreInto(File * fp, UlamValue& uvpass);

    virtual void genCodeExtern(File * fp, bool declOnly);

    virtual void generateUlamClassInfo(File * fp, bool declOnly, u32& dmcount);

  protected:
    Node * m_node;
    NodeStatements * m_nodeNext;

  private:


  };

}

#endif //end NODESTATEMENTS_H
