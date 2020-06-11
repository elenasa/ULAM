/**                                        -*- mode:C++ -*-
 * NodeVarRefAs.h - Node handling of Conditional-As for ULAM
 *
 * Copyright (C) 2015-2019 The Regents of the University of New Mexico.
 * Copyright (C) 2015-2019 Ackleyshack LLC.
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
  \file NodeVarRefAs.h - Node handling of Conditional-As for ULAM
  \author Elena S. Ackley.
  \author David H. Ackley.
  \date (C) 2015-2019 All rights reserved.
  \gpl
*/


#ifndef NODEVARREFAS_H
#define NODEVARREFAS_H

#include "NodeVarRef.h"

namespace MFM{

  class NodeVarRefAs : public NodeVarRef
  {
  public:

    NodeVarRefAs(SymbolVariable * sym, NodeTypeDescriptor * nodetype, CompilerState & state);

    NodeVarRefAs(const NodeVarRefAs& ref);

    virtual ~NodeVarRefAs();

    virtual Node * instantiate();

    virtual void updateLineage(NNO pno);

    virtual bool findNodeNo(NNO n, Node *& foundNode);

    virtual void printPostfix(File * f);

    virtual const char * getName();

    virtual const std::string prettyNodeName();

    virtual FORECAST safeToCastTo(UTI newType);

    virtual bool checkReferenceCompatibility(UTI uti);

    virtual UTI checkAndLabelType();

    virtual TBOOL packBitsInOrderOfDeclaration(u32& offset);

    virtual void calcMaxDepth(u32& depth, u32& maxdepth, s32 base);

    virtual void countNavHzyNoutiNodes(u32& ncnt, u32& hcnt, u32& nocnt);

    virtual EvalStatus eval();

    virtual EvalStatus evalToStoreInto();

    virtual void genCode(File * fp, UVPass& uvpass);

  protected:

  private:

    void genCodeRefAsSelf(File * fp, UVPass& uvpass);
    void makeSuperSymbolForAsBlock();
  };

} //MFM

#endif //end NODEVARREFAS_H
