/**                                        -*- mode:C++ -*-
 * NodeInstanceof.h - Node handling the Instanceof Statement for ULAM
 *
 * Copyright (C) 2016-2017 The Regents of the University of New Mexico.
 * Copyright (C) 2016-2024 Ackleyshack LLC.
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
  \file NodeInstanceof.h - Node handling the Instanceof Statement for ULAM
  \author Elena S. Ackley.
  \author David H. Ackley.
  \date (C) 2016-2024 All rights reserved.
  \gpl
*/


#ifndef NODEINSTANCEOF_H
#define NODEINSTANCEOF_H

#include "NodeStorageof.h"

namespace MFM{

  class NodeInstanceof : public NodeStorageof
  {
  public:

    NodeInstanceof(Node * ofnode, NodeTypeDescriptor * nodetype, CompilerState & state);

    NodeInstanceof(const NodeInstanceof& ref);

    virtual ~NodeInstanceof();

    virtual Node * instantiate();

    virtual const char * getName();

    virtual const std::string prettyNodeName();

    virtual u32 getPositionOf();

    virtual FORECAST safeToCastTo(UTI newType);

    virtual UTI checkAndLabelType(Node * thisparentnode);

    virtual TBOOL checkVarUsedBeforeDeclared(u32 id, NNO declblockno);

    virtual void genCode(File * fp, UVPass& uvpass);

    virtual void genCodeToStoreInto(File * fp, UVPass& uvpass);

  protected:
    virtual UlamValue makeUlamValuePtr();

  private:
    SymbolTmpVar * m_tmpvarSymbol;
  };

}

#endif //end NODEINSTANCEOF_H
