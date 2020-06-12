/**                                        -*- mode:C++ -*-
 * NodeBlock.h - Basic Node for handling Blocks for ULAM
 *
 * Copyright (C) 2014-2017 The Regents of the University of New Mexico.
 * Copyright (C) 2014-2017 Ackleyshack LLC.
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
  \file NodeBlock.h - Basic Node for handling Blocks for ULAM
  \author Elena S. Ackley.
  \author David H. Ackley.
  \date (C) 2014-2017 All rights reserved.
  \gpl
*/


#ifndef NODEBLOCK_H
#define NODEBLOCK_H

#include "File.h"
#include "NodeStatements.h"
#include "SymbolTableOfVariables.h"
#include "MapClassMemberDesc.h"

namespace MFM{

  class NodeBlock : public NodeStatements
  {
  public:

    NodeBlock(NodeBlock * prevBlockNode, CompilerState & state, NodeStatements * s = NULL);

    NodeBlock(const NodeBlock& ref);

    virtual ~NodeBlock();

    virtual Node * instantiate();

    virtual void updateLineage(NNO pno);

    virtual bool findNodeNo(NNO n, Node *& foundNode);

    virtual void checkAbstractInstanceErrors();

    virtual void print(File * fp);

    virtual void printPostfix(File * fp);

    virtual const char * getName();

    virtual const std::string prettyNodeName();

    void setLastStatementPtr(NodeStatements * laststmt);

    NodeStatements * getLastStatementPtr();

    void appendNextNode(Node * node);

    virtual UTI checkAndLabelType();

    virtual void countNavHzyNoutiNodes(u32& ncnt, u32& hcnt, u32& nocnt);

    virtual EvalStatus eval();

    virtual void calcMaxDepth(u32& depth, u32& maxdepth, s32 base);

    virtual bool isIdInScope(u32 id, Symbol * & symptrref);

    void addIdToScope(u32 id, Symbol * symptr);

    void replaceIdInScope(u32 oldid, u32 newid, Symbol * symptr);

    void replaceIdInScope(Symbol * oldsym, Symbol * newsym);

    bool removeIdFromScope(u32 id, Symbol *& rtnsymptr);

    NodeBlock * getPreviousBlockPointer();

    void setPreviousBlockPointer(NodeBlock *);

    virtual bool isAClassBlock();

    virtual bool isASwitchBlock();

    virtual u32 getNumberOfSymbolsInTable();

    virtual u32 getSizeOfSymbolsInTable();

    virtual s32 getBitSizesOfVariableSymbolsInTable();

    virtual s32 getMaxBitSizeOfVariableSymbolsInTable();

    u32 findTypedefNameIdByType(UTI uti);

    virtual void genCode(File * fp, UVPass& uvpass);

    void genModelParameterImmediateDefinitions(File * fp);

    virtual void addMemberDescriptionsToInfoMap(ClassMemberMap& classmembers);

  protected:
    SymbolTableOfVariables m_ST;

    //void genCodeDeclsForVariableDataMembers(File * fp, ULAMCLASSTYPE classtype);

  private:
    NodeBlock * m_prevBlockNode;
    NodeStatements * m_nodeEndingStmt; //ptr to last statement node while parsing.

    SymbolTable * getSymbolTablePtr(); //use with caution, esp. with inheritance

  };

}

#endif //end NODEBLOCK_H
