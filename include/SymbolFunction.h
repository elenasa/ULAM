/**                                        -*- mode:C++ -*-
 * SymbolFunction.h -  Function Symbol handling for ULAM
 *
 * Copyright (C) 2014-2020 The Regents of the University of New Mexico.
 * Copyright (C) 2014-2020 Ackleyshack LLC.
 * Copyright (C) 2020 The Living Computation Foundation.
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
  \file SymbolFunction.h -  Function Symbol handling for ULAM
  \author Elena S. Ackley.
  \author David H. Ackley.
  \date (C) 2014-2020   All rights reserved.
  \gpl
*/


#ifndef SYMBOLFUNCTION_H
#define SYMBOLFUNCTION_H

#include <vector>
#include "Symbol.h"
#include "UlamTypeClass.h"
#include "Node.h"
#include "FunctionSignatureTable.h"

namespace MFM{

  class NodeBlockFunctionDefinition; //forward
  class CompilerState; //forward
  class SymbolClass; //forward

  class SymbolFunction : public Symbol
  {
  public:
    SymbolFunction(const Token& id, UTI typetoreturn, CompilerState& state);
    SymbolFunction(const SymbolFunction& sref);
    virtual ~SymbolFunction();

    virtual Symbol * clone();

    void addParameterSymbol(Symbol * argSym);
    u32 getNumberOfParameters();
    void getVectorOfParameterTypes(std::vector<UTI>& pTypesref);
    u32 getTotalParameterSlots();

    Symbol * getParameterSymbolPtr(u32 n);
    UTI getParameterType(u32 n);

    void markForVariableArgs(bool m = true);
    bool takesVariableArgs();

    virtual bool isFunction();
    void setFunctionNode(NodeBlockFunctionDefinition * func);
    NodeBlockFunctionDefinition *  getFunctionNode();
    const u32 getOrderNumber() const;

    virtual const std::string getMangledPrefix();

    const std::string getFunctionNameWithTypes();
    const std::string getMangledNameWithTypes(bool dereftypes = false);
    const std::string getMangledNameWithUTIparameters();

    bool checkParameterTypes();

    bool matchingTypesStrictly(std::vector<Node *> argNodes, bool& hasHazyArgs);
    bool matchingTypesStrictly(std::vector<UTI> argTypes, bool& hasHazyArgs);
    bool matchingTypes(std::vector<Node *> argNodes, bool& hasHazyArgs, u32& numUTmatch);

    bool checkFunctionSignatureTable(FSTable & fst);

    u32 isNativeFunctionDeclaration();

    bool isVirtualFunction();
    void setVirtualFunction();

    bool isPureVirtualFunction();
    void setPureVirtualFunction();

    bool getInsureVirtualOverrideFunction();
    void setInsureVirtualOverrideFunction();

    u32 getVirtualMethodIdx();
    void setVirtualMethodIdx(u32 idx);

    u32 getVirtualMethodOriginatingClassUTI();
    void setVirtualMethodOriginatingClassUTI(UTI uti);

    void calcMaxIndexOfVirtualFunction(SymbolClass * csym, s32& maxidx);

    bool isConstructorFunction();
    void setConstructorFunction();

    bool isDefinedInAQuark();
    void setDefinedInAQuark();

    void generateFunctionDeclaration(File * fp, bool declOnly, ULAMCLASSTYPE classtype);

    virtual void setStructuredComment();

    u32 getUlamFunctionSignatureId(); //for UlamInfo, and VTable comments

  protected:


  private:
    std::vector<Symbol *> m_parameterSymbols;  // variable or function can be an args
    NodeBlockFunctionDefinition * m_functionNode;
    const u32 m_declOrderNum; //must be unique among overloaded funcs of same name
    bool m_hasVariableArgs;
    bool m_isVirtual; //overloaded funcs may have different virtual status
    bool m_pureVirtual; //overloaded funcs may have different pure virtual status
    bool m_insureVirtualOverride;
    u32 m_virtualIdx;
    UTI m_virtualOrigUTI;
    bool m_isConstructor;
    bool m_definedinaQuark;
    u32 m_signatureid;
    void generateFunctionDeclarationVirtualTypedef(File * fp, bool declOnly, ULAMCLASSTYPE classtype);
    const std::string generateUlamFunctionSignature(); //for UlamInfo

    void initFSEntry(FSEntry& entry);
  };

}

#endif //end SYMBOLFUNCTION_H
