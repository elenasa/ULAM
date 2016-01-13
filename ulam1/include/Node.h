/**                                        -*- mode:C++ -*-
 * Node.h - Basic Node of Nodes for ULAM
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
  \file Node.h - Basic Node of Nodes for ULAM
  \author Elenas S. Ackley.
  \author David H. Ackley.
  \date (C) 2014-2015 All rights reserved.
  \gpl
*/


#ifndef NODE_H
#define NODE_H

#include <sstream>
#include <stdio.h>
#include <assert.h>
#include "CastOps.h"
#include "File.h"
#include "Locator.h"
#include "Symbol.h"
#include "UlamType.h"
#include "UlamValue.h"

namespace MFM{

  enum EVALS { EVAL_RHS, EVAL_LHS, EVAL_SIDEEFFECTS};
  enum EvalStatus {ERROR, NOTREADY, NORMAL, RETURN, BREAK, CONTINUE};

  struct CompilerState; //forward
  struct TypeArgs; //forward
  class NodeFunctionCall; //forward

  class Node
  {
  public:

    Node(CompilerState & state);

    Node(const Node& ref);

    virtual ~Node() {}

    virtual Node * instantiate() = 0;

    virtual void setYourParentNo(NNO pno);

    NNO getYourParentNo();

    virtual void updateLineage(NNO pno);

    virtual bool exchangeKids(Node * oldnptr, Node * newnptr);

    NNO getNodeNo();

    void resetNodeNo(NNO no); //for constant folding

    virtual bool findNodeNo(NNO n, Node *& foundNode);

    virtual void checkAbstractInstanceErrors();

    virtual void print(File * fp);

    virtual void printPostfix(File * fp) = 0;

    virtual const char * getName() = 0;

    virtual const std::string prettyNodeName() = 0;

    const std::string nodeName(const std::string& prettyFunction);

    UTI getNodeType();

    void setNodeType(UTI ut);

    virtual bool isStoreIntoAble();

    void setStoreIntoAble(bool s);

    Locator getNodeLocation();

    virtual void setNodeLocation(Locator loc);

    void printNodeLocation(File * fp);

    std::string getNodeLocationAsString();

    virtual bool getSymbolPtr(Symbol *& symptrref);

    virtual bool isAConstant();

    virtual bool isReadyConstant();

    virtual bool isFunctionCall();

    virtual FORECAST safeToCastTo(UTI newType);

    virtual UTI checkAndLabelType();

    virtual void countNavNodes(u32& cnt);

    virtual void constantFoldAToken(Token tok);

    virtual UTI constantFold();

    virtual bool buildDefaultQuarkValue(u32& dqref);

    virtual bool isNegativeConstant();

    virtual bool isWordSizeConstant();

    virtual bool installSymbolTypedef(TypeArgs& args, Symbol *& asymptr);

    virtual bool installSymbolConstantValue(TypeArgs& args, Symbol *& asymptr);

    virtual bool installSymbolParameterValue(TypeArgs& args, Symbol *& asymptr);

    virtual bool installSymbolVariable(TypeArgs& args,  Symbol *& asymptr);

    virtual bool assignClassArgValueInStubCopy();

    virtual EvalStatus eval() = 0;

    virtual EvalStatus evalToStoreInto();

    virtual void packBitsInOrderOfDeclaration(u32& offset);

    virtual void calcMaxDepth(u32& depth, u32& maxdepth, s32 base);

    virtual void genCode(File * fp, UlamValue& uvpass);

    virtual void genCodeToStoreInto(File * fp, UlamValue& uvpass);

    virtual void genCodeReadIntoATmpVar(File * fp, UlamValue& uvpass);

    virtual void genCodeWriteFromATmpVar(File * fp, UlamValue& luvpass, UlamValue& ruvpass);

    virtual void generateUlamClassInfo(File * fp, bool declOnly, u32& dmcount);

    virtual void genCodeExtern(File * fp, bool declOnly);

    /**
     * Returns converted const argument to all capital letters as a string
     */
    static std::string allCAPS(const char * s);

  protected:

    CompilerState & m_state;  //for printing error messages with path

    bool checkSafeToCastTo(UTI fromType, UTI newType);

    bool makeCastingNode(Node * node, UTI tobeType, Node*& rtnNode, bool isExplicit = false);

    bool warnOfNarrowingCast(UTI nodeType, UTI tobeType);

    void evalNodeProlog(u32 depth);

    void evalNodeEpilog();

    u32 makeRoomForNodeType(UTI type, STORAGE where = EVALRETURN);

    u32 makeRoomForSlots(u32 slots, STORAGE where = EVALRETURN);

    void assignReturnValueToStack(UlamValue rtnUV, STORAGE where = EVALRETURN);

    void assignReturnValuePtrToStack(UlamValue rtnUVptr);

