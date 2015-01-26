/* -*- c++ -*- */
/**                                        -*- mode:C++ -*-
 * CompilerState.h - Global Compiler State for ULAM
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
  \file CompilerState.h - Global Compiler State for ULAM
  \author Elenas S. Ackley.
  \author David H. Ackley.
  \date (C) 2014-2015 All rights reserved.
  \gpl
*/

#ifndef COMPILERSTATE_H
#define COMPILERSTATE_H

#include <string>
#include <string.h>
#include <vector>
#include <map>
#include <set>
#include "itype.h"
#include "CallStack.h"
#include "Constants.h"
#include "ErrorMessageHandler.h"
#include "UEventWindow.h"
#include "File.h"
#include "NodeBlock.h"
#include "NodeCast.h"
#include "NodeConstantDef.h"
#include "NodeReturnStatement.h"
#include "NodeTypeBitsize.h"
#include "NodeSquareBracket.h"
#include "StringPool.h"
#include "SymbolClass.h"
#include "SymbolClassName.h"
#include "SymbolFunction.h"
#include "SymbolTable.h"
#include "SymbolVariable.h"
#include "Token.h"
#include "Tokenizer.h"
#include "UlamType.h"


namespace MFM{

  struct less_than_key
  {
    inline bool operator() (const UlamKeyTypeSignature key1, const UlamKeyTypeSignature key2)
    {
      if(key1.m_typeNameId < key2.m_typeNameId) return true;
      if(key1.m_typeNameId > key2.m_typeNameId) return false;
      if(key1.m_bits < key2.m_bits) return true;
      if(key1.m_bits > key2.m_bits ) return false;
      if(key1.m_arraySize < key2.m_arraySize) return true;
      if(key1.m_arraySize > key2.m_arraySize) return false;
      //if(key1.m_bits < key2.m_bits && key1.m_bits > UNKNOWNSIZE) return true;
      //if(key1.m_bits > key2.m_bits && key2.m_bits > UNKNOWNSIZE) return false;
      //if(key1.m_arraySize < key2.m_arraySize && key1.m_arraySize > UNKNOWNSIZE) return true;
      //if(key1.m_arraySize > key2.m_arraySize && key2.m_arraySize > UNKNOWNSIZE) return false;
      if(key1.m_classInstanceIdx < key2.m_classInstanceIdx) return true;
      if(key1.m_classInstanceIdx > key2.m_classInstanceIdx) return false;
      return false;
    }
  };

  class Symbol;         //forward
  class NodeBlockClass; //forward
  class SymbolTable;    //forward

  struct CompilerState
  {
    // tokenizer ptr replace by StringPool, service for deferencing strings
    // e.g., Token identifiers that are variables, path names read by SS, etc.)
    StringPool m_pool;

    // map key is the prefix id in the Locator; value is a vector of
    // stringpool id's indexed by line into the original ulam source
    // code; built by SourceStream during parsing; used for
    // documentation during code generation.
    std::map<u32,std::vector<u32>* > m_textByLinePerFilePath;
    Locator m_locOfNextLineText;

    u32 m_compileThisId;                 // the subject of this compilation; id into m_pool
    SymbolTable m_programDefST;

    NodeBlock *      m_currentBlock;     //replaces m_stackOfBlocks
    NodeBlockClass * m_classBlock;       //holds ST with function defs

    bool m_useMemberBlock;               // used during parsing member select expression
    NodeBlockClass * m_currentMemberClassBlock;

    s32 m_currentFunctionBlockDeclSize;   //used to calc framestack size for function def
    s32 m_currentFunctionBlockMaxDepth;   //framestack for function def saved in NodeBlockFunctionDefinition

    s32 m_parsingControlLoop;                // used for break/continue control statement parsing;
                                             // label num for end of loop, or 0
    bool m_parsingElementParameterVariable;  // used for static 'element' variables

    bool m_parsingConditionalAs;             // used for Conditional-As parsing
    Token m_identTokenForConditionalAs;      // used for Conditional-As parsing
    bool m_genCodingConditionalAs;   // used for Conditional-As code gen

    CallStack m_funcCallStack;    //local variables and arguments
    //UlamAtom  m_selectedAtom;   //storage for data member (static/global) variables
    UEventWindow  m_eventWindow;  //storage for 41 atoms (elements)
    CallStack m_nodeEvalStack;    //for node eval return values,
                                  //uses evalNodeProlog/Epilog; EVALRETURN storage

    ErrorMessageHandler m_err;

    std::map<UlamKeyTypeSignature, UTI, less_than_key> m_keyToaUTI;   //key -> index of ulamtype (UTI)
    std::vector<UlamKeyTypeSignature> m_indexToUlamKey;   //UTI -> ulamkey, many-to-one
    std::map<UlamKeyTypeSignature, UlamType *, less_than_key> m_definedUlamTypes;   //key -> ulamtype *

