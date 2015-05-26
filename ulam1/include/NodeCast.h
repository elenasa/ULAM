/**                                        -*- mode:C++ -*-
 * NodeCast.h - Basic Node for handling Type Casting for ULAM
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
  \file NodeCast.h - Basic Node for handling Type Casting for ULAM
  \author Elenas S. Ackley.
  \author David H. Ackley.
  \date (C) 2014-2015 All rights reserved.
  \gpl
*/


#ifndef NODECAST_H
#define NODECAST_H

#include "NodeUnaryOp.h"
#include "NodeTypeDescriptor.h"

namespace MFM{

  class NodeCast : public NodeUnaryOp
  {
  public:

    NodeCast(Node * n, UTI typeToBe, NodeTypeDescriptor * nodetype, CompilerState & state);

    NodeCast(const NodeCast& ref);

    virtual ~NodeCast();

    virtual Node * instantiate();

    virtual void updateLineage(NNO pno);

    virtual bool findNodeNo(NNO n, Node *& foundNode);

    virtual const char * getName();

    virtual const std::string prettyNodeName();

    void setExplicitCast();
    bool isExplicitCast();

    virtual bool isReadyConstant();

    virtual UTI checkAndLabelType();

    virtual void countNavNodes(u32& cnt);

    virtual EvalStatus eval();

    virtual void genCode(File * fp, UlamValue& uvpass);
    virtual void genCodeToStoreInto(File * fp, UlamValue& uvpass);

    virtual void genCodeReadIntoATmpVar(File * fp, UlamValue& uvpass);
    virtual void genCodeWriteFromATmpVar(File * fp, UlamValue& luvpass, UlamValue& ruvpass);

  protected:
    virtual UlamValue makeImmediateUnaryOp(UTI type, u32 data, u32 len); //noop

  private:
    bool m_explicit;   // requested by user (not automatic)
    NodeTypeDescriptor * m_nodeTypeDesc; //can be NULL

    bool needsACast(); // trying to avoid extraneous casting.

    void genCodeCastAtomAndElement(File * fp, UlamValue & uvpass);
    void genCodeCastAtomAndQuark(File * fp, UlamValue & uvpass);
  };

}

#endif //end NODECAST_H
