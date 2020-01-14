/**                                        -*- mode:C++ -*-
 * NodeClassId.h - Node handling the ClassId Statement for ULAM
 *
 * Copyright (C) 2020 The Regents of the University of New Mexico.
 * Copyright (C) 2020 Ackleyshack LLC.
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
  \file NodeClassid.h - Node handling the Classid Statement for ULAM
  \author Elena S. Ackley.
  \author David H. Ackley.
  \date (C) 2020 All rights reserved.
  \gpl
*/


#ifndef NODECLASSID_H
#define NODECLASSID_H

#include "NodeStorageof.h"

namespace MFM{

  class NodeClassId : public NodeStorageof
  {
  public:

    NodeClassId(Node * ofnode, NodeTypeDescriptor * nodetype, CompilerState & state);

    NodeClassId(const NodeClassId& ref);

    virtual ~NodeClassId();

    virtual Node * instantiate();

    virtual const char * getName();

    virtual void printPostfix(File * fp);

    virtual const std::string prettyNodeName();

    virtual FORECAST safeToCastTo(UTI newType);

    virtual UTI checkAndLabelType();

    virtual EvalStatus eval();

    virtual EvalStatus evalToStoreInto();

    virtual void genCode(File * fp, UVPass& uvpass);

    virtual void genCodeToStoreInto(File * fp, UVPass& uvpass);

  protected:
    virtual UlamValue makeUlamValuePtr();

  private:
    SymbolTmpVar * m_tmpvarSymbol;

    u32 getClassId();
  };

}

#endif //end NODECLASSID_H