    std::map<UTI, NodeTypeBitsize *> m_unknownBitsizeSubtrees; //constant expr to resolve, and empty
    std::map<UTI, NodeSquareBracket *> m_unknownArraysizeSubtrees;  //constant expr to resolve, and empty
    std::set<NodeConstantDef *> m_nonreadyNamedConstantSubtrees; //constant expr to resolve, and empty; various scopes
    std::map<UTI, std::vector<NodeConstantDef *> > m_nonreadyClassArgSubtrees; //constant expr to resolve, and empty for a class' args.
    std::map<UlamKeyTypeSignature, u32, less_than_key> m_unknownKeyUTICounter; //track how many uti's to an "unknown" key, before delete
    std::map<UTI, std::set<UTI> > m_scalarUTItoArrayUTIs; //help update array's bitsizes when scalar's is known

    std::vector<NodeReturnStatement *> m_currentFunctionReturnNodes;   //nodes of return nodes in a function; verify type
    UTI m_currentFunctionReturnType;  //used during type labeling to check return types
    UlamValue m_currentObjPtr;        //used in eval of members: data or funcs; updated at each '.'
    UlamValue m_currentSelfPtr;       //used in eval of func calls: updated after args, becomes currentObjPtr for args

    std::vector<Symbol *> m_currentObjSymbolsForCodeGen;  //used in code generation;
    Symbol * m_currentSelfSymbolForCodeGen; //used in code generation; parallels m_currentSelf
    u32 m_currentIndentLevel;         //used in code generation: func def, blocks, control body
    s32 m_nextTmpVarNumber;           //used in code gen when a "slot index" is not available

    CompilerState();
    ~CompilerState();

    void clearAllDefinedUlamTypes();
    void clearLeftoverSubtrees();
    void clearAllLinesOfText();

    UTI makeUlamType(Token typeTok, s32 bitsize, s32 arraysize);
    UTI makeUlamType(UlamKeyTypeSignature key, ULAMTYPE utype);
    bool isDefined(UlamKeyTypeSignature key, UlamType *& foundUT);
    bool aDefinedUTI(UlamKeyTypeSignature key, UTI& foundUTI);
    UlamType * createUlamType(UlamKeyTypeSignature key, ULAMTYPE utype);
    void incrementUnknownKeyUTICounter(UlamKeyTypeSignature key);
    u32 decrementUnknownKeyUTICounter(UlamKeyTypeSignature key);
    u32 findUnknownKeyUTICounter(UlamKeyTypeSignature key);
    bool deleteUlamKeyTypeSignature(UlamKeyTypeSignature key);
    bool updateUlamKeyTypeSignatureToaUTI(UlamKeyTypeSignature oldkey, UlamKeyTypeSignature newkey);

    void constantFoldIncompleteUTI(UTI uti);
    void linkConstantExpression(UTI uti, NodeTypeBitsize * ceNode);
    bool statusUnknownBitsizeUTI();
    bool constantFoldUnknownBitsize(UTI auti, s32& bitsize);
    void linkConstantExpression(UTI uti, NodeSquareBracket * ceNode);
    bool constantFoldUnknownArraysize(UTI auti, s32& arraysize);
    bool statusUnknownArraysizeUTI();
    void linkArrayUTItoScalarUTI(UTI suti, UTI auti);
    void updatelinkedArrayUTIsWithKnownBitsize(UTI suti);

    void linkConstantExpression(NodeConstantDef * ceNode);
    bool statusNonreadyNamedConstants();

    void linkConstantExpression(UTI uti, NodeConstantDef * ceNode);
    bool constantFoldNonreadyClassArgs(UTI cuti);
    bool statusNonreadyClassArguments();

    UlamType * getUlamTypeByIndex(UTI uti);
    const std::string getUlamTypeNameBriefByIndex(UTI uti);
    const std::string getUlamTypeNameByIndex(UTI uti);

    ULAMTYPE getBaseTypeFromToken(Token tok);
    UTI getUlamTypeFromToken(Token tok, s32 typebitsize, s32 arraysize);
    bool getUlamTypeByTypedefName(u32 nameIdx, UTI & rtnType);

    /** turns array into its single element type */
    UTI getUlamTypeAsScalar(UTI utArg);
    UTI getUlamTypeOfConstant(ULAMTYPE etype);
    UTI getDefaultUlamTypeOfConstant(UTI ctype);
    bool isConstant(UTI uti);

    bool isScalar(UTI utArg);
    s32 getArraySize(UTI utArg);
    s32 getBitSize(UTI utArg);
    bool isComplete(UTI utArg);
    void setBitSize(UTI utArg, s32 total);
    void setUTISizes(UTI utArg, s32 bitsize, s32 arraysize);
    void setSizesOfNonClass(UTI utArg, s32 bitsize, s32 arraysize);

    s32 getDefaultBitSize(UTI uti);
    u32 getTotalBitSize(UTI utArg);
    s32 slotsNeeded(UTI uti);

