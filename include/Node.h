/**                                        -*- mode:C++ -*-
 * Node.h - Basic Node of Nodes for ULAM
 *
 * Copyright (C) 2014-2019 The Regents of the University of New Mexico.
 * Copyright (C) 2014-2024 Ackleyshack LLC.
 * Copyright (C) 2020-2024 The Living Computation Foundation.
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
  \author Elena S. Ackley.
  \author David H. Ackley.
  \date (C) 2014-2024 All rights reserved.
  \gpl
*/


#ifndef NODE_H
#define NODE_H

#include <sstream>
#include <stdio.h>
#include <assert.h>
#include <vector>
#include "BitVector.h"
#include "CastOps.h"
#include "File.h"
#include "Locator.h"
#include "Symbol.h"
#include "SymbolTmpVar.h"
#include "UlamType.h"
#include "UlamValue.h"
#include "UVPass.h"
#include "MapClassMemberDesc.h"

namespace MFM{

#define NODE_ASSERT(expr) { if(!(expr)) { Node::nodeFailPrint(); assert((expr)); } }

enum EVALS { EVAL_RHS, EVAL_LHS, EVAL_SIDEEFFECTS};
enum EvalStatus {ERROR, NOTREADY, NORMAL, RETURN, BREAK, CONTINUE, UNEVALUABLE};

  struct CompilerState; //forward
  struct TypeArgs; //forward
  class NodeFunctionCall; //forward
  class NodeBlock; //forward
  class SymbolClass; //forward

  class Node
  {
  public:

    Node(CompilerState & state);

    Node(const Node& ref);

    virtual ~Node() {}

    virtual Node * instantiate() = 0;

    virtual void setYourParentNo(NNO pno);

    NNO getYourParentNo() const;

    virtual void updateLineage(NNO pno);

    virtual bool exchangeKids(Node * oldnptr, Node * newnptr);

    NNO getNodeNo() const;

    virtual void resetNodeNo(NNO no); //for constant folding

    virtual bool findNodeNo(NNO n, Node *& foundNode);

    virtual void checkAbstractInstanceErrors();

    virtual TBOOL checkVarUsedBeforeDeclared(u32 id, NNO declblockno);

    virtual void print(File * fp);

    virtual void printPostfix(File * fp) = 0;

    virtual void noteTypeAndName(UTI cuti, s32 totalsize, u32& accumsize);

    virtual void genTypeAndNameEntryAsComment(File * fp, s32 totalsize, u32& accumsize);

    virtual const char * getName() = 0;

    virtual u32 getNameId();

    virtual u32 getTypeNameId();

    virtual UTI getTypeDescriptorGivenType();

    virtual ALT getTypeDescriptorRefType();

    virtual const std::string prettyNodeName() = 0;

    const std::string nodeName(const std::string& prettyFunction);

    virtual UTI getNodeType() const;

    virtual void setNodeType(UTI ut);

    virtual void resetOfClassType(UTI cuti);

    virtual void setClassType(UTI cuti);

    virtual bool isClassInit();

    virtual TBOOL getStoreIntoAble() const;

    void setStoreIntoAble(TBOOL s);

    virtual TBOOL getReferenceAble() const;

    void setReferenceAble(TBOOL s);

    static TBOOL minTBOOL(TBOOL atb, TBOOL btb);

    Locator getNodeLocation() const;

    virtual void setNodeLocation(Locator loc);

    virtual void resetNodeLocations(Locator loc);

    void printNodeLocation(File * fp);

    std::string getNodeLocationAsString() const;

    virtual void clearSymbolPtr();

    virtual bool cloneSymbol(Symbol *& symptrref);

    virtual bool getSymbolPtr(const Symbol *& symptrref);

    virtual bool getStorageSymbolPtr(const Symbol *& symptrref);

    virtual bool getSymbolPtr(Symbol *& symptrref) const;

    virtual bool getSymbolValue(BV8K& bv);

    virtual bool compareSymbolPtrs(Symbol * ptr);

    virtual u32 getSymbolId();

    virtual bool hasASymbol();

    virtual bool hasASymbolDataMember();

    virtual bool hasASymbolSuper();

    virtual bool hasASymbolSelf();

    virtual bool hasASymbolReference();

    virtual bool hasASymbolReferenceConstant();

    virtual u32 getPositionOf();

    virtual s32 getSymbolStackFrameSlotIndex();

    virtual UlamValue getSymbolAutoPtrForEval();

    virtual UTI getSymbolAutoStorageTypeForEval();

    virtual u32 getSymbolDataMemberPosOffset();

