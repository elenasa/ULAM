/**                                        -*- mode:C++ -*-
 * NodeList.h - Basic handling a list of nodes for ULAM
 *
 * Copyright (C) 2015-2018 The Regents of the University of New Mexico.
 * Copyright (C) 2015-2018 Ackleyshack LLC.
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
  \author Elena S. Ackley.
  \author David H. Ackley.
  \date (C) 2015-2018 All rights reserved.
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

    virtual void resetNodeLocations(Locator loc);

    virtual void print(File * fp);

    virtual void printPostfix(File * fp);

    virtual const char * getName();

    virtual const std::string prettyNodeName();

    virtual FORECAST safeToCastTo(UTI newType);

    virtual UTI checkAndLabelType();

    virtual bool foldArrayInitExpression();

    virtual void calcMaxDepth(u32& depth, u32& maxdepth, s32 base);

    virtual void countNavHzyNoutiNodes(u32& ncnt, u32& hcnt, u32& nocnt);

    virtual void printUnresolvedLocalVariables(u32 fid);

    virtual EvalStatus eval();

    virtual EvalStatus eval(u32 n);

    virtual EvalStatus evalToStoreInto(u32 n);

    virtual void addNodeToList(Node * argNode);

    u32 getNumberOfNodes() const;

    virtual bool isEmptyList() const;

    u32 getTotalSlotsNeeded();

    Node * getNodePtr (u32 n) const;

    UTI getNodeType(u32 n); //overloads Node.h

    virtual bool isAConstant();

    bool isAConstant(u32 n);

    bool isFunctionCall(u32 n);

    virtual bool isAList();

    virtual void genCode(File * fp, UVPass& uvpass);

    void genCode(File * fp, UVPass& uvpass, u32 n);

    void genCodeToStoreInto(File * fp, UVPass& uvpass, u32 n);

    virtual void genCodeConstantArrayInitialization(File * fp);

    virtual void generateBuiltinConstantClassOrArrayInitializationFunction(File * fp, bool declOnly);

    virtual bool initDataMembersConstantValue(BV8K& bvref, BV8K& bvmask);

    virtual bool buildArrayValueInitialization(BV8K& bvtmp);

    virtual bool buildClassArrayValueInitialization(BV8K& bvtmp);

  protected:
    std::vector<Node *> m_nodes;

  private:
    void clearNodeList();

  };

} //MFM

#endif //NODELIST_H
