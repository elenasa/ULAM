/**                                        -*- mode:C++ -*-
 * NodeBinaryOpEqualArithPreIncr.h - Node for handling Plus Equal for ULAM
 *
 * Copyright (C) 2017 The Regents of the University of New Mexico.
 * Copyright (C) 2017 Ackleyshack LLC.
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
  \file NodeBinaryOpEqualArithPreIncr.h - Node for handling Plus Equal for ULAM
  \author Elena S. Ackley.
  \author David H. Ackley.
  \date (C) 2017 All rights reserved.
  \gpl
*/


#ifndef NODEBINARYOPEQUALARITHPREINCR_H
#define NODEBINARYOPEQUALARITHPREINCR_H

#include "NodeBinaryOpEqualArith.h"

namespace MFM{

  class NodeBinaryOpEqualArithPreIncr : public NodeBinaryOpEqualArith
  {
  public:

    NodeBinaryOpEqualArithPreIncr(Node * left, Node * right, CompilerState & state);
    NodeBinaryOpEqualArithPreIncr(const NodeBinaryOpEqualArithPreIncr& ref);
    virtual ~NodeBinaryOpEqualArithPreIncr();

    virtual Node * instantiate();

    virtual const char * getName();

    virtual const std::string prettyNodeName();

    virtual const std::string methodNameForCodeGen();

  protected:
    virtual UlamValue makeImmediateBinaryOp(UTI type, u32 ldata, u32 rdata, u32 len);
    virtual UlamValue makeImmediateLongBinaryOp(UTI type, u64 ldata, u64 rdata, u32 len);
    virtual void appendBinaryOp(UlamValue& refUV, u32 ldata, u32 rdata, u32 pos, u32 len);

    virtual Node * buildOperatorOverloadFuncCallNode(bool& hazyArg);

  private:

  };

}

#endif //end NODEBINARYOPEQUALARITHPREINCR_H