    virtual bool belongsToVOWN(UTI vown);

    virtual bool isACast();

    virtual TBOOL isAConstant();

    virtual bool isAConstantClass();

    virtual bool isAConstantClassArray();

    virtual bool isReadyConstant();

    virtual bool isNegativeConstant();

    virtual bool isWordSizeConstant(u32 wordsize);

    virtual bool isFunctionCall();

    virtual bool isAConstructorFunctionCall();

    virtual bool isAVirtualFunctionCall();

    virtual bool isArrayItem();

    virtual bool isEmptyArraysizeDecl();

    virtual bool isAList();

    virtual bool isExplicitReferenceCast(); //only NodeCast may return true

    virtual bool isExplicitCast(); //only NodeCast may return true

    virtual bool asConditionalNode(); //only NodeConditionalAs returns true

    virtual bool isAMemberSelect();

    virtual bool isTernaryExpression();

    virtual bool getConstantValue(BV8K& bval);

    virtual FORECAST safeToCastTo(UTI newType);

    virtual UTI checkAndLabelType(Node * thisparentnode);

    bool exchangeNodeWithParent(Node * newnode, Node * parent);

    virtual bool trimToTheElement(Node ** fromleftnode, Node *& rtnnodeptr);

    virtual void countNavHzyNoutiNodes(u32& ncnt, u32& hcnt, u32& nocnt);

    virtual UTI constantFold(Node * parentnode);

    virtual bool buildDefaultValue(u32 wlen, BV8K& dvref);

    virtual bool buildDefaultValueForClassConstantDefs();

    virtual bool initDataMembersConstantValue(BV8K& bvref, BV8K& bvmask);

    virtual bool installSymbolTypedef(TypeArgs& args, Symbol *& asymptr);

    virtual bool installSymbolConstantValue(TypeArgs& args, Symbol *& asymptr);

    virtual bool installSymbolModelParameterValue(TypeArgs& args, Symbol *& asymptr);

    virtual bool installSymbolVariable(TypeArgs& args,  Symbol *& asymptr);

    virtual EvalStatus eval() = 0;
    virtual EvalStatus evalToStoreInto();
    virtual EvalStatus evalErrorReturn();
    virtual EvalStatus evalStatusReturnNoEpilog(EvalStatus evs);
    virtual EvalStatus evalStatusReturn(EvalStatus evs);
    virtual UlamValue makeUlamValuePtr();

    virtual TBOOL packBitsInOrderOfDeclaration(u32& offset);

    virtual void printUnresolvedVariableDataMembers();

    virtual void printUnresolvedLocalVariables(u32 fid);

    virtual void calcMaxDepth(u32& depth, u32& maxdepth, s32 base);

    virtual void calcMaxIndexOfVirtualFunctionInOrderOfDeclaration(SymbolClass* csym, s32& maxidx);

    virtual void genCode(File * fp, UVPass& uvpass);

    virtual void genCodeToStoreInto(File * fp, UVPass& uvpass);

    virtual void generateFunctionInDeclarationOrder(File * fp, bool declOnly, ULAMCLASSTYPE classtype);

    virtual void genCodeReadIntoATmpVar(File * fp, UVPass& uvpass);

    virtual void genCodeWriteFromATmpVar(File * fp, UVPass& luvpass, UVPass& ruvpass);

    virtual void genCodeConstantArrayInitialization(File * fp);

    virtual void generateBuiltinConstantClassOrArrayInitializationFunction(File * fp, bool declOnly);

    virtual void cloneAndAppendNode(std::vector<Node *> & cloneVec);

    virtual void generateTestInstance(File * fp, bool runtest);

    virtual void generateUlamClassInfo(File * fp, bool declOnly, u32& dmcount);

    virtual void addMemberDescriptionToInfoMap(UTI classType, ClassMemberMap& classmembers);

    virtual void genCodeExtern(File * fp, bool declOnly);

    virtual void genCodeConvertATmpVarIntoBitVector(File * fp, UVPass & uvpass);

    /**
     * Returns converted const argument to all capital letters as a string
     */
    static std::string allCAPS(const char * s);


    virtual void nodeFailPrint();

    virtual void nodeFailPrint() const;

  protected:

    CompilerState & m_state;  //for printing error messages with path

    virtual bool checkSafeToCastTo(UTI fromType, UTI& newType);

    bool makeCastingNode(Node * node, UTI tobeType, Node*& rtnNode, bool isExplicit = false);

    bool warnOfNarrowingCast(UTI nodeType, UTI tobeType);

    void evalNodeProlog(u32 depth);

