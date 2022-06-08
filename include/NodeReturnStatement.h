/**                                        -*- mode:C++ -*-
 * NodeReturnStatement.h - Node handling the Return Statement for ULAM
 *
 * Copyright (C) 2014-2017 The Regents of the University of New Mexico.
 * Copyright (C) 2014-2021 Ackleyshack LLC.
 * Copyright (C) 2020-2021 The Living Computation Foundation
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
  \file NodeReturnStatement.h - Node handling the Return Statement for ULAM
  \author Elena S. Ackley.
  \author David H. Ackley.
  \date (C) 2014-2021  All rights reserved.
  \gpl
*/


#ifndef NODERETURN_H
#define NODERETURN_H

#include "File.h"
#include "Node.h"

namespace MFM{

  class NodeReturnStatement : public Node
  {
  public:

    NodeReturnStatement(Node * s, CompilerState & state);
    NodeReturnStatement(const NodeReturnStatement& ref);
    virtual ~NodeReturnStatement();

    virtual Node * instantiate();

    virtual void updateLineage(NNO pno);

    virtual bool exchangeKids(Node * oldnptr, Node * newnptr);

    virtual bool findNodeNo(NNO n, Node *& foundNode);

    virtual void checkAbstractInstanceErrors();

    virtual void print(File * fp);

    virtual void printPostfix(File * fp);

    virtual UTI checkAndLabelType(Node * thisparentnode);

    virtual void countNavHzyNoutiNodes(u32& ncnt, u32& hcnt, u32& nocnt);

    virtual EvalStatus eval();

    virtual EvalStatus evalToStoreInto();

    virtual const char * getName();

    virtual const std::string prettyNodeName();

    virtual void genCode(File * fp, UVPass& uvpass);

    virtual void genCodeToStoreInto(File * fp, UVPass& uvpass); //returning a reference

    virtual void calcMaxDepth(u32& depth, u32& maxdepth, s32 base);

  protected:

  private:
    Node * m_node;

    bool checkForErrorReturningRefToLocalFuncVarOnStack();

  };

}

#endif //end NODERETURNSTATEMENT_H
