/**                                        -*- mode:C++ -*-
 * NodeStatementEmpty.h - Node handling the Empty Statement for ULAM
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
  \file NodeStatementEmpty.h - Node handling the Empty Statement for ULAM
  \author Elena S. Ackley.
  \author David H. Ackley.
  \date (C) 2014-2021  All rights reserved.
  \gpl
*/


#ifndef NODESTATEMENTEMPTY_H
#define NODESTATEMENTEMPTY_H

#include "Node.h"

namespace MFM{

  class NodeStatementEmpty : public Node
  {
  public:

    NodeStatementEmpty(CompilerState & state);
    NodeStatementEmpty(const NodeStatementEmpty& ref);
    virtual ~NodeStatementEmpty();

    virtual Node * instantiate();

    virtual void updateLineage(NNO pno);

    virtual bool findNodeNo(NNO n, Node *& foundNode);

    virtual void print(File * fp);

    virtual void printPostfix(File * fp);

    virtual void printOp(File * fp);

    virtual const char * getName();

    virtual const std::string prettyNodeName();

    virtual UTI checkAndLabelType(Node * thisparentnode);

    virtual EvalStatus eval();

    virtual void genCode(File * fp, UVPass& uvpass);

  protected:

  };

}

#endif //end NODESTATEMENTEMPTY_H