    void evalNodeEpilog();

    u32 makeRoomForNodeType(UTI type, STORAGE where = EVALRETURN);

    u32 makeRoomForSlots(u32 slots, STORAGE where = EVALRETURN);

    void assignReturnValueToStack(UlamValue rtnUV, STORAGE where = EVALRETURN);

    void assignReturnValuePtrToStack(UlamValue rtnUVptr, STORAGE where = EVALRETURN);

    UlamValue assignAnonymousClassReturnValueToStack(UlamValue rtnUV);

    bool returnValueOnStackNeededForEval(UTI rtnType);

    virtual void genMemberNameOfMethod(File * fp, const UVPass& uvpass, bool endingdot = true); //helper method to read/write into/from tmpvar
    virtual void genModelParameterMemberNameOfMethod(File * fp, s32 epi);

    virtual void genLocalMemberNameOfMethod(File * fp, const UVPass& uvpass);

    void genLocalMemberNameOfMethodForAtomof(File * fp, const UVPass& uvpass);

    //return index of stgcos in stack of symbols, -1 if stack is empty and currentSelf.
    s32 loadStorageAndCurrentObjectSymbols(Symbol *& stgcosref, Symbol *&cosref);

    //i.e. an immediate (right-justified); not a data member or self;
    bool isCurrentObjectALocalVariableOrArgument();

    //index of last "static" MP object; o.w.-1
    s32 isCurrentObjectsContainingAModelParameter();

    //index of last named constant (array) object; o.w.-1
    s32 isCurrentObjectsContainingAConstant();

    //index of last named constant class object; o.w.-1
    s32 isCurrentObjectsContainingAConstantClass();

    //index of first element or ele ref object; o.w. -1
    s32 isCurrentObjectsContainingAnElement();

    //index of last selected Base Type tmp symbol object; o.w.-1
    s32 isCurrentObjectsContainingABaseTypeTmpSymbol();

    //index of last selected Sub/Base ClassId tmp symbol object; o.w.-1
    s32 isCurrentObjectsContainingABaseRegNumTmpSymbol();

    //index of last tmp symbol object; o.w.-1
    s32 isCurrentObjectsContainingATmpVarSymbol();

    //index of first DM symbol object; o.w.-1
    s32 isCurrentObjectsContainingFirstDataMember();

    // used by genHiddenArg2 for function calls;
    std::string calcPosOfCurrentObjectClassesAsString(const UVPass& uvpass, bool adjstEle, bool askeffselfarg, UTI funcclassarg);

    //called when (implicit self) data member is a complete class;
    //pos known at compile time (e.g. t3541)
    u32 calcDataMemberPosOfCurrentObjectClasses(bool askingeffself, UTI funcclassarg);

    //called when storage is a ref, but pos from first data member is known at compile time
    u32 calcDataMemberPosOfCurrentObjectClassesFromFirstDMIndex(u32 firstdmindex);

    //true means we can't know rel pos of 'stg' until runtime; o.w. known at compile time.
    bool askEffectiveSelfAtRuntimeForRelPosOfBase(UTI funcclassarg = Nouti);

    //false means its the entire array or not an array at all
    bool isCurrentObjectAnArrayItem(UTI cosuti, const UVPass& uvpass);

    bool isCurrentObjectACustomArrayItem(UTI cosuti, const UVPass& uvpass);

    bool isCurrentObjectAnUnpackedArray(UTI cosuti, const UVPass& uvpass);

    bool isHandlingImmediateType();

    //true if a non-ref, scalar element
    bool needAdjustToStateBits(UTI cuti);

    SymbolTmpVar * makeTmpVarSymbolForCodeGen(UVPass& uvpass, Symbol * symarg);

    std::string genUlamRefUsageAsString(UTI uti);

    void genCodeConvertABitVectorIntoATmpVar(File * fp, UVPass & uvpass);

    //e.g. when lhs of member select is an array item of class type
    void genCodeConvertATmpVarAutoRefIntoAutoRef(File * fp, UVPass & uvpass);

    //e.g. when a variable array item, may be of class type
    void genCodeConvertATmpVarIntoAutoRef(File * fp, UVPass & luvpass, UVPass ruvpass);

    //e.g. when constant array item, primitive
    void genCodeConvertATmpVarIntoConstantAutoRef(File * fp, UVPass & luvpass, UVPass ruvpass);
    //e.g. when lhs of member select is an array item of class type, rhs data member
    void genCodeARefFromARefStorage(File * fp, UVPass stguvpass, UVPass uvpass);

