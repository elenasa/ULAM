/**                                        -*- mode:C++ -*-
 * NodeIdent.h - Node handling Identifiers for ULAM
 *
 * Copyright (C) 2014-2019 The Regents of the University of New Mexico.
 * Copyright (C) 2014-2019 Ackleyshack LLC.
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
  \file NodeIdent.h - Node handling Identifiers for ULAM
  \author Elena S. Ackley.
  \author David H. Ackley.
  \date (C) 2014-2019 All rights reserved.
  \gpl
*/


#ifndef NODEIDENT_H
#define NODEIDENT_H

#include "Node.h"
#include "Token.h"
#include "SymbolVariable.h"
#include "NodeBlock.h"

namespace MFM{

  class NodeIdent : public Node
  {
  public:

    NodeIdent(const Token& tok, SymbolVariable * symptr, CompilerState & state);
    NodeIdent(const NodeIdent& ref);
    virtual ~NodeIdent();

    virtual Node * instantiate();

    virtual void printPostfix(File * fp);

    virtual const char * getName();

    virtual const std::string prettyNodeName();

    void setSymbolPtr(SymbolVariable * vsymptr);

    virtual bool getSymbolPtr(Symbol *& symptrref);

    bool getSymbolPtr(SymbolVariable *& symptrref) const;

    void setupBlockNo();

    virtual bool getStorageSymbolPtr(Symbol *& symptrref);

    virtual bool hasASymbolDataMember();

    virtual bool hasASymbolSuper();

    virtual bool hasASymbolSelf();

    virtual bool hasASymbolReference();

    virtual bool hasASymbolReferenceConstant();

    const Token& getToken() const;

    virtual bool belongsToVOWN(UTI vown);

    virtual bool isAConstant();

    virtual void setClassType(UTI cuti); //noop

    virtual FORECAST safeToCastTo(UTI newType);

    virtual UTI checkAndLabelType();

    virtual bool trimToTheElement(Node ** fromleftnode, Node *& rtnnodeptr);

    virtual EvalStatus eval();

    virtual EvalStatus evalToStoreInto();

    virtual bool installSymbolTypedef(TypeArgs& args, Symbol *& asymptr);
    virtual bool installSymbolConstantValue(TypeArgs& args, Symbol*& asymptr);
    virtual bool installSymbolModelParameterValue(TypeArgs& args, Symbol*& asymptr);
    virtual bool installSymbolVariable(TypeArgs& args,  Symbol *& asymptr);

    virtual void genCode(File * fp, UVPass& uvpass);

    virtual void genCodeToStoreInto(File * fp, UVPass& uvpass);

    virtual void genCodeReadIntoATmpVar(File * fp, UVPass & uvpass);

    virtual UlamValue makeUlamValuePtr();


  protected:

  private:
    const Token m_token;
    SymbolVariable * m_varSymbol;
    NNO m_currBlockNo;
    NodeBlock * m_currBlockPtr;

    void setBlockNo(NNO n);
    NNO getBlockNo() const;
    void setBlock(NodeBlock * ptr);
    NodeBlock * getBlock();

    bool replaceOurselves(Symbol * symptr);
    TBOOL lookagainincaseimplicitselfchanged();
    UTI specifyimplicitselfexplicitly();
    UTI checkUsedBeforeDeclared();

    SymbolVariable *  makeSymbol(UTI auti, ALT reftype, const TypeArgs& args);
    bool checkVariableTypedefSizes(TypeArgs& args, UTI auti);
    bool checkTypedefOfTypedefSizes(TypeArgs& args, UTI tduti);
    bool checkConstantTypedefSizes(TypeArgs& args, UTI tduti);
    void makeUVPassForCodeGen(UVPass& uvpass);
  };

}

#endif //end NODEIDENT_H
