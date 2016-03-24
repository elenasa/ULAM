/* -*- c++ -*- */
/**                                        -*- mode:C++ -*-
 * CompilerState.h - Global Compiler State for ULAM
 *
 * Copyright (C) 2014-2016 The Regents of the University of New Mexico.
 * Copyright (C) 2014-2016 Ackleyshack LLC.
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
  \date (C) 2014-2016 All rights reserved.
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
#include "ClassContextStack.h"
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
#include "SymbolClassNameTemplate.h"
#include "SymbolFunction.h"
#include "SymbolTable.h"
#include "SymbolVariable.h"
#include "Token.h"
#include "Tokenizer.h"
#include "TypeArgs.h"
#include "UlamType.h"
#include "UlamUtil.h"


namespace MFM{

  struct less_than_key
  {
    //see operator< in UlamKeyTypeSignature
    inline bool operator() (const UlamKeyTypeSignature key1, const UlamKeyTypeSignature key2)
    {
      return (key1 < key2);
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

    SymbolTable m_programDefST; // holds SymbolClassName and SymbolClassNameTemplate

    s32 m_currentFunctionBlockDeclSize; //used to calc framestack size for function def
    s32 m_currentFunctionBlockMaxDepth; //framestack saved in NodeBlockFunctionDefinition

    s32 m_parsingControlLoop;             // used for break/continue control stmt parsing;
                                          // label num for end of loop, or 0

    bool m_gotStructuredCommentToken; // avoid testing uninitialized value
    Token m_precedingStructuredCommentToken; //for next class or parameter

    bool m_parsingConditionalAs;          // used for Conditional-As/Has parsing
    Token m_identTokenForConditionalAs;   // used for Conditional-As/Has parsing
    Token m_parsingConditionalToken;       // used for Conditional-As/Has parsing
    bool m_genCodingConditionalHas; // used for Conditional-Has code gen

    CallStack m_funcCallStack;    //local variables and arguments
    UEventWindow  m_eventWindow;  //storage for 41 atoms (elements)
    CallStack m_nodeEvalStack;    //for node eval return values,
                                  //uses evalNodeProlog/Epilog; EVALRETURN storage

    bool m_goAgainResolveLoop; //true means a node has a type that's not ready
    UTI m_pendingArgStubContext; //non-Nav helps find parentNode in case of surgery

    ErrorMessageHandler m_err;

    std::vector<UlamKeyTypeSignature> m_indexToUlamKey; //UTI->ulamkey, many-to-one
    std::map<UlamKeyTypeSignature, UlamType *, less_than_key> m_definedUlamTypes; //key->ulamtype *
    std::map<UlamKeyTypeSignature, std::set<UTI>, less_than_key> m_keyToAnyUTI; //key->set of indexes of ulamtype (UTI); tracks how many uti's to an "unknown" key, before delete

    std::set<SymbolClassName *> m_unseenClasses;

    std::vector<UTI> m_unionRootUTI; //UTI's root UTI to manage holder/aliases

    std::vector<NodeReturnStatement *> m_currentFunctionReturnNodes; //nodes of return nodes in a function; verify type
    UTI m_currentFunctionReturnType;  //used during type labeling to check return types
    UlamValue m_currentObjPtr; //used in eval of members: data or funcs; updated at each '.'
    UlamValue m_currentSelfPtr; //used in eval of func calls: updated after args,
                                // becomes currentObjPtr for args
    UlamValue m_currentAutoObjPtr; //used in eval, uses rhs type of conditional as/has:
    UTI m_currentAutoStorageType; //used in eval, uses lhs type of conditional as/has:

    std::vector<Symbol *> m_currentObjSymbolsForCodeGen;  //used in code generation;
    Symbol * m_currentSelfSymbolForCodeGen; //used in code gen; parallels m_currentSelf
    bool m_gencodingAVirtualFunctionDefinedInAQuark; //uses less efficient read/write without POS template arg

    u32 m_currentIndentLevel; //used in code generation: func def, blocks, control body
    s32 m_nextTmpVarNumber; //used in code gen when a "slot index" is not available
    NNO m_nextNodeNumber; //used to identify blocks in clone classes with unknown subtrees

    CompilerState();
    ~CompilerState();

    void clearAllDefinedUlamTypes();
    void clearAllLinesOfText();

    bool getClassNameFromFileName(std::string startstr, u32& compileThisId);

    UTI makeUlamTypeHolder();
    UTI makeUlamTypeFromHolder(UlamKeyTypeSignature newkey, ULAMTYPE utype, UTI uti);
    UTI makeUlamTypeFromHolder(UlamKeyTypeSignature oldkey, UlamKeyTypeSignature newkey, ULAMTYPE utype, UTI uti);
    SymbolClassName * makeClassFromHolder(UTI huti, Token tok);
    void cleanupExistingHolder(UTI huti, UTI newuti);
    SymbolClassName * makeAnonymousClassFromHolder(UTI cuti, Locator cloc);

    UTI makeUlamType(Token typeTok, s32 bitsize, s32 arraysize, UTI classinstanceidx, ALT reftype = ALT_NOT);
    UTI makeUlamType(UlamKeyTypeSignature key, ULAMTYPE utype);
    bool isDefined(UlamKeyTypeSignature key, UlamType *& foundUT);
    bool anyDefinedUTI(UlamKeyTypeSignature key, UTI& foundUTI);
    UlamType * createUlamType(UlamKeyTypeSignature key, ULAMTYPE utype);
    void incrementKeyToAnyUTICounter(UlamKeyTypeSignature key, UTI utarg);
    u32 decrementKeyToAnyUTICounter(UlamKeyTypeSignature key, UTI utarg);
    bool deleteUlamKeyTypeSignature(UlamKeyTypeSignature key, UTI utarg);
    bool mappedIncompleteUTI(UTI cuti, UTI auti, UTI& mappedUTI);
    UTI mapIncompleteUTIForCurrentClassInstance(UTI suti);
    void mapTypesInCurrentClass(UTI fm, UTI to);

    UlamKeyTypeSignature getUlamKeyTypeSignatureByIndex(UTI typidx);
    UlamType * getUlamTypeByIndex(UTI uti);
    const std::string getUlamTypeNameBriefByIndex(UTI uti);
    const std::string getUlamTypeNameByIndex(UTI uti);
    const std::string getEffectiveSelfMangledNameByIndex(UTI uti);

    ULAMTYPE getBaseTypeFromToken(Token tok);
    UTI getUlamTypeFromToken(Token tok, s32 typebitsize, s32 arraysize);
    UTI getUlamTypeFromToken(TypeArgs & args);

    bool getUlamTypeByTypedefName(u32 nameIdx, UTI & rtnType, UTI & rtnScalarType);

    /** turns array into its single element type */
    UTI getUlamTypeAsScalar(UTI utArg);
    /** turns a reference into its dereferenced type */
    UTI getUlamTypeAsDeref(UTI utArg);
    /** turns a regular type into its referenced type */
    UTI getUlamTypeAsRef(UTI utArg);
    UTI getUlamTypeAsRef(UTI utArg, ALT altArg);
    ULAMTYPECOMPARERESULTS isARefTypeOfUlamType(UTI refuti, UTI ofuti); //true if de-refuti is same as ofuti

    UTI getUlamTypeOfConstant(ULAMTYPE etype);
    UTI getDefaultUlamTypeOfConstant(UTI ctype);
    bool getDefaultQuark(UTI cuti, u32& dqref);

    bool isScalar(UTI utArg);
    s32 getArraySize(UTI utArg);
    s32 getBitSize(UTI utArg);
    ALT getReferenceType(UTI utArg);
    bool isReference(UTI utArg);
    bool isComplete(UTI utArg);
    bool completeAReferenceType(UTI utArg);
    bool completeAReferenceTypeWith(UTI utArg, UTI derefuti);
    bool isHolder(UTI utArg);
    bool setBitSize(UTI utArg, s32 total);
    bool setUTISizes(UTI utArg, s32 bitsize, s32 arraysize);
    void mergeClassUTI(UTI olduti, UTI cuti);
    void rekeyToReferenceUTI(ALT autoreftype, UTI auti);
    bool isARootUTI(UTI auti);
    bool findaUTIAlias(UTI auti, UTI& aliasuti);
    void updateUTIAlias(UTI auti, UTI buti);
    void updateUTIAliasForced(UTI auti, UTI buti);
    void initUTIAlias(UTI auti);

    bool setSizesOfNonClass(UTI utArg, s32 bitsize, s32 arraysize);

    s32 getDefaultBitSize(UTI uti);
    u32 getTotalBitSize(UTI utArg);
    u32 getTotalWordSize(UTI utArg);
    s32 slotsNeeded(UTI uti);
    bool isClassATemplate(UTI cuti);
    UTI isClassASubclass(UTI cuti); //returns super UTI, or Nav if no inheritance
    void resetClassSuperclass(UTI cuti, UTI superuti);
    bool isClassASubclassOf(UTI cuti, UTI superp);
    bool isClassAStub(UTI cuti);
    bool hasClassAStub(UTI cuti);
    bool isClassAQuarkUnion(UTI cuti);
    bool isClassACustomArray(UTI cuti);
    UTI getAClassCustomArrayType(UTI cuti);
    UTI getAClassCustomArrayIndexType(UTI cuti, Node * rnode, UTI& idxuti, bool& hasHazyArgs);

    /** return true and the Symbol pointer in 2nd arg if found;
	search SymbolTables LIFO order; o.w. return false
    */
    bool alreadyDefinedSymbolByAncestor(u32 dataindex, Symbol *& symptr, bool& hasHazyKin);
    bool alreadyDefinedSymbol(u32 dataindex, Symbol * & symptr, bool& hasHazyKin);
    bool isDataMemberIdInClassScope(u32 dataindex, Symbol * & symptr, bool& hasHazyKin);
    bool isFuncIdInClassScope(u32 dataindex, Symbol * & symptr, bool& hasHazyKin);
    bool isFuncIdInClassScopeNNO(NNO cnno, u32 dataindex, Symbol * & symptr, bool& hasHazyKin);
    bool isFuncIdInAClassScope(UTI cuti, u32 dataindex, Symbol * & symptr, bool& hasHazyKin);
    bool findMatchingFunctionInAncestor(UTI cuti, u32 fid, std::vector<UTI> typeVec, SymbolFunction*& fsymref, UTI& foundInAncestor);

    void addSymbolToCurrentScope(Symbol * symptr); //ownership goes to the block
    void addSymbolToCurrentMemberClassScope(Symbol * symptr); //making stuff up for member
    void replaceSymbolInCurrentScope(u32 oldid, Symbol * symptr); //same symbol, new id
    void replaceSymbolInCurrentScope(Symbol * oldsym, Symbol * newsym); //same id, new symbol
    bool takeSymbolFromCurrentScope(u32 id, Symbol *& rtnsymptr); //ownership to the caller

    /** return true and the SymbolClassName pointer in 2nd arg if found; */
    bool alreadyDefinedSymbolClassName(u32 dataindex, SymbolClassName * & symptr);

    /** return true and the SymbolClassNameTemplate pointer in 2nd arg if found AND is a template; */
    bool alreadyDefinedSymbolClassNameTemplate(u32 dataindex, SymbolClassNameTemplate * & symptr);

    /** return true and the SymbolClass pointer in 2nd arg if uti found; */
    bool alreadyDefinedSymbolClass(UTI uti, SymbolClass * & symptr);

    /** return true and the SymbolClass pointer in 2nd arg if holder with uti as key nameid found; */
    bool alreadyDefinedSymbolClassAsHolder(UTI uti, SymbolClass * & symptr);

    /** in case of typedef's in ancestor class */
    void addUnknownTypeTokenToAClassResolver(UTI cuti, Token tok, UTI huti);
    void addUnknownTypeTokenToThisClassResolver(Token tok, UTI huti);
    Token removeKnownTypeTokenFromThisClassResolver(UTI huti);
    bool hasUnknownTypeInThisClassResolver(UTI huti);
    bool statusUnknownTypeInThisClassResolver(UTI huti);

    /** creates temporary class type for dataindex, returns the new Symbol pointer in 2nd arg; */
    bool addIncompleteClassSymbolToProgramTable(Token cTok, SymbolClassName * & symptr);
    bool addIncompleteTemplateClassSymbolToProgramTable(Token cTok, SymbolClassNameTemplate * & symptr);
    UTI addStubCopyToAncestorClassTemplate(UTI stubTypeToCopy,  UTI context);

    void resetUnseenClass(SymbolClassName * cnsym, Token identTok);
    bool getUnseenClassFilenames(std::vector<std::string>& unseenFiles);

    /** during type labeling, sets ULAMCLASSTYPE for typedefs that
	involved incomplete Class types */
    bool completeIncompleteClassSymbolForTypedef(UTI incomplete) ;

    /** helper methods for error messaging, uses string pool */
    const std::string getTokenLocationAsString(Token * tok);
    const std::string getFullLocationAsString(const Locator& loc);
    const std::string getPathFromLocator(const Locator& loc);
    const std::string getFullPathFromLocator(const Locator& loc);

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
    const char * getCustomArrayGetMangledFunctionName();
    const char * getCustomArraySetMangledFunctionName();
    const char * getIsMangledFunctionName(UTI ltype);
    const char * getHasMangledFunctionName(UTI ltype);
    const char * getAsMangledFunctionName(UTI ltype, UTI rtype);
    const char * getBuildDefaultAtomFunctionName(UTI ltype);
    const char * getDefaultQuarkFunctionName();

    std::string getFileNameForAClassHeader(UTI cuti, bool wSubDir = false);
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

    /** returns ulamvalue ptr to entire atom/element from m_currentSelfPtr */
    UlamValue getAtomPtrFromSelfPtr();

    /** returns immediate target value: extracts data from packed targets;
	unpacked array targets are invalid */
    UlamValue getPtrTarget(UlamValue ptr);

    UlamValue getPtrTargetFromAbsoluteIndex(UlamValue ptr);

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

    bool thisClassHasTheTestMethod();

    bool thisClassIsAQuark();

    bool quarkHasAToIntMethod(UTI quti);

    bool classHasACustomArraySetMethod(UTI cuti);

    void setupCenterSiteForTesting();

    /** used by SourceStream to build m_textByLinePerFilePath during parsing */
    void appendNextLineOfText(Locator loc, std::string textstr);

    /** used during codeGen to document the source Ulam code */
    std::string getLineOfText(Locator loc);
    std::string getLocationTextAsString(Locator nodeloc);
    void outputTextAsComment(File * fp, Locator nodeloc);

    s32 getNextTmpVarNumber();
    const std::string getTmpVarAsString(UTI uti, s32 num, STORAGE stg = TMPREGISTER);
    const std::string getUlamRefTmpVarAsString(s32 num);
    const std::string getUlamClassTmpVarAsString(s32 num);
    const std::string getLabelNumAsString(s32 num);

    /** for conditional h/as-magic */
    void saveIdentTokenForConditionalAs(Token iTok, Token cTok);

    /** class or model parameter structured comment for MFM */
    void saveStructuredCommentToken(Token scTok);
    void clearStructuredCommentToken();
    bool getStructuredCommentToken(Token& scTok);

    /** to identify each node */
    NNO getNextNodeNo();

    Node * findNodeNoInThisClass(NNO n);
    Node * findNodeNoInAClass(NNO n, UTI cuti);
    UTI findAClassByNodeNo(NNO n);
    NodeBlockClass * getAClassBlock(UTI cuti);
    NNO getAClassBlockNo(UTI cuti);

    /** methods for context switching */
    u32 getCompileThisId();

    UTI getCompileThisIdx();

    SymbolClass * getCurrentSelfSymbolForCodeGen();

    NodeBlock * getCurrentBlock();

    NNO getCurrentBlockNo();

    NodeBlockClass * getClassBlock();

    NNO getClassBlockNo();

    bool useMemberBlock();

    NodeBlockClass * getCurrentMemberClassBlock();

    void pushClassContext(UTI idx, NodeBlock * currblock, NodeBlockClass * classblock, bool usemember, NodeBlockClass * memberblock);

    void popClassContext();

    void pushCurrentBlock(NodeBlock * currblock);

    void pushCurrentBlockAndDontUseMemberBlock(NodeBlock * currblock);

    void pushClassContextUsingMemberClassBlock(NodeBlockClass * memberblock);

    std::string getClassContextAsStringForDebugging();

    /** flag to resolving loop to go again for non-ready types (i.e. Navs) */
    void clearGoAgain();
    void setGoAgain();
    bool goAgain();

    UTI getLongUTI();
    UTI getUnsignedLongUTI();
    UTI getBigBitsUTI();

    bool isPtr(UTI puti);
    bool isAtom(UTI auti);
    bool isAtomRef(UTI auti);
    bool okUTItoContinue(UTI uti);

  private:
    ClassContextStack m_classContextStack; // the current subject of this compilation


  };
}

#endif //end COMPILERSTATE_H
