/**                                        -*- mode:C++ -*-
 * NodeStorageof.h - Basic Node handling the Storageof Statement for ULAM
 *
 * Copyright (C) 2016-2017 The Regents of the University of New Mexico.
 * Copyright (C) 2016-2017 Ackleyshack LLC.
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
  \file NodeStorageof.h - Basic Node handling the Storageof Statement for ULAM
  \author Elena S. Ackley.
  \author David H. Ackley.
  \date (C) 2016-2017 All rights reserved.
  \gpl
*/


#ifndef NODESTORAGEOF_H
#define NODESTORAGEOF_H

#include "File.h"
#include "Node.h"
#include "Token.h"
#include "SymbolVariable.h"
#include "NodeTypeDescriptor.h"
#include "NodeBlock.h"

namespace MFM{

  class NodeStorageof : public Node
  {
  public:

    NodeStorageof(Node * ofnode, NodeTypeDescriptor * nodetype, CompilerState & state);

    NodeStorageof(const NodeStorageof& ref);

    virtual ~NodeStorageof();

    virtual void updateLineage(NNO pno);

    virtual bool exchangeKids(Node * oldnptr, Node * newnptr);

    virtual bool findNodeNo(NNO n, Node *& foundNode);

    virtual void print(File * fp);

    virtual void printPostfix(File * fp);

    virtual const char * getName();

    virtual const std::string prettyNodeName();

    virtual FORECAST safeToCastTo(UTI newType);

    virtual UTI checkAndLabelType();

    virtual EvalStatus eval();

    virtual EvalStatus evalToStoreInto();

    virtual void genCode(File * fp, UVPass& uvpass) = 0;

    virtual void genCodeToStoreInto(File * fp, UVPass& uvpass) = 0;

  protected:

    Node * m_nodeOf;

    UTI getOfType();
    void setOfType(UTI oftyp);

    virtual UlamValue makeUlamValuePtr() = 0;

  private:

    UTI m_oftype;
    NodeTypeDescriptor * m_nodeTypeDesc;
  };

}

#endif //end NODESTORAGEOF_H