    /** return true and the Symbol pointer in 2nd arg if found;
	search SymbolTables LIFO order; o.w. return false
    */
    bool alreadyDefinedSymbol(u32 dataindex, Symbol * & symptr);
    bool isFuncIdInClassScope(u32 dataindex, Symbol * & symptr);
    bool isIdInClassScope(u32 dataindex, Symbol * & symptr);
    void addSymbolToCurrentScope(Symbol * symptr); //ownership goes to the block


    /** searches table of class defs for specific name, by token or idx,
        returns a place-holder type if class def not yet seen */
    bool getUlamTypeByClassToken(Token ctok, UTI & rtnType);
    bool getUlamTypeByClassNameId(u32 idx, UTI & rtnType);

    /** return true and the SymbolClassName pointer in 2nd arg if found; */
    bool alreadyDefinedSymbolClassName(u32 dataindex, SymbolClassName * & symptr);

    /** return true and the SymbolClass pointer in 2nd arg if uti found; */
    bool alreadyDefinedSymbolClass(UTI uti, SymbolClass * & symptr);

    /** creates temporary class type for dataindex, returns the new Symbol pointer in 2nd arg; */
    void addIncompleteClassSymbolToProgramTable(u32 dataindex, SymbolClassName * & symptr);

    /** during type labeling, sets the ULAMCLASSTYPE and bitsize for typedefs that involved incomplete Class types */
    bool completeIncompleteClassSymbol(UTI incomplete) ;

    /** helper methods for error messaging, uses string pool */
    const std::string getTokenLocationAsString(Token * tok);
    const std::string getFullLocationAsString(Locator& loc);
    const std::string getPathFromLocator(Locator& loc);

    /** helper method, uses string pool */
    const std::string getTokenDataAsString(Token * tok);
    std::string getDataAsStringMangled(u32 dataindex);
    const std::string getTokenAsATypeName(Token tok);
    u32 getTokenAsATypeNameId(Token tok);

    bool checkFunctionReturnNodeTypes(SymbolFunction * fsym);
    void indent(File * fp);
    const char * getHiddenArgName();
    const char * getHiddenContextArgName();
    u32 getCustomArrayGetFunctionNameId();
    u32 getCustomArraySetFunctionNameId();
    const char * getIsMangledFunctionName();
    const char * getHasMangledFunctionName(UTI ltype);
    const char * getAsMangledFunctionName(UTI ltype, UTI rtype);

    s32 getNextTmpVarNumber();
    const std::string getTmpVarAsString(UTI uti, s32 num, STORAGE stg = TMPREGISTER);
    const std::string getLabelNumAsString(s32 num);

    std::string getFileNameForAClassHeader(u32 id, bool wSubDir = false);
    std::string getFileNameForThisClassHeader(bool wSubDir = false);
    std::string getFileNameForThisClassBody(bool wSubDir = false);
    std::string getFileNameForThisClassBodyNative(bool wSubDir = false);
    std::string getFileNameForThisClassCPP(bool wSubDir = false);
    std::string getFileNameForThisTypesHeader(bool wSubDir = false);
    std::string getFileNameForThisClassMain(bool wSubDir = false);

    ULAMCLASSTYPE getUlamClassForThisClass();
    UTI getUlamTypeForThisClass();

    const std::string getBitSizeTemplateString(UTI uti);

    const std::string getBitVectorLengthAsStringForCodeGen(UTI uti);

    /** returns immediate target value: extracts data from packed targets; unpacked array targets are invalid */
    UlamValue getPtrTarget(UlamValue ptr);

    /** general purpose store value (except for Ptr as value) */
    void assignValue(UlamValue lptr, UlamValue ruv);

    /** used by assignValue when rhs is a Ptr (private) */
    void assignArrayValues(UlamValue lptr, UlamValue rptr);

    /** assign pointer as value */
    void assignValuePtr(UlamValue lptr, UlamValue rptr);

    /** determinePackable: returns true
	if entire array can fit within an atom (including type);
	or if a 32-bit immediate scalar (non-Classes);
	discovered when installing a variable symbol
    */
    PACKFIT determinePackable(UTI aut);

    bool findAndSizeANodeDeclWithType(UTI argut);

    bool thisClassHasTheTestMethod();

    bool thisClassIsAQuark();

    void setupCenterSiteForTesting();

    /** used by SourceStream to build m_textByLinePerFilePath during parsing */
    void appendNextLineOfText(Locator loc, std::string textstr);

    /** used during codeGen to document the source Ulam code */
    std::string getLineOfText(Locator loc);
    std::string getLocationTextAsString(Locator nodeloc);
    void outputTextAsComment(File * fp, Locator nodeloc);

    /** for conditional as-magic */
    void saveIdentTokenForConditionalAs(Token iTok);
  };

}

#endif //end COMPILERSTATE_H
