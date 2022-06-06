/**                                        -*- mode:C++ -*-
 * NodeBinaryOpLogical.h - Basic Node for handling Binary Logical Operations for ULAM
 *
 * Copyright (C) 2014-2017 The Regents of the University of New Mexico.
 * Copyright (C) 2014-2017 Ackleyshack LLC.
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
  \file NodeBinaryOpLogical.h - Basic Node for handling Binary Logical Operations for ULAM
  \author Elena S. Ackley.
  \author David H. Ackley.
  \date (C) 2014-2017 All rights reserved.
  \gpl
*/

#ifndef NODEBINARYOPLOGICAL_H
#define NODEBINARYOPLOGICAL_H

#include <assert.h>
#include "NodeBinaryOp.h"
#include "UlamTypePrimitiveBool.h"
#include "NodeCast.h"

namespace MFM{

  class NodeBinaryOpLogical : public NodeBinaryOp
  {
  public:

    NodeBinaryOpLogical(Node * left, Node * right, CompilerState & state);
    NodeBinaryOpLogical(const NodeBinaryOpLogical& ref);
    virtual ~NodeBinaryOpLogical();

    virtual const std::string methodNameForCodeGen();

  protected:
    virtual bool doBinaryOperation(s32 lslot, s32 rslot, u32 slots);

    virtual UTI calcNodeType(UTI lt, UTI rt);
  };

}

#endif //NODEBINARYOPLOGICAL_H
