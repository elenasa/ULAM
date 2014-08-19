/* -*- c++ -*- */
/**                                        -*- mode:C++ -*-
 * CompilerState.h - Global Compiler State for ULAM
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
  \file CompilerState.h - Global Compiler State for ULAM
  \author Elenas S. Ackley.
  \author David H. Ackley.
  \date (C) 2014 All rights reserved.
  \gpl
*/

#ifndef COMPILERSTATE_H
#define COMPILERSTATE_H

#include <string>
#include <string.h>
#include <vector>
#include <map>
#include "itype.h"
#include "CallStack.h"
#include "ErrorMessageHandler.h"
#include "File.h"
#include "NodeBlock.h"
#include "NodeCast.h"
#include "NodeReturnStatement.h"
#include "Token.h"
#include "Tokenizer.h"
#include "StringPool.h"
#include "SymbolFunction.h"
#include "SymbolTable.h"
#include "UlamAtom.h"
#include "UlamType.h"


namespace MFM{

  struct less_than_key
  {
    inline bool operator() (const UlamKeyTypeSignature key1, const UlamKeyTypeSignature key2)
    { 
      //if(strcmp(key1.m_typeName, key2.m_typeName) < 0) return true;
      //if(strcmp(key1.m_typeName, key2.m_typeName) > 0) return false;  
      if(key1.m_typeNameId < key2.m_typeNameId) return true;
      if(key1.m_typeNameId > key2.m_typeNameId) return false;  
      if(key1.m_bits < key2.m_bits) return true;
      if(key1.m_bits > key2.m_bits) return false;
      if(key1.m_arraySize < key2.m_arraySize) return true;
      if(key1.m_arraySize > key2.m_arraySize) return false;
      return false; 
    }
  };

  class Symbol;         //forward
  class NodeBlockClass; //forward

  struct CompilerState
  {
    // tokenizer ptr replace by StringPool, service for deferencing strings 
    // e.g., Token identifiers that are variables, path names read by SS, etc.)
    StringPool m_pool;

    NodeBlock * m_currentBlock;     //replaces m_stackOfBlocks
    NodeBlockClass * m_classBlock;  //holds ST with function defs

    s32 m_currentFunctionBlockDeclSize;   //used to calc framestack size for function def
    s32 m_currentFunctionBlockMaxDepth;   //framestack for function def saved in NodeBlockFunctionDefinition

    CallStack m_funcCallStack;    //local variables and arguments
    UlamAtom m_selectedAtom;      //storage for data member (static/global) variables
    CallStack m_nodeEvalStack;    //for node eval return values, 
                                  //uses evalNodeProlog/Epilog; EVALRETURN storage

    ErrorMessageHandler m_err;

    std::vector<UlamType *> m_indexToUlamType;   //ulamtype ptr by index
    std::map<UlamKeyTypeSignature, UTI, less_than_key> m_definedUlamTypes;   //key -> index of ulamtype (UTI) 

    std::vector<NodeReturnStatement *> m_currentFunctionReturnNodes;   //nodes of return nodes in a function; verify type 
    UlamType * m_currentFunctionReturnType;

    u32 m_currentIndentLevel;    //for code generation: func def, blocks, control body

    CompilerState();
    ~CompilerState();

    void clearAllDefinedUlamTypes();

    UlamType * makeUlamType(Token typeTok, u32 bitsize, u32 arraysize);
    UTI makeUlamType(UlamKeyTypeSignature key, ULAMTYPE utype);
    bool isDefined(UlamKeyTypeSignature key, UTI& foundUTI);
    UlamType * createUlamType(UlamKeyTypeSignature key, UTI uti, ULAMTYPE utype);

    UlamType * getUlamTypeByIndex(UTI uti);
    const std::string getUlamTypeNameByIndex(UTI uti);
    UTI getUlamTypeIndex(UlamType * ut);

    ULAMTYPE getBaseTypeFromToken(Token tok);
    bool getUlamTypeByTypedefName(const char * name, UlamType* & rtnType);

    UlamType * getUlamTypeAsScalar(UlamType * utArg); //turns array into its single element type

    /** return true and the Symbol pointer in 2nd arg if found;
	search SymbolTables LIFO order; o.w. return false
    */
    bool alreadyDefinedSymbol(u32 dataindex, Symbol * & symptr);
    void addSymbolToCurrentScope(Symbol * symptr); //ownership goes to the block


    /** helper methods for error messaging, uses string pool */
    const std::string getTokenLocationAsString(Token * tok);
    const std::string getFullLocationAsString(Locator& loc);
    const std::string getPathFromLocator(Locator& loc);

    /** helper method, uses string pool */
    const std::string getDataAsString(Token * tok);
    const std::string getTokenAsATypeName(Token tok);
    u32 getTokenAsATypeNameId(Token tok);

    bool checkFunctionReturnNodeTypes(SymbolFunction * fsym);
    void indent(File * fp);
  };
  
}

#endif //end COMPILERSTATE_H
