/**                                        -*- mode:C++ -*-
 * NodeModelParameterDef.h - Node handling Model Parameter Definition for ULAM
 *
 * Copyright (C) 2015-2018 The Regents of the University of New Mexico.
 * Copyright (C) 2015-2021 Ackleyshack LLC.
 * Copyright (C) 2020-2021 The Living Computation Foundation
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
  \file NodeModelParameterDef.h - Node handling Model Parameter Definition for ULAM
  \author Elena S. Ackley.
  \author David H. Ackley.
  \date (C) 2015-2021 All rights reserved.
  \gpl
*/


#ifndef NODEMODELPARAMETERDEF_H
#define NODEMODELPARAMETERDEF_H

#include "NodeConstantDef.h"
#include "SymbolModelParameterValue.h"

namespace MFM{

  class NodeModelParameterDef : public NodeConstantDef
  {
  public:

    NodeModelParameterDef(SymbolModelParameterValue * symptr, NodeTypeDescriptor * nodetype, CompilerState & state);

    NodeModelParameterDef(const NodeModelParameterDef& ref);

    virtual ~NodeModelParameterDef();

    virtual Node * instantiate();

    virtual void printPostfix(File * f);

    virtual const std::string prettyNodeName();

    virtual void fixPendingArgumentNode();

    virtual UTI checkAndLabelType(Node * thisparentnode);

    virtual bool buildDefaultValue(u32 wlen, BV8K& dvref);

    virtual bool buildDefaultValueForClassConstantDefs();

    virtual UTI foldConstantExpression();

    virtual void genCode(File * fp, UVPass& uvpass);

    virtual void addMemberDescriptionToInfoMap(UTI classType, ClassMemberMap& classmembers);

  protected:
    virtual void checkForSymbol();

  private:

  };

} //MFM

#endif //NODEMODELPARAMETERDEF_H