    //e.g. when a ref to a primitive is used as a lhs binary op, or if condition (t41140)
    void genCodeReadAutorefIntoATmpVar(File * fp, UVPass& uvpass);

    void genCodeReferenceInitialization(File * fp, UVPass& uvpass, Symbol * vsymptr);

    void genCodeReadFromAConstantClassIntoATmpVar(File * fp, UVPass& uvpass);

    void genCodeReadArrayItemFromAConstantClassIntoATmpVar(File * fp, UVPass & luvpass, UVPass & ruvpass);
    void genCodeReadPrimitiveArrayItemIntoATmpVar(File * fp, UVPass & luvpass, UVPass & ruvpass);

    virtual void checkForSymbol();
    virtual TBOOL replaceOurselves(Symbol * symptr, Node * parentnode);

    void genCodeReadElementTypeField(File * fp, UVPass & uvpass);
    void restoreElementTypeForAncestorCasting(File * fp, UVPass & uvpass);

    //common helpers for safe casting
    bool buildCastingFunctionCallNode(Node * node, UTI tobeType, Node*& rtnNode);
    Node * buildToIntCastingNode(Node * node);
    Node * newCastingNode(Node * node, UTI tobeType);
    bool newCastingNodeWithCheck(Node * node, UTI tobeType, Node*& rtnNode);

    //helper for Operator OVerload Function Calls
    Node * buildOperatorOverloadFuncCallNodeHelper(Node * selectNode, Node * argNode, const char * nameForTok);

    //used for function calls second arg, including custom array accessors, and virtual funcso
    std::string genHiddenArg2(const UVPass& uvpass, u32& urtmpnumref, UTI vownarg, UTI funcclassarg);
    u32 getLengthOfMemberClassForHiddenArg(UTI uti);
    u32 getBaseLengthOfMemberClassForHiddenArg(UTI uti);

  private:
    UTI m_utype;
    TBOOL m_storeIntoAble;
    TBOOL m_referenceAble;
    Locator m_loc;
    NNO m_parentNo;
    NNO m_no;

    void genCodeReadSelfIntoATmpVar(File * fp, UVPass & uvpass);
    void genSelfNameOfMethod(File * fp, bool appenddot = true);
    void genCodeWriteToSelfFromATmpVar(File * fp, UVPass& luvpass, UVPass& ruvpass);
    void genCodeWriteToRefselfFromATmpVar(File * fp, UVPass& luvpass, UVPass& ruvpass);

    void genCodeReadStringArrayIntoATmpVar(File * fp, UVPass & uvpass);
    void genCodeWriteToStringArrayFromATmpVar(File * fp, UVPass & luvpass, UVPass & ruvpass);

    void genCodeReadTransientIntoATmpVar(File * fp, UVPass & uvpass);
    void genCodeWriteToTransientFromATmpVar(File * fp, UVPass & luvpass, UVPass & ruvpass);

    void genCodeWriteToAtomofRefFromATmpVar(File * fp, UVPass& luvpass, UVPass& ruvpass);

    void genCodeWriteToAutorefFromATmpVar(File * fp, UVPass& luvpass, UVPass& ruvpass);

    virtual void genModelParameterHiddenArgsToFile(File * fp, s32 epi);

    void genCustomArrayMemberNameOfMethod(File * fp);
    void genCustomArrayHiddenArgs(File * fp, u32 urtmpnum);

    void genLocalMemberNameOfMethodByUsTypedef(File * fp, const UVPass& uvpass);
    void genCustomArrayLocalMemberNameOfMethod(File * fp);

    void genConstantClassMangledName(File * fp, const char * const prefix = NULL);
    void genConstantArrayMangledName(File * fp, const char * const prefix = NULL);

    const std::string localStorageTypeAsString(UTI nuti);

    const std::string tmpStorageTypeForRead(UTI nuti, const UVPass& uvpass);
    const std::string tmpStorageTypeForReadArrayItem(UTI nuti, const UVPass& uvpass);

    const std::string readMethodForCodeGen(UTI nuti, const UVPass& uvpass);
    const std::string readArrayItemMethodForCodeGen(UTI nuti, const UVPass& uvpass);

    const std::string writeMethodForCodeGen(UTI nuti, const UVPass& uvpass);
    const std::string writeArrayItemMethodForCodeGen(UTI nuti, const UVPass& uvpass);

    void genCodeArrayRefInit(File * fp, UVPass & uvpass, Symbol * vsymptr);
    void genCodeArrayItemRefInit(File * fp, UVPass & uvpass, Symbol * vsymptr);
  };

}

#endif //end NODE_H
