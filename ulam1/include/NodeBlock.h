/**                                        -*- mode:C++ -*-
 * NodeBlock.h - Basic Node for handling Blocks for ULAM
 *
 * Copyright (C) 2014 The Regents of the University of New Mexico.
 * Copyright (C) 2014 Ackleyshack LLC.
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
  \date (C) 2014 All rights reserved.
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

    virtual ~NodeBlock();

    virtual void print(File * fp);

    virtual void printPostfix(File * fp);

    virtual UlamType * checkAndLabelType();

    virtual EvalStatus eval();

    virtual bool isIdInScope(u32 id, Symbol * & symptrref);

    void addIdToScope(u32 id, Symbol * symptr);

    NodeBlock * getPreviousBlockPointer();

    virtual const char * getName();

    virtual const std::string prettyNodeName();

    u32 getNumberOfSymbolsInTable();
    
    u32 getSizeOfSymbolsInTable();

    u32 getBitSizesOfVariableSymbolsInTable();

    virtual void genCode(File * fp);

  protected:
    SymbolTable m_ST;

  private:
    NodeBlock * m_prevBlockNode;

  };

}

#endif //end NODEBLOCK_H
