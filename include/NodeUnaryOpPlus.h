/**                                        -*- mode:C++ -*-
 * NodeUnaryOpPlus.h -  Node handling Unary Plus Operator for ULAM
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
  \file NodeUnaryOpPlus.h -  Node handling Unary Plus Operator for ULAM
  \author Elena S. Ackley.
  \author David H. Ackley.
  \date (C) 2014-2017   All rights reserved.
  \gpl
*/


#ifndef NODEUNARYOPPLUS_H
#define NODEUNARYOPPLUS_H

#include "NodeUnaryOp.h"

namespace MFM{

  class NodeUnaryOpPlus : public NodeUnaryOp
  {
  public:

    NodeUnaryOpPlus(Node * n, CompilerState & state);

    NodeUnaryOpPlus(const NodeUnaryOpPlus& ref);

    virtual ~NodeUnaryOpPlus();

    virtual Node * instantiate();

    virtual const char * getName();

    virtual const std::string prettyNodeName();

    virtual void genCode(File * fp, UVPass& uvpass);

    virtual void genCodeToStoreInto(File * fp, UVPass& uvpass);

  protected:
    virtual UTI calcNodeType(UTI uti);

    virtual UlamValue makeImmediateUnaryOp(UTI type, u32 data, u32 len);
    virtual UlamValue makeImmediateLongUnaryOp(UTI type, u64 data, u32 len);

  private:

  };

} //MFM

#endif //NODEUNARYOPPLUS_H
