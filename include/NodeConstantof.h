/**                                        -*- mode:C++ -*-
 * NodeInstanceof.h - Node handling the Constantof Statement for ULAM
 *
 * Copyright (C) 2021-2024 Ackleyshack LLC.
 * Copyright (C) 2021-2024 The Living Computation Foundation
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
  \file NodeConstantof.h - Node handling the Constantof Statement for ULAM
  \author Elena S. Ackley.
  \author David H. Ackley.
  \date (C) 2021-2024 All rights reserved.
  \gpl
*/


#ifndef NODECONSTANTOF_H
#define NODECONSTANTOF_H

#include "NodeInstanceof.h"

namespace MFM{

  class NodeConstantof : public NodeInstanceof
  {
  public:

    NodeConstantof(Node * ofnode, NodeTypeDescriptor * nodetype, CompilerState & state);

    NodeConstantof(const NodeConstantof& ref);

    virtual ~NodeConstantof();

    virtual Node * instantiate();

    virtual const char * getName();

    virtual const std::string prettyNodeName();

    virtual u32 getPositionOf();

    virtual bool isAConstant();

    virtual bool isAConstantClass();

    virtual bool getConstantValue(BV8K& bval);

    virtual void setClassType(UTI cuti);

    virtual bool initDataMembersConstantValue(BV8K& bvref, BV8K& bvmask);

    virtual UTI checkAndLabelType(Node * thisparentnode);

    virtual TBOOL checkVarUsedBeforeDeclared(u32 id, NNO declblockno);

  protected:
    virtual UlamValue makeUlamValuePtr();

  private:

  };

}

#endif //end NODECONSTANTOF_H
