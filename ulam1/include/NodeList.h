/**                                        -*- mode:C++ -*-
 * NodeList.h - Basic handling a list of nodes for ULAM
 *
 * Copyright (C) 2015-2016 The Regents of the University of New Mexico.
 * Copyright (C) 2015-2016 Ackleyshack LLC.
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
  \file NodeList.h - Basic handling a list of nodes for ULAM
  \author Elenas S. Ackley.
  \author David H. Ackley.
  \date (C) 2015-2016 All rights reserved.
  \gpl
*/


#ifndef NODELIST_H
#define NODELIST_H

#include "Node.h"
#include <vector>

namespace MFM{

  struct CompilerState; //forward

  class NodeList : public Node
  {
  public:

    NodeList(CompilerState & state);

    NodeList(const NodeList& ref);

    virtual ~NodeList();

    virtual Node * instantiate();

    virtual void updateLineage(NNO pno);

    virtual bool exchangeKids(Node * oldnptr, Node * newnptr);

    bool exchangeKids(Node * oldnptr, Node * newnptr, u32 n);

    virtual bool findNodeNo(NNO n, Node *& foundNode);

    virtual void checkAbstractInstanceErrors();

    virtual void print(File * fp);

    virtual void printPostfix(File * fp);

    virtual const char * getName();

    virtual const std::string prettyNodeName();

    virtual UTI checkAndLabelType();

    virtual void calcMaxDepth(u32& depth, u32& maxdepth, s32 base);

    virtual void countNavHzyNoutiNodes(u32& ncnt, u32& hcnt, u32& nocnt);

    virtual void printUnresolvedLocalVariables(u32 fid);

    virtual EvalStatus eval();

    EvalStatus eval(u32 n);

    EvalStatus evalToStoreInto(u32 n);

    void addNodeToList(Node * argNode);

    u32 getNumberOfNodes() const;

    u32 getTotalSlotsNeeded();

    Node * getNodePtr (u32 n) const;

    UTI getNodeType(u32 n); //overloads Node.h

    bool isAConstant(u32 n);

    bool isFunctionCall(u32 n);

    void genCode(File * fp, UlamValue& uvpass, u32 n);

    void genCodeToStoreInto(File * fp, UlamValue& uvpass, u32 n);

  protected:

  private:
    std::vector<Node *> m_nodes;
  };

} //MFM

#endif //NODELIST_H
