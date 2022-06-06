/**                                        -*- mode:C++ -*-
 * NodeVarRef.h - Node handling of Variable References for ULAM
 *
 * Copyright (C) 2015-2017 The Regents of the University of New Mexico.
 * Copyright (C) 2015-2021 Ackleyshack LLC.
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
  \file NodeVarRef.h - Node handling of Variable References for ULAM
  \author Elena S. Ackley.
  \author David H. Ackley.
  \date (C) 2015-2021 All rights reserved.
  \gpl
*/


#ifndef NODEVARREF_H
#define NODEVARREF_H

#include "NodeVarDecl.h"

namespace MFM{

  class NodeVarRef : public NodeVarDecl
  {
  public:

    NodeVarRef(SymbolVariable * sym, NodeTypeDescriptor * nodetype, CompilerState & state);

    NodeVarRef(const NodeVarRef& ref);

    virtual ~NodeVarRef();

    virtual Node * instantiate();

    virtual void updateLineage(NNO pno);

    virtual bool findNodeNo(NNO n, Node *& foundNode);

    virtual void checkAbstractInstanceErrors();

    virtual void printPostfix(File * f);

    virtual const char * getName();

    virtual const std::string prettyNodeName();

    virtual FORECAST safeToCastTo(UTI newType);

    virtual bool checkReferenceCompatibility(UTI uti, Node * parentnode);

    virtual UTI checkAndLabelType(Node * thisparentnode);

    virtual TBOOL packBitsInOrderOfDeclaration(u32& offset);

    virtual void calcMaxDepth(u32& depth, u32& maxdepth, s32 base);

    virtual void countNavHzyNoutiNodes(u32& ncnt, u32& hcnt, u32& nocnt);

    virtual EvalStatus eval();

    virtual EvalStatus evalToStoreInto();

    virtual void genCode(File * fp, UVPass& uvpass);

  protected:

    virtual void printTypeAndName(File * fp);

    virtual UlamValue makeUlamValuePtr();

  private:

  };

} //MFM

#endif //end NODEVARREF_H
