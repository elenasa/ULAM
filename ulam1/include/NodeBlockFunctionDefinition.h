/**                                        -*- mode:C++ -*-
 * NodeBlockFunctionDefinition.h - Node for handling Function Definitions for ULAM
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
  \file NodeBlockFunctionDefinition.h - Node for handling Function Definitions for ULAM
  \author Elenas S. Ackley.
  \author David H. Ackley.
  \date (C) 2014-2015 All rights reserved.
  \gpl
*/


#ifndef NODEBLOCKFUNCTIONDEFINITION_H
#define NODEBLOCKFUNCTIONDEFINITION_H

#include "File.h"
#include "NodeBlock.h"
#include "NodeList.h"
#include "NodeTypeDescriptor.h"
#include "SymbolFunction.h"

namespace MFM {

class NodeBlockFunctionDefinition : public NodeBlock
  {
  public:

    NodeBlockFunctionDefinition(SymbolFunction * fsym, NodeBlock * prevBlockNode, NodeTypeDescriptor* nodetype, CompilerState & state, NodeStatements * s = NULL);

    NodeBlockFunctionDefinition(const NodeBlockFunctionDefinition& ref);

    virtual ~NodeBlockFunctionDefinition();

    virtual Node * instantiate();

    virtual void updateLineage(NNO pno);

    virtual bool findNodeNo(NNO n, Node *& foundNode);

    virtual void checkAbstractInstanceErrors();

    virtual void setNodeLocation(Locator loc);

    virtual void print(File * fp);

    virtual void printPostfix(File * fp);

    virtual UTI checkAndLabelType();

    bool checkParameterNodeTypes();

    void addParameterNode(Node * nodeArg);

    virtual void countNavHzyNoutiNodes(u32& ncnt, u32& hcnt, u32& nocnt);

    virtual EvalStatus eval();

    virtual const char * getName();

    virtual const std::string prettyNodeName();

    void setDefinition();
    bool isDefinition();

    // for framestack
    void setMaxDepth(u32 depth);
    u32 getMaxDepth();
    virtual void calcMaxDepth(u32& depth, u32& maxdepth, s32 base);

    // for keyword 'native'; has empty block (i.e. not defined in Ulam); eval skipped
    void setNative();
    bool isNative();

    SymbolFunction * getFuncSymbolPtr();
    void setFuncSymbolPtr(SymbolFunction * fsymptr); //during instantiation

    virtual void genCode(File * fp, UlamValue& uvpass);

  protected:


  private:
    SymbolFunction * m_funcSymbol;
    bool m_isDefinition;
    u32 m_maxDepth;
    bool m_native;  //false by default, requires keyword
    NodeList * m_nodeParameterList; //variable or function can be an args
    NodeTypeDescriptor * m_nodeTypeDesc; //return type
  };

}

#endif //NODEBLOCKFUNCTIONDEFINITION_H
