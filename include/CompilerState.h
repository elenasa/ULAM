/* -*- c++ -*- */
/**                                        -*- mode:C++ -*-
 * CompilerState.h - Global Compiler State for ULAM
 *
 * Copyright (C) 2014-2018 The Regents of the University of New Mexico.
 * Copyright (C) 2014-2018 Ackleyshack LLC.
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
  \date (C) 2014-2018 All rights reserved.
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
#include "NodeBlockContext.h"
#include "NodeBlockLocals.h"
#include "NodeCast.h"
#include "NodeConstantDef.h"
#include "NodeReturnStatement.h"
#include "NodeTypeBitsize.h"
#include "NodeSquareBracket.h"
#include "ParsingLoopsSwitchStack.h"
#include "StringPool.h"
#include "StringPoolUser.h"
#include "SymbolClass.h"
#include "SymbolClassName.h"
#include "SymbolClassNameTemplate.h"
#include "SymbolFunction.h"
#include "SymbolTableOfClasses.h"
#include "Token.h"
#include "Tokenizer.h"
#include "TypeArgs.h"
#include "UlamType.h"
#include "UlamUtil.h"


namespace MFM{

#define _USE_GCNL_TAG
#ifndef _USE_GCNL_TAG
#define GCNL (fp->write("\n"))
#else
#define GCNL (fp->write_tagged_end( __FILE__ , __LINE__))
#endif


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
  class SymbolTableOfClasses;    //forward

  struct CompilerState
  {
    // tokenizer ptr replace by StringPool, service for deferencing strings
    // e.g., Token identifiers that are variables, path names read by SS, etc.)
    StringPool m_pool;

    StringPoolUser m_tokenupool; //for double quoted strings Tokens only

    StringPoolUser m_upool; //for double quoted strings only (global ulam-4)

    // map key is the prefix id in the Locator; value is a vector of
    // stringpool id's indexed by line into the original ulam source
    // code; built by SourceStream during parsing; used for
    // documentation during code generation.
    std::map<u32,std::vector<u32>* > m_textByLinePerFilePath;
    Locator m_locOfNextLineText;
    bool m_linesForDebug;

    SymbolTableOfClasses m_programDefST; // holds SymbolClassName and SymbolClassNameTemplate

    std::map<u32, NodeBlockLocals *> m_localsPerFilePath; //holds block of local constants and typedefs
    bool m_parsingLocalDef; //used for populating m_localsPerFilePath
    Token m_currentLocalDefToken; //used to identify current file path when m_parsingLocalDef is true

    SYMBOLTYPEFLAG m_parsingVariableSymbolTypeFlag;

    // used for break/continue stmt parsing; label num for end of loop, or 0
    ParsingLoopsSwitchStack m_parsingControlLoopsSwitchStack;

    bool m_gotStructuredCommentToken; // avoid testing uninitialized value
    Token m_precedingStructuredCommentToken; //for next class or parameter

    bool m_parsingConditionalAs;          // used for Conditional-As parsing
    Token m_identTokenForConditionalAs;   // used for Conditional-As parsing
    Token m_parsingConditionalToken;       // used for Conditional-As parsing

    CallStack m_funcCallStack;    //local variables and arguments
    UEventWindow  m_eventWindow;  //storage for 41 atoms (elements)
    CallStack m_nodeEvalStack;    //for node eval return values,
                                  //uses evalNodeProlog/Epilog; EVALRETURN storage
    CallStack m_constantStack;    //for constant arrays (non-function variables); CNSTSTACK

    bool m_goAgainResolveLoop; //true means a node has a type that's not ready
    UTI m_pendingArgStubContext; //non-Nouti helps find parentNode in case of surgery
    UTI m_pendingArgTypeStubContext; //non-Nouti helps find parentNode in case of surgery

    ErrorMessageHandler m_err;

    std::vector<UlamKeyTypeSignature> m_indexToUlamKey; //UTI->ulamkey, many-to-one
    std::map<UlamKeyTypeSignature, UlamType *, less_than_key> m_definedUlamTypes; //key->ulamtype *
    std::map<UlamKeyTypeSignature, std::set<UTI>, less_than_key> m_keyToAnyUTI; //key->set of indexes of ulamtype (UTI); tracks how many uti's to an "unknown" key, before delete

    std::set<u32> m_unseenClasses; //name id of possible classes (no longer SymbolClassName *)

    std::vector<UTI> m_unionRootUTI; //UTI's root UTI to manage holder/aliases

    std::vector<NodeReturnStatement *> m_currentFunctionReturnNodes; //nodes of return nodes in a function; verify type
    UTI m_currentFunctionReturnType;  //used during type labeling to check return types
    UlamValue m_currentObjPtr; //used in eval of members: data or funcs; updated at each '.'
    UlamValue m_currentSelfPtr; //used in eval of func calls: updated after args,
                                // becomes currentObjPtr for args
    UlamValue m_currentAutoObjPtr; //used in eval, uses rhs type of conditional as:
    UTI m_currentAutoStorageType; //used in eval, uses lhs type of conditional as:

    std::vector<Symbol *> m_currentObjSymbolsForCodeGen;  //used in code generation;
    Symbol * m_currentSelfSymbolForCodeGen; //used in code gen; parallels m_currentSelf
    bool m_gencodingAVirtualFunctionDefinedInAQuark; //uses less efficient read/write without POS template arg

    u32 m_currentIndentLevel; //used in code generation: func def, blocks, control body
    s32 m_nextTmpVarNumber; //used in code gen when a "slot index" is not available
    NNO m_nextNodeNumber; //used to identify blocks in clone classes with unknown subtrees

    UTI m_urSelfUTI; //original ancestor of all classes
    UTI m_emptyUTI; //the Empty class

    CompilerState();
    ~CompilerState();

    void clearAllDefinedUlamTypes();
    void clearAllLinesOfText();
    void clearAllLocalsPerFilePath();
    void clearCurrentObjSymbolsForCodeGen();

    bool getClassNameFromFileName(std::string startstr, u32& compileThisId);

    void getTargetDescriptorsForLocalsFilescopes(TargetMap & localstargets);
    void getMembersDescriptionsForLocalsFilescopes(ClassMemberMap & localsmembers);

    UTI makeUlamTypeHolder();
    UTI makeUlamTypeFromHolder(UlamKeyTypeSignature newkey, ULAMTYPE utype, UTI uti, ULAMCLASSTYPE classtype);
    UTI makeUlamTypeFromHolder(UlamKeyTypeSignature oldkey, UlamKeyTypeSignature newkey, ULAMTYPE utype, UTI uti, ULAMCLASSTYPE classtype);
    void cleanupExistingHolder(UTI huti, UTI newuti);
    SymbolClassName * makeAnonymousClassFromHolder(UTI cuti, Locator cloc);

    UTI makeUlamType(const Token& typeTok, s32 bitsize, s32 arraysize, UTI classinstanceidx, ALT reftype = ALT_NOT, ULAMCLASSTYPE classtype = UC_NOTACLASS);
    UTI makeUlamType(UlamKeyTypeSignature key, ULAMTYPE utype, ULAMCLASSTYPE classtype);
    void addCompleteUlamTypeToThisContextSet(UTI uti);
    void addCompleteUlamTypeToContextBlockSet(UTI uti, NodeBlockContext * contextblock);
    bool isOtherClassInThisContext(UTI suti);
    bool isAStringDataMemberInClass(UTI cuti);
    bool isDefined(UlamKeyTypeSignature key, UlamType *& foundUT);
    bool anyDefinedUTI(UlamKeyTypeSignature key, UTI& foundUTI);
    UlamType * createUlamType(UlamKeyTypeSignature key, ULAMTYPE utype, ULAMCLASSTYPE classtype);
    void incrementKeyToAnyUTICounter(UlamKeyTypeSignature key, UTI utarg);
    u32 decrementKeyToAnyUTICounter(UlamKeyTypeSignature key, UTI utarg);
    bool deleteUlamKeyTypeSignature(UlamKeyTypeSignature key, UTI utarg);
    bool replaceUlamTypeForUpdatedClassType(UlamKeyTypeSignature key, ULAMTYPE etype, ULAMCLASSTYPE classtype, bool isCArray);
    bool mappedIncompleteUTI(UTI cuti, UTI auti, UTI& mappedUTI);
    UTI mapIncompleteUTIForCurrentClassInstance(UTI suti, Locator loc);
    UTI mapIncompleteUTIForAClassInstance(UTI cuti, UTI suti, Locator loc);
    void mapTypesInCurrentClass(UTI fm, UTI to);

    UlamKeyTypeSignature getUlamKeyTypeSignatureByIndex(UTI typidx);
    u32 getUlamTypeNameIdByIndex(UTI typidx);
    UlamType * getUlamTypeByIndex(UTI uti);
    const std::string getUlamTypeNameBriefByIndex(UTI uti);
    const std::string getUlamTypeNameByIndex(UTI uti);
    const std::string getTheInstanceMangledNameByIndex(UTI uti); //formerly EffectiveSelf
    const std::string getLocalsFilescopeTheInstanceMangledNameByIndex(UTI uti);

    ULAMTYPE getBaseTypeFromToken(const Token& tok);
    UTI getUlamTypeFromToken(const Token& tok, s32 typebitsize, s32 arraysize);
    UTI getUlamTypeFromToken(TypeArgs & args);

    bool getUlamTypeByTypedefName(u32 nameIdx, UTI & rtnType, UTI & rtnScalarType);
    bool getUlamTypeByTypedefNameinLocalsScope(u32 nameIdx, UTI & rtnType, UTI & rtnScalarType);

    /** turns array into its scalar type */
    UTI getUlamTypeAsScalar(UTI utArg);
    /** turns scalar type into its UNKNOWNSIZE array type */
    UTI getUlamTypeAsArrayOfScalar(UTI utiArg);
    /** turns a reference into its dereferenced type */
    UTI getUlamTypeAsDeref(UTI utArg);
    /** turns a regular type into its referenced type */
    UTI getUlamTypeAsRef(UTI utArg);
    UTI getUlamTypeAsRef(UTI utArg, ALT altArg, bool isConstRef);
    UTI getUlamTypeAsRef(UTI utArg, ALT altArg);
    ULAMTYPECOMPARERESULTS isARefTypeOfUlamType(UTI refuti, UTI ofuti); //true if de-refuti is same as ofuti

    UTI getUlamTypeOfConstant(ULAMTYPE etype);
    UTI getDefaultUlamTypeOfConstant(UTI ctype);
    bool getDefaultQuark(UTI cuti, u32& dqref);
    bool getPackedDefaultClass(UTI auti, u64& dpkref);
    void getDefaultAsPackedArray(UTI auti, u64 dval, u64& darrval);
    void getDefaultAsPackedArray(u32 len, u32 bitsize, u32 arraysize, u32 pos, u64 dval, u64& darrval);
    TBOOL tryToPackAClass(UTI cuti);
    bool getDefaultClassValue(UTI cuti, BV8K& dvref);
    void getDefaultAsArray(u32 bitsize, u32 arraysize, u32 tpos, const BV8K& dval, BV8K& darrval);
    bool genCodeClassDefaultConstantArray(File * fp, u32 len, BV8K& dval);

    bool isScalar(UTI utArg);
    s32 getArraySize(UTI utArg);
    s32 getBitSize(UTI utArg);
    ALT getReferenceType(UTI utArg);
    bool isReference(UTI utArg);
    bool isAltRefType(UTI utiArg);
    bool isConstantRefType(UTI utiArg);
    bool correctAReferenceTypeWith(UTI utiArg, UTI derefuti);
    bool correctAnArrayTypeWith(UTI utiArg, UTI scalaruti);
    bool isComplete(UTI utArg);
    bool completeAReferenceType(UTI utArg);
    bool completeAReferenceTypeWith(UTI utArg, UTI derefuti);
    bool isHolder(UTI utArg);
    bool setBitSize(UTI utArg, s32 total);
    bool setUTISizes(UTI utArg, s32 bitsize, s32 arraysize);
    void noteClassDataMembersTypeAndName(UTI cuti, s32 totalsize); //for errors
    void verifyZeroSizeUrSelf();
    void mergeClassUTI(UTI olduti, UTI cuti);
    bool isARootUTI(UTI auti);
    bool findaUTIAlias(UTI auti, UTI& aliasuti);
    void updateUTIAlias(UTI auti, UTI buti);
    void updateUTIAliasForced(UTI auti, UTI buti);
    void initUTIAlias(UTI auti);

    bool setSizesOfNonClassAndArrays(UTI utArg, s32 bitsize, s32 arraysize);

    s32 getDefaultBitSize(UTI uti);
    u32 getTotalBitSize(UTI utArg);
    u32 getTotalWordSize(UTI utArg);
    s32 slotsNeeded(UTI uti);
    bool isClassATemplate(UTI cuti);
    UTI isClassASubclass(UTI cuti); //returns super UTI, or Nav if no inheritance
    bool findClassAncestorWithMatchingNameid(UTI cuti, u32 nameid, UTI& superp);
    void resetClassSuperclass(UTI cuti, UTI superuti);
    bool isClassASubclassOf(UTI cuti, UTI superp);
    bool isClassAStub(UTI cuti);
    bool hasClassAStub(UTI cuti);
    bool isClassAQuarkUnion(UTI cuti);
    bool isClassACustomArray(UTI cuti);
    UTI getAClassCustomArrayType(UTI cuti);

    //returns number of matches (aref); updates last 2 args)
    u32 getAClassCustomArrayIndexType(UTI cuti, Node * rnode, UTI& idxuti, bool& hasHazyArgs);

    bool hasAClassCustomArrayLengthof(UTI cuti);

    /** return true and the Symbol pointer in 2nd arg if found;
	search SymbolTables LIFO order; o.w. return false
    */
    bool alreadyDefinedSymbolByAncestor(u32 dataindex, Symbol *& symptr, bool& hasHazyKin);
    bool alreadyDefinedSymbol(u32 dataindex, Symbol * & symptr, bool& hasHazyKin);
    bool isDataMemberIdInClassScope(u32 dataindex, Symbol * & symptr, bool& hasHazyKin);
    bool isIdInLocalFileScope(u32 id, Symbol *& symptr);

    bool isFuncIdInClassScope(u32 dataindex, Symbol * & symptr, bool& hasHazyKin);
    bool isFuncIdInClassScopeNNO(NNO cnno, u32 dataindex, Symbol * & symptr, bool& hasHazyKin);
    bool isFuncIdInAClassScope(UTI cuti, u32 dataindex, Symbol * & symptr, bool& hasHazyKin);
    bool findMatchingFunctionInAncestor(UTI cuti, u32 fid, std::vector<UTI> typeVec, SymbolFunction*& fsymref, UTI& foundInAncestor);

    bool isIdInCurrentScope(u32 id, Symbol *& asymptr);
    void addSymbolToCurrentScope(Symbol * symptr); //ownership goes to the block
    void addSymbolToLocalsScope(Symbol * symptr, Locator loc); //ownership goes to the m_localsPerFilePath ST
    void addSymbolToCurrentMemberClassScope(Symbol * symptr); //making stuff up for member
    void replaceSymbolInCurrentScope(u32 oldid, Symbol * symptr); //same symbol, new id
    void replaceSymbolInCurrentScope(Symbol * oldsym, Symbol * newsym); //same id, new symbol
    bool takeSymbolFromCurrentScope(u32 id, Symbol *& rtnsymptr); //ownership to the caller

    /** does a member class search for id */
    bool findSymbolInAClass(u32 id, UTI inClassUTI, Symbol *& rtnsymptr, bool& isHazy);

    /** return true and the SymbolClassName pointer in 2nd arg if found; */
    bool alreadyDefinedSymbolClassName(u32 dataindex, SymbolClassName * & symptr);

    /** return true and the SymbolClassName pointer in 2nd arg if found; uses key nameid of first arg */
    bool alreadyDefinedSymbolClassNameByUTI(UTI suti, SymbolClassName * & symptr); //helper

    /** return true and the SymbolClassNameTemplate pointer in 2nd arg if found AND is a template; */
    bool alreadyDefinedSymbolClassNameTemplate(u32 dataindex, SymbolClassNameTemplate * & symptr);

    /** return true and the SymbolClassNameTemplate pointer in 2nd arg for stubuti; uses key nameid of first arg */
    bool alreadyDefinedSymbolClassNameTemplateByUTI(UTI stubuti, SymbolClassNameTemplate * & symptr); //helper

    /** return true and the SymbolClass pointer in 2nd arg if uti found; */
    bool alreadyDefinedSymbolClass(UTI uti, SymbolClass * & symptr);

    /** return true and the SymbolClass pointer in 2nd arg if holder with uti as key nameid found; */
    bool alreadyDefinedSymbolClassAsHolder(UTI uti, SymbolClass * & symptr);

    /** in case of typedef's in ancestor class */
    void addUnknownTypeTokenToAClassResolver(UTI cuti, const Token& tok, UTI huti);
    void addUnknownTypeTokenToThisClassResolver(const Token& tok, UTI huti);
    Token removeKnownTypeTokenFromThisClassResolver(UTI huti);
    bool hasUnknownTypeInThisClassResolver(UTI huti);
    bool statusUnknownTypeInAClassResolver(UTI acuti, UTI huti);
    bool statusUnknownTypeInThisClassResolver(UTI huti);
    bool statusUnknownClassTypeInThisLocalsScope(const Token& tok, UTI huti, UTI& rtnuti);

    /** creates temporary class type for dataindex, returns the new Symbol pointer in 2nd arg; */
    bool removeIncompleteClassSymbolFromProgramTable(u32 id); //helper
    bool removeIncompleteClassSymbolFromProgramTable(const Token& nTok);
    bool addIncompleteClassSymbolToProgramTable(const Token& cTok, SymbolClassName * & symptr);
    bool addIncompleteTemplateClassSymbolToProgramTable(const Token& cTok, SymbolClassNameTemplate * & symptr);
    UTI addStubCopyToAncestorClassTemplate(UTI stubTypeToCopy, UTI argvaluecontext, UTI argtypecontext, Locator stubloc);

    void resetUnseenClass(SymbolClassName * cnsym, const Token& identTok);
    bool getUnseenClassFilenames(std::vector<std::string>& unseenFiles);

    /** during type labeling, sets ULAMCLASSTYPE for typedefs that
	involved incomplete Class types */
    bool completeIncompleteClassSymbolForTypedef(UTI incomplete) ;

    void updateLineageAndFirstCheckAndLabelPass();
    void updateLineageAndFirstCheckAndLabelPassForLocals();
    bool checkAndLabelPassForLocals();
    void defineRegistrationNumberForUlamClasses(); //ulam-4
    void defineRegistrationNumberForLocals(); //ulam-4

    void generateCodeForUlamClasses(FileManager * fm);
    void generateUlamClassForLocals(FileManager * fm);

    void generateGlobalUserStringPool(FileManager * fm);
    void genCodeBuiltInFunctionGetString(File * fp, bool declOnly);
    void genCodeBuiltInFunctionGetStringLength(File * fp, bool declOnly);

    const std::string & getDataAsFormattedUserString(u32 combinedidx);
    const std::string & getDataAsUnFormattedUserString(u32 combinedidx);
    bool isValidUserStringIndex(u32 combinedidx);
    u32 getUserStringLength(u32 combinedidx);

    bool countNavHzyNoutiNodesPass();
    void countNavNodesForLocals(u32& navcount, u32& hzycount, u32& unsetcount);

    /** helper methods for error messaging, uses string pool */
    const std::string getTokenLocationAsString(const Token * tok);
    const std::string getFullLocationAsString(const Locator& loc);
    const std::string getPathFromLocator(const Locator& loc);
    const std::string getFullPathFromLocator(const Locator& loc);

    /** helper method, uses string pool */
    const std::string getTokenDataAsString(const Token & tok);
    std::string getDataAsStringMangled(u32 dataindex);
    const std::string getTokenAsATypeName(const Token& tok);
    u32 getTokenAsATypeNameId(const Token& tok);

    bool checkFunctionReturnNodeTypes(SymbolFunction * fsym);
    void indentUlamCode(File * fp);
    void indent(File * fp);

    const char * getHiddenArgName();
    const char * getHiddenContextArgName();
    u32 getCustomArrayGetFunctionNameId();
    u32 getCustomArraySetFunctionNameId();
    const char * getCustomArrayGetMangledFunctionName();
    u32 getCustomArrayLengthofFunctionNameId();
    const char * getCustomArrayLengthofMangledFunctionName();

    const char * getIsMangledFunctionName(UTI ltype);
    const char * getAsMangledFunctionName(UTI ltype, UTI rtype);
    const char * getClassLengthFunctionName(UTI ltype);
    const char * getClassRegistrationNumberFunctionName(UTI ltype);
    const char * getGetStringFunctionName();
    const char * getGetStringLengthFunctionName();
    const char * getBuildDefaultAtomFunctionName(UTI ltype);
    const char * getDefaultQuarkFunctionName();

    std::string getFileNameForAClassHeader(UTI cuti, bool wSubDir = false);
    std::string getFileNameForThisClassHeader(bool wSubDir = false);
    void genCModeForHeaderFile(File * fp);
    void genCopyrightAndLicenseForUlamHeader(File * fp);
    std::string getFileNameForThisClassBody(bool wSubDir = false);
    std::string getFileNameForThisClassBodyNative(bool wSubDir = false);
    std::string getFileNameForThisClassCPP(bool wSubDir = false);
    std::string getFileNameForThisTypesHeader(bool wSubDir = false);
    std::string getFileNameForThisClassMain(bool wSubDir = false);
    u32 getMangledClassNameIdForUlamLocalsFilescope(UTI locuti);
    u32 getClassNameIdForUlamLocalsFilescope(UTI locuti);

    const std::string getStringMangledName();
    const char * getMangledNameForUserStringPool();
    const char * getDefineNameForUserStringPoolSize();
    std::string getFileNameForUserStringPoolHeader(bool wSubDir = false);
    std::string getFileNameForUserStringPoolCPP(bool wSubDir = false);

    ULAMCLASSTYPE getUlamClassForThisClass();
    UTI getUlamTypeForThisClass();

    const std::string getBitVectorLengthAsStringForCodeGen(UTI uti);

    /** returns immediate target value: extracts data from packed targets;
	unpacked array targets are invalid */
    UlamValue getPtrTarget(UlamValue ptr);

    UlamValue getPtrTargetFromAbsoluteIndex(UlamValue ptr);

    /** returns true if local variable */
    bool isLocalUnreturnableReferenceForEval(UlamValue ptr);

    bool isLocalUnreturnableAbsoluteReferenceForEval(UlamValue ptr);

    /** general purpose store value (except for Ptr as value) */
    void assignValue(UlamValue lptr, UlamValue ruv);

    /** used by assignValue when rhs is a Ptr (private) */
    void assignArrayValues(UlamValue lptr, UlamValue rptr);

    /** assign pointer as value */
    void assignValuePtr(UlamValue lptr, UlamValue rptr);

    UlamValue getByteOfUserStringForEval(u32 usrStr, u32 offsetInt);

    /** PACKEDLOADABLE fits in u32/u64, PACKED into an atom, o.w. UNPACKED */
    PACKFIT determinePackable(UTI aut);

    bool thisClassHasTheTestMethod();

    bool thisClassIsAQuark();

    bool quarkHasAToIntMethod(UTI quti);

    bool classCustomArraySetable(UTI cuti);

    bool hasThisClassStringDataMembers();

    void setupCenterSiteForTesting();
    void setupCenterSiteForGenCode();
    void generateTestInstancesForLocalsFilescopes(File * fp);

    /** used by SourceStream to build m_textByLinePerFilePath during parsing */
    void appendNextLineOfText(Locator loc, std::string textstr);

    /** used during codeGen to document the source Ulam code */
    std::string getLineOfText(Locator loc);
    std::string getLocationTextAsString(Locator nodeloc);
    void outputTextAsComment(File * fp, Locator nodeloc);
    void outputTextAsCommentWithLocationUpdate(File * fp, Locator nodeloc);
    void outputLineNumberForDebugging(File * fp, Locator nodeloc);

    s32 getNextTmpVarNumber();
    const std::string getTmpVarAsString(UTI uti, s32 num, TMPSTORAGE stg);
    const std::string getUlamRefTmpVarAsString(s32 num);
    const std::string getUlamClassTmpVarAsString(s32 num);
    const std::string getAtomBitStorageTmpVarAsString(s32 num);
    const std::string getLabelNumAsString(s32 num);
    const std::string getSwitchConditionNumAsString(s32 num);
    const std::string getSwitchTypedefNameAsString(s32 num);
    const std::string getInitDoneVarAsString(s32 num);
    const std::string getVFuncPtrTmpNumAsString(s32 num);
    const std::string getParserSymbolTypeFlagAsString(SYMBOLTYPEFLAG stf);

    /** for conditional h/as-magic */
    void saveIdentTokenForConditionalAs(const Token& iTok, const Token& cTok);
    void saveIdentTokenForPendingConditionalAs(const Token& iTok);
    void confirmParsingConditionalAs(const Token& cTok);

    /** class or model parameter structured comment for MFM */
    void saveStructuredCommentToken(const Token& scTok);
    void clearStructuredCommentToken();
    bool getStructuredCommentToken(Token& scTok);

    /** helpers: local def location and flag for parsing*/
    void setLocalsScopeForParsing(const Token& localTok);
    void clearLocalsScopeForParsing();
    bool isParsingLocalDef();
    Locator getLocalsScopeLocator();
    NodeBlockLocals * getLocalsScopeBlock(Locator loc);
    NodeBlockLocals * getLocalsScopeBlockByIndex(UTI luti);
    NodeBlockLocals * getLocalsScopeBlockByPathId(u32 pathid);
    NodeBlockLocals * makeLocalsScopeBlock(Locator loc);

    u32 findTypedefNameIdInLocalsScopeByIndex(UTI uti);

    /** to identify each node */
    NNO getNextNodeNo();

    Node * findNodeNoInThisClassOrLocalsScope(NNO n);
    Node * findNodeNoInAncestorsClassOrLocalsScope(NNO n, UTI cuti);
    Node * findNodeNoInThisClassForParent(NNO n);
    Node * findNodeNoInThisClassStubFirst(NNO n);
    Node * findNodeNoInAClass(NNO n, UTI cuti);
    Node * findNodeNoInAClassOrLocalsScope(NNO n, UTI cuti);
    UTI findAClassByNodeNo(NNO n);
    NodeBlockLocals * findALocalsScopeByNodeNo(NNO n);
    Node * findNodeNoInALocalsScope(Locator loc, NNO n);
    Node * findNodeNoInAncestorsLocalsScope(NNO n, UTI cuti);

    u32 getRegistrationNumberForClassOrLocalsScope(UTI cuti); //ulam-4
    u32 getAClassRegistrationNumber(UTI cuti); //ulam-4
    u32 getALocalsScopeRegistrationNumber(UTI cuti); //ulam-4

    NodeBlockClass * getAClassBlock(UTI cuti);
    NNO getAClassBlockNo(UTI cuti);

    /** methods for context switching */
    u32 getCompileThisId();

    UTI getCompileThisIdx();

    SymbolClass * getCurrentSelfSymbolForCodeGen();

    void appendNodeToCurrentBlock(Node * node);

    NodeBlock * getCurrentBlock();

    NNO getCurrentBlockNo();

    NodeBlockContext * getContextBlock();

    NNO getContextBlockNo();

    Locator getContextBlockLoc();

    bool useMemberBlock();

    NodeBlockClass * getCurrentMemberClassBlock();

    void pushClassContext(UTI idx, NodeBlock * currblock, NodeBlockContext * contextblock, bool usemember, NodeBlockClass * memberblock);

    void popClassContext();

    void pushCurrentBlock(NodeBlock * currblock);

    void pushCurrentBlockAndDontUseMemberBlock(NodeBlock * currblock);

    void pushClassContextUsingMemberClassBlock(NodeBlockClass * memberblock);

    void pushClassOrLocalContextAndDontUseMemberBlock(UTI context);

    void pushClassOrLocalCurrentBlock(UTI context);

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
    bool isThisLocalsFileScope();
    bool isALocalsFileScope(UTI uti);
    bool isAClass(UTI uti);
    bool isASeenClass(UTI cuti);
    bool isAnonymousClass(UTI cuti);
    void saveUrSelf(UTI uti);
    bool isUrSelf(UTI cuti);
    void saveEmptyUTI(UTI uti);
    bool isEmpty(UTI cuti);
    bool okUTItoContinue(UTI uti);
    bool neitherNAVokUTItoContinue(UTI uti1, UTI uti2); //false if either is Nav
    bool checkHasHazyKin(NodeBlock * block);

    inline void abortGreaterThanMaxBitsPerLong() { assert(0); }
    inline void abortUndefinedUlamType() { assert(0); }
    inline void abortUndefinedUlamClassType() { assert(0); }
    inline void abortUndefinedUlamPrimitiveType() { assert(0); }
    inline void abortUndefinedCallStack() { assert(0); }
    inline void abortNotImplementedYet() { assert(0); }
    inline void abortNotSupported() { assert(0); }
    inline void abortShouldntGetHere() { assert(0); }
    inline void abortNeedsATest() { assert(0); }

  private:
    ClassContextStack m_classContextStack; // the current subject of this compilation
    u32 m_registeredUlamClassCount; //borrowed from UlamClass in MFM

  };
}

#endif //end COMPILERSTATE_H
