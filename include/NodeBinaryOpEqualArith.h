/**                                        -*- mode:C++ -*-
 * NodeBinaryOpEqualArith.h -  Basic Node for handling Arithmetic Operator Equal for ULAM
 *
 * Copyright (C) 2014-2017 The Regents of the University of New Mexico.
 * Copyright (C) 2014-2021 Ackleyshack LLC.
 * Copyright (C) 2020-2021 The Living Computation Foundation

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
  \file NodeBinaryOpEqualArith.h - Basic Node for handling Arithmetic Operator Equal for ULAM
  \author Elena S. Ackley.
  \author David H. Ackley.
  \date (C) 2014-2021 All rights reserved.
  \gpl
*/


#ifndef NODEBINARYOPEQUALARITH_H
#define NODEBINARYOPEQUALARITH_H

#include "NodeBinaryOpEqual.h"

namespace MFM{

  class NodeBinaryOpEqualArith : public NodeBinaryOpEqual
  {
  public:

    NodeBinaryOpEqualArith(Node * left, Node * right, CompilerState & state);

    NodeBinaryOpEqualArith(const NodeBinaryOpEqualArith& ref);

    virtual ~NodeBinaryOpEqualArith();

    virtual UTI checkAndLabelType(Node * thisparentnode);

    virtual void genCode(File * fp, UVPass& uvpass);

    virtual const std::string methodNameForCodeGen();

  protected:

    virtual bool doBinaryOperation(s32 lslot, s32 rslot, u32 slots);

  };

}

#endif //NODEBINARYOPEQUALARITH_H
