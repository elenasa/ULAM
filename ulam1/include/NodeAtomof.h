/**                                        -*- mode:C++ -*-
 * NodeAtomof.h - Node handling the Atomof Statement for ULAM
 *
 * Copyright (C) 2016 The Regents of the University of New Mexico.
 * Copyright (C) 2016 Ackleyshack LLC.
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
  \file NodeAtomof.h - Node handling the Atomof Statement for ULAM
  \author Elenas S. Ackley.
  \author David H. Ackley.
  \date (C) 2016 All rights reserved.
  \gpl
*/


#ifndef NODEATOMOF_H
#define NODEATOMOF_H

#include "File.h"
#include "Node.h"
#include "Token.h"
#include "SymbolVariable.h"
#include "NodeTypeDescriptor.h"
#include "NodeBlock.h"

namespace MFM{

  class NodeAtomof : public Node
  {
  public:

    NodeAtomof(Token tokatomof, NodeTypeDescriptor * nodetype, CompilerState & state);

    NodeAtomof(const NodeAtomof& ref);

    virtual ~NodeAtomof();

    virtual Node * instantiate();

    virtual void updateLineage(NNO pno);

    virtual bool findNodeNo(NNO n, Node *& foundNode);

    virtual void print(File * fp);

    virtual void printPostfix(File * fp);

    virtual const char * getName();

    virtual const std::string prettyNodeName();

    virtual FORECAST safeToCastTo(UTI newType);

    UTI getAtomType();

    virtual UTI checkAndLabelType();

    NNO getBlockNo() const;

    NodeBlock * getBlock();

    virtual EvalStatus eval();

    virtual EvalStatus evalToStoreInto();

    virtual void genCode(File * fp, UlamValue& uvpass);

    virtual void genCodeToStoreInto(File * fp, UlamValue& uvpass);

  protected:

  private:
    NodeTypeDescriptor * m_nodeTypeDesc;
    Token m_token;
    UTI m_atomoftype;
    SymbolVariable * m_varSymbol;
    NNO m_currBlockNo;

    UlamValue makeUlamValuePtr();
  };

}

#endif //end NODEATOMOF_H
