/**                                        -*- mode:C++ -*-
 * NodeMemberSelectOnConstructorCall.h -  Node for handling Class Constructor Member Selection for ULAM
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
  \file NodeMemberSelectOnConstructionCall.h -  Node for handling Class Constructor Member Selection for ULAM
  \author Elena S. Ackley.
  \author David H. Ackley.
  \date (C) 2017  All rights reserved.
  \gpl
*/

#ifndef NODEMEMBERSELECTONCONSTRUCTORCALL_H
#define NODEMEMBERSELECTONCONSTRUCTORCALL_H

#include "NodeMemberSelect.h"

namespace MFM{

  class NodeMemberSelectOnConstructorCall : public NodeMemberSelect
  {
  public:

    NodeMemberSelectOnConstructorCall(Node * left, Node * right, CompilerState & state);

    NodeMemberSelectOnConstructorCall(const NodeMemberSelectOnConstructorCall& ref);

    virtual ~NodeMemberSelectOnConstructorCall();

    virtual Node * instantiate();

    virtual const char * getName();

    virtual const std::string prettyNodeName();

    virtual bool getStorageSymbolPtr(Symbol *& symptrref);

    virtual bool hasASymbolDataMember();

    virtual FORECAST safeToCastTo(UTI newType);

    virtual UTI checkAndLabelType();

    virtual bool isAConstructorFunctionCall();

    virtual bool isArrayItem();

    virtual EvalStatus evalToStoreInto();

    virtual void genCode(File * fp, UVPass& uvpass);

    virtual void genCodeToStoreInto(File * fp, UVPass& uvpass);

  protected:

    virtual bool doBinaryOperation(s32 lslot, s32 rslot, u32 slots);
    virtual bool passalongUVPass();

  private:

  };

} //MFM

#endif //NODEMEMBERSELECTONCONSTRUCTORCALL_H
