/**                                        -*- mode:C++ -*-
 * NodeListEmpty.h - Empty List of nodes for ULAM
 *
 * Copyright (C) 2018 The Regents of the University of New Mexico.
 * Copyright (C) 2018 Ackleyshack LLC.
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
  \file NodeListEmpty.h - Empty List of nodes for ULAM
  \author Elena S. Ackley.
  \author David H. Ackley.
  \date (C) 2018 All rights reserved.
  \gpl
*/


#ifndef NODELISTEMPTY_H
#define NODELISTEMPTY_H

#include "NodeList.h"

namespace MFM{

  class NodeListEmpty : public NodeList
  {
  public:

    NodeListEmpty(CompilerState & state);

    NodeListEmpty(const NodeListEmpty& ref);

    virtual ~NodeListEmpty();

    virtual Node * instantiate();

    virtual void print(File * fp);

    virtual void printPostfix(File * fp);

    virtual const char * getName();

    virtual const std::string prettyNodeName();

    virtual bool isEmptyList() const;

    virtual FORECAST safeToCastTo(UTI newType);

    virtual UTI checkAndLabelType();

    virtual EvalStatus evalToStoreInto(u32 n);

    virtual void addNodeToList(Node * argNode);

    virtual void printUnresolvedLocalVariables(u32 fid);

    virtual void calcMaxDepth(u32& depth, u32& maxdepth, s32 base);

    virtual bool isAConstant();

    virtual UTI foldConstantExpression();

    virtual UTI constantFold();

    virtual void genCode(File * fp, UVPass& uvpass);

    virtual void genCodeConstantArrayInitialization(File * fp);

    virtual void generateBuiltinConstantClassOrArrayInitializationFunction(File * fp, bool declOnly);

    virtual bool initDataMembersConstantValue(BV8K& bvref, BV8K& bvmask);

    virtual bool buildArrayValueInitialization(BV8K& bvtmp);

    virtual bool buildClassArrayValueInitialization(BV8K& bvtmp);

  protected:

  private:

  };

} //MFM

#endif //NODELISTEMPTY_H
