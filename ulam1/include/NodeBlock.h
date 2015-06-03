/**                                        -*- mode:C++ -*-
 * NodeBlock.h - Basic Node for handling Blocks for ULAM
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
  \file NodeBlock.h - Basic Node for handling Blocks for ULAM
  \author Elenas S. Ackley.
  \author David H. Ackley.
  \date (C) 2014-2015 All rights reserved.
  \gpl
*/


#ifndef NODEBLOCK_H
#define NODEBLOCK_H

#include "File.h"
#include "NodeStatements.h"
#include "SymbolTable.h"

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

    virtual void print(File * fp);

    virtual void printPostfix(File * fp);

    virtual const char * getName();

    virtual const std::string prettyNodeName();

    virtual UTI checkAndLabelType();

    virtual void countNavNodes(u32& cnt);

    virtual EvalStatus eval();

    virtual void calcMaxDepth(u32& depth, u32& maxdepth, s32 base);

    virtual bool isIdInScope(u32 id, Symbol * & symptrref);

    void addIdToScope(u32 id, Symbol * symptr);

    void replaceIdInScope(u32 oldid, u32 newid, Symbol * symptr);

    void replaceIdInScope(Symbol * oldsym, Symbol * newsym);

    bool removeIdFromScope(u32 id, Symbol *& rtnsymptr);

    void removeAllSymbolsFromScope();

    //bool mergeAllSymbolsFromScopeIntoTable(NodeBlock * toBlocksTable);

    NodeBlock * getPreviousBlockPointer();

    void setPreviousBlockPointer(NodeBlock *);

    u32 getNumberOfSymbolsInTable();

    u32 getSizeOfSymbolsInTable();

    s32 getBitSizesOfVariableSymbolsInTable();

    s32 getMaxBitSizeOfVariableSymbolsInTable();

    s32 findUlamTypeInTable(UTI utype);

    SymbolTable * getSymbolTablePtr(); //used for print postfix

    virtual void genCode(File * fp, UlamValue& uvpass);

  protected:
    SymbolTable m_ST;

    void genCodeDeclsForVariableDataMembers(File * fp, ULAMCLASSTYPE classtype);

    virtual void generateCodeForBuiltInClassFunctions(File * fp, bool declOnly, ULAMCLASSTYPE classtype);

  private:
    NodeBlock * m_prevBlockNode;

  };

}

#endif //end NODEBLOCK_H