    virtual void genMemberNameOfMethod(File * fp); //helper method to read/write into/from tmpvar
    virtual void genModelParameterMemberNameOfMethod(File * fp, s32 epi);

    virtual void genLocalMemberNameOfMethod(File * fp);

    //i.e. an immediate (right-justified); not a data member or self;
    bool isCurrentObjectALocalVariableOrArgument();

    //index of last "static" MP object; o.w.-1
    s32 isCurrentObjectsContainingAModelParameter();

    //index of last subclass; o.w.-1
    s32 isCurrentObjectsContainingASubClass();
    UTI findTypeOfAncestorAndBlockNo(NNO bno, s32 subcosidx);
    s32 calcPosOfCurrentObjectsContainingASubClass(bool isLocal);

    //false means its the entire array or not an array at all
    bool isCurrentObjectAnArrayItem(UTI cosuti, UlamValue uvpass);

    bool isCurrentObjectACustomArrayItem(UTI cosuti, UlamValue uvpass);

    bool isHandlingImmediateType();

    u32 adjustedImmediateArrayItemPtrPos(UTI cosuti, UlamValue uvpass);

    void genCodeConvertATmpVarIntoBitVector(File * fp, UlamValue & uvpass);

    void genCodeConvertABitVectorIntoATmpVar(File * fp, UlamValue & uvpass);

    //e.g. when lhs of member select is an array item of class type
    void genCodeConvertATmpVarIntoAutoRef(File * fp, UlamValue & uvpass);

    //e.g. when lhs of member select is an array item of class type, rhs data member
    void genCodeARefFromARefStorage(File * fp, UlamValue stguvpass, UlamValue uvpass);

    virtual void checkForSymbol();

    void genCodeReadElementTypeField(File * fp, UlamValue & uvpass);

    void restoreElementTypeForAncestorCasting(File * fp, UlamValue & uvpass);

  private:
    bool m_storeIntoAble;
    UTI m_utype;
    Locator m_loc;
    NNO m_parentNo;
    NNO m_no;

    //common helpers for safe casting
    NodeFunctionCall * buildCastingFunctionCallNode(Node * node, UTI tobeType);

    // 'UsingBitVector' variations for virtual quark functions only
    void genCodeReadIntoATmpVarUsingBitVector(File * fp, UlamValue & uvpass);
    void genCodeWriteFromATmpVarUsingBitVector(File * fp, UlamValue& luvpass, UlamValue& ruvpass);

    void genCodeReadSelfIntoATmpVar(File * fp, UlamValue & uvpass);
    void genCodeWriteToSelfFromATmpVar(File * fp, UlamValue& luvpass, UlamValue& ruvpass);

    void genCodeReadAutorefIntoATmpVar(File * fp, UlamValue& uvpass);
    void genCodeWriteToAutorefFromATmpVar(File * fp, UlamValue& luvpass, UlamValue& ruvpass);

    void genCodeReadArrayItemIntoATmpVar(File * fp, UlamValue& uvpass);
    void genCodeReadArrayItemIntoATmpVarUsingBitVector(File * fp, UlamValue& uvpass);
    void genCodeReadCustomArrayItemIntoATmpVar(File * fp, UlamValue & uvpass);

    void genCodeWriteArrayItemFromATmpVar(File * fp, UlamValue& luvpass, UlamValue& ruvpass);
    void genCodeWriteArrayItemFromATmpVarUsingBitVector(File * fp, UlamValue& luvpass, UlamValue& ruvpass);
    void genCodeWriteCustomArrayItemFromATmpVar(File * fp, UlamValue& luvpass, UlamValue& ruvpass);

    virtual void genModelParameterHiddenArgs(File * fp, s32 epi);

    void genCustomArrayMemberNameOfMethod(File * fp);
    void genCustomArrayHiddenArgs(File * fp);

    void genLocalMemberNameOfMethodByUsTypedef(File * fp);
    void genCustomArrayLocalMemberNameOfMethod(File * fp);

    const std::string localStorageTypeAsString(UTI nuti);

    const std::string tmpStorageTypeForRead(UTI nuti, UlamValue uvpass);
    const std::string tmpStorageTypeForReadArrayItem(UTI nuti, UlamValue uvpass);

    const std::string readMethodForCodeGen(UTI nuti, UlamValue uvpass);
    const std::string readArrayItemMethodForCodeGen(UTI nuti, UlamValue uvpass);

    const std::string writeMethodForCodeGen(UTI nuti, UlamValue uvpass);
    const std::string writeArrayItemMethodForCodeGen(UTI nuti, UlamValue uvpass);

    const std::string readMethodForImmediateBitValueForCodeGen(UTI nuti, UlamValue uvpass);
    const std::string writeMethodForImmediateBitValueForCodeGen(UTI nuti, UlamValue uvpass);

  };

}

#endif //end NODE_H
