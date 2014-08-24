/**                                        -*- mode:C++ -*-
 * Node.h - Basic Node of Nodes for ULAM
 *
 * Copyright (C) 2014 The Regents of the University of New Mexico.
 * Copyright (C) 2014 Ackleyshack LLC.
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
  \file Node.h - Basic Node of Nodes for ULAM
  \author Elenas S. Ackley.
  \author David H. Ackley.
  \date (C) 2014 All rights reserved.
  \gpl
*/


#ifndef NODE_H
#define NODE_H

#include <sstream>
#include <stdio.h>
#include <assert.h>
#include "File.h"
#include "Locator.h"
#include "Symbol.h"
#include "UlamType.h"
#include "UlamValue.h"

namespace MFM{

  enum EVALS { EVAL_RHS, EVAL_LHS, EVAL_SIDEEFFECTS};
  enum EvalStatus {ERROR, NORMAL, RETURN, BREAK, CONTINUE};

  struct CompilerState; //forward

  class Node
  {
  public:

    Node(CompilerState & state);
    virtual ~Node() {}

    virtual void print(File * fp);
    virtual void printPostfix(File * fp) = 0;

    virtual UlamType * checkAndLabelType();

    virtual EvalStatus eval() = 0;
    virtual EvalStatus evalToStoreInto();

    UlamType * getNodeType();
    void setNodeType(UlamType * ut);

    bool isStoreIntoAble();
    void setStoreIntoAble(bool s);

    Locator getNodeLocation();
    void setNodeLocation(Locator loc);
    void printNodeLocation(File * fp);
    std::string getNodeLocationAsString();

    virtual bool getSymbolPtr(Symbol *& symptrref);
    virtual bool installSymbolTypedef(Token atok, u32 bitsize, u32 arraysize, Symbol *& asymptr);
    virtual bool installSymbolVariable(Token atok, u32 arraysize, Symbol *& asymptr);

    virtual const char * getName() = 0;

    virtual const std::string prettyNodeName() = 0;
    const std::string nodeName(const std::string& prettyFunction);

    void evalNodeProlog(u32 depth);
    void evalNodeEpilog();
    u32 makeRoomForNodeType(UlamType * type, STORAGE where = EVALRETURN);
    u32 makeRoomForSlots(u32 slots);
    void assignReturnValueToStack(UlamValue rtnUV, STORAGE where = EVALRETURN);
    void assignReturnValuePtrToStack(UlamValue rtnUVptr);

    virtual void genCode(File * fp);

  protected:

    CompilerState & m_state;  //for printing error messages with path

  private:
    bool m_storeIntoAble;
    UlamType * m_nodeUType;
    Locator m_nodeLoc;

  };

}

#endif //end NODE_H
