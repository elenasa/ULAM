/**                                        -*- mode:C++ -*-
 * Symbol.h -  Basic handling of Symbols for ULAM
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
  \file Symbol.h -  Basic handling of Symbols for ULAM
  \author Elena S. Ackley.
  \author David H. Ackley.
  \date (C) 2014-2019 All rights reserved.
  \gpl
*/


#ifndef SYMBOL_H
#define SYMBOL_H

#include "itype.h"
#include "Token.h"
#include "Constants.h"
#include "BitVector.h"

namespace MFM{

  class CompilerState; //forward

  class Symbol
  {
  public:
    Symbol(const Token& id, UTI utype, CompilerState& state);
    Symbol(const Symbol& sref);
    Symbol(const Symbol& sref, bool keepType);
    virtual ~Symbol(); //abstract

    virtual Symbol * clone() = 0;
    virtual Symbol * cloneKeepsType();

    void resetIdToken(const Token& newtok);
    void setId(u32 newid);

    u32 getId();
    Locator getLoc();
    Locator getLoc() const;
    Token * getTokPtr(); //for err msgs

    void resetUlamType(UTI newuti); //e.g. mappedUTI
    UTI getUlamTypeIdx();

    virtual u32 getPosOffset();
    virtual bool isPosOffsetReliable();

    virtual bool isFunction();
    virtual bool isFunctionParameter();
    virtual bool isTypedef();
    virtual bool isConstant();
    virtual bool isClass();
    virtual bool isTmpVarSymbol();

    void setDataMemberClass(UTI cuti);
    UTI getDataMemberClass();
    bool isDataMember();

    void setLocalsFilescopeDef(UTI locuti);
    UTI getLocalsFilescopeType();
    bool isLocalsFilescopeDef();

    virtual bool isModelParameter();

    void setAutoLocalType(const Token& cTok);
    void setAutoLocalType(ALT alt);
    ALT getAutoLocalType();
    bool isAutoLocal();

    void setIsSelf();
    bool isSelf();

    void setIsSuper();
    bool isSuper();

    NNO getBlockNoOfST();
    void setBlockNoOfST(NNO n);


    virtual const std::string getMangledName();

    static const std::string getParameterTypePrefix(bool isaclass);

    const std::string getMangledNameForParameterType();

    virtual const std::string getMangledPrefix() = 0;

    virtual void printPostfixValuesOfVariableDeclarations(File * fp, s32 slot, u32 startpos, ULAMCLASSTYPE classtype);

    virtual void setStructuredComment();

    virtual bool getStructuredComment(Token& scTok);

  protected:
    CompilerState & m_state;

    Token m_structuredCommentToken;
    bool m_gotStructuredCommentToken;

  private:
    Token m_idtok; // id to its name (string) in lexer; also in ST
    UTI m_uti; // may seem redundant, but not; from NodeVarDecl, before m_value known.
               // base type, not array type, used here (e.g. NodeBinaryOp::calcNodeType)
    UTI m_dataMemberClass;
    UTI m_localsfilescopeType;

    ALT m_autoLocalType;
    bool m_isSelf; // hidden arg symbol
    bool m_isSuper; // to modify effective self
    NNO m_stBlockNo;
  };

}

#endif //end SYMBOL_H
