/**                                        -*- mode:C++ -*-
 * NodeType.h - Basic Node handling of Node Type for ULAM
 *
 * Copyright (C) 2015 The Regents of the University of New Mexico.
 * Copyright (C) 2015 Ackleyshack LLC.
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
  \file NodeType.h - Basic Node handling of Node Type for ULAM
  \author Elenas S. Ackley.
  \author David H. Ackley.
  \date (C) 2015 All rights reserved.
  \gpl
*/


#ifndef NODETYPE_H
#define NODETYPE_H

#include "Node.h"
#include "NodeTypeBitsize.h"
#include "NodeSquareBracket.h"

namespace MFM{

  class NodeType : public Node
  {
  public:

    NodeType(Token typetoken, UTI auti, CompilerState & state);
    NodeType(const NodeType& ref);
    virtual ~NodeType();

    virtual Node * instantiate();

    virtual void updateLineage(NNO pno);

    virtual bool findNodeNo(NNO n, Node *& foundNode);

    virtual void printPostfix(File * f);

    virtual const char * getName();

    virtual const std::string prettyNodeName();

    void linkConstantExpressionBitsize(NodeTypeBitsize * ceForBitSize);

    void linkConstantExpressionArraysize(NodeSquareBracket * ceForArraySize);

    bool isReadyType();

    virtual UTI checkAndLabelType();

    virtual bool resolveType(UTI& rtnuti);
    bool resolveTypeBitsize(UTI auti);
    bool resolveTypeArraysize(UTI auti);

    virtual void countNavNodes(u32& cnt);

    virtual bool assignClassArgValueInStubCopy();

    virtual EvalStatus eval();

  protected:
    Token m_typeTok;
    bool m_ready;

  private:
    NodeTypeBitsize * m_unknownBitsizeSubtree;
    NodeSquareBracket * m_unknownArraysizeSubtree;

  };

} //MFM

#endif //NODETYPE_H
