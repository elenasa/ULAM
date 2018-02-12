/**                                        -*- mode:C++ -*-
 * NodeListArrayInitialization.h - Handle array initialization list of nodes for ULAM
 *
 * Copyright (C) 2016-2018 The Regents of the University of New Mexico.
 * Copyright (C) 2016-2018 Ackleyshack LLC.
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
  \file NodeListArrayInitialization.h - Handle array initialization list of nodes for ULAM
  \author Elenas S. Ackley.
  \author David H. Ackley.
  \date (C) 2016-2018 All rights reserved.
  \gpl
*/


#ifndef NODELISTARRAYINITIALIZATION_H
#define NODELISTARRAYINITIALIZATION_H

#include "NodeList.h"

namespace MFM{

  struct CompilerState; //forward

  class NodeListArrayInitialization : public NodeList
  {
  public:

    NodeListArrayInitialization(CompilerState & state);

    NodeListArrayInitialization(const NodeListArrayInitialization& ref);

    virtual ~NodeListArrayInitialization();

    virtual Node * instantiate();

    virtual const char * getName();

    virtual const std::string prettyNodeName();

    virtual void printPostfix(File * fp);

    virtual bool isAConstant();

    virtual void setNodeType(UTI uti);

    virtual void setClassType(UTI cuti);

    virtual UTI checkAndLabelType();

    virtual bool foldArrayInitExpression();

    virtual FORECAST safeToCastTo(UTI newType);

    virtual EvalStatus eval();

    bool buildArrayValueInitialization(BV8K& bvtmp);

    bool buildClassArrayValueInitialization(BV8K& bvtmp);

    virtual void genCode(File * fp, UVPass& uvpass);

    void genCodeClassInitArray(File * fp, UVPass& uvpass);

    virtual bool initDataMembersConstantValue(BV8K& bvref, BV8K& bvmask);

  protected:

  private:
    bool buildArrayItemInitialValue(u32 n, u32 pos, BV8K& bvtmp);

    bool buildClassArrayItemInitialValue(u32 n, u32 pos, BV8K& bvtmp);

    bool foldInitExpression(u32 n);


    void genCodeClassInitArrayItem(File * fp, UVPass& uvpass, u32 n, u32 useitem, UVPass& uvpass2);
  };

} //MFM

#endif //NODELISTARRAYINITIALIZATION_H
