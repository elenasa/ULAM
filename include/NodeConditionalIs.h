/**                                        -*- mode:C++ -*-
 * NodeConditionalIs.h - Node for handling Is Expressions for ULAM
 *
 * Copyright (C) 2014-2017 The Regents of the University of New Mexico.
 * Copyright (C) 2014-2024 Ackleyshack LLC.
 * Copyright (C) 2020-2024 The Living Computation Foundation
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
  \file NodeConditionalIs.h - Node for handling Is Expressions for ULAM
  \author Elena S. Ackley.
  \author David H. Ackley.
  \date (C) 2014-2024 All rights reserved.
  \gpl
*/


#ifndef NODECONDITIONALIS_H
#define NODECONDITIONALIS_H

#include "NodeConditional.h"

namespace MFM{

  class NodeConditionalIs : public NodeConditional
  {
  public:

    NodeConditionalIs(Node * leftNode, NodeTypeDescriptor * classType, CompilerState & state);
    NodeConditionalIs(const NodeConditionalIs& ref);
    virtual ~NodeConditionalIs();

    virtual Node * instantiate();

    virtual const char * getName();

    virtual const std::string prettyNodeName();

    virtual const std::string methodNameForCodeGen();

    virtual UTI checkAndLabelType(Node * thisparentnode);

    virtual TBOOL checkVarUsedBeforeDeclared(u32 id, NNO declblockno);

    virtual EvalStatus eval();

    virtual void genCode(File * fp, UVPass& uvpass);

    //TODO:
    //virtual void genCodeToStoreInto(File * fp, UVPass& uvpass);

  protected:

  private:
    void genCodeAtomIs(File * fp, UVPass & uvpass);
    void genCodeReferenceIs(File * fp, UVPass & uvpass);

  };

}

#endif //end NODECONDITIONALIS_H
