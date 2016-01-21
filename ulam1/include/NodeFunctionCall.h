/**                                        -*- mode:C++ -*-
 * NodeFunctionCall.h - Basic Node for handling Function Calls for ULAM
 *
 * Copyright (C) 2014-2015 The Regents of the University of New Mexico.
 * Copyright (C) 2014-2015 Ackleyshack LLC.
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
  \file NodeFunctionCall.h - Basic Node for handling Function Calls for ULAM
  \author Elenas S. Ackley.
  \author David H. Ackley.
  \date (C) 2014-2015 All rights reserved.
  \gpl
*/


#ifndef NODEFUNCTIONCALL_H
#define NODEFUNCTIONCALL_H

#include <vector>
#include "Node.h"
#include "NodeList.h"
#include "SymbolFunction.h"
#include "Token.h"

namespace MFM{

  class NodeFunctionCall : public Node
  {
  public:

    NodeFunctionCall(Token fname, SymbolFunction * fsym, CompilerState & state);
    NodeFunctionCall(const NodeFunctionCall& ref);
    virtual ~NodeFunctionCall();

    virtual Node * instantiate();

    virtual void updateLineage(NNO pno);

    virtual bool exchangeKids(Node * oldnptr, Node * newnptr);

    virtual bool findNodeNo(NNO n, Node *& foundNode);

    virtual void printPostfix(File * fp);

    virtual const char * getName();

    virtual const std::string prettyNodeName();

    virtual FORECAST safeToCastTo(UTI newType);

    virtual UTI checkAndLabelType();

    virtual void countNavHzyNoutiNodes(u32& ncnt, u32& hcnt, u32& nocnt);

    virtual void calcMaxDepth(u32& depth, u32& maxdepth, s32 base);

    virtual bool isFunctionCall();

    virtual EvalStatus eval();

    virtual EvalStatus evalToStoreInto();

    void addArgument(Node * n);

    u32 getNumberOfArguments();

    virtual bool getSymbolPtr(Symbol *& symptrref);

    virtual void genCode(File * fp, UlamValue& uvpass);

    virtual void genCodeToStoreInto(File * fp, UlamValue& uvpass);

    virtual void genCodeReadIntoATmpVar(File * fp, UlamValue & uvpass);

  protected:
    //helper methods override Node read/write
    virtual void genMemberNameOfMethod(File * fp);

    virtual void genModelParameterMemberNameOfMethod(File * fp, s32 epi);

    virtual std::string genModelParameterHiddenArgs(s32 epi);

    virtual void genLocalMemberNameOfMethod(File * fp);

  private:

    Token m_functionNameTok;
    SymbolFunction * m_funcSymbol;
    NodeList * m_argumentNodes;

    void genCodeIntoABitValue(File * fp, UlamValue& uvpass);
    void genCodeVirtualFunctionCall(File * fp, UlamValue & uvpass);
    std::string genHiddenArgs();
    std::string genRestOfFunctionArgs(File * fp, UlamValue & uvpass);
    void genCodeReferenceArg(File * fp, UlamValue & uvpass, u32 n);
    std::string genStorageType(); //for VTable entry
  };

}

#endif //end NODEFUNCTIONCALL_H
