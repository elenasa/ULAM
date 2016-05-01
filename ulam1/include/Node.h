/**                                        -*- mode:C++ -*-
 * Node.h - Basic Node of Nodes for ULAM
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
  \file Node.h - Basic Node of Nodes for ULAM
  \author Elenas S. Ackley.
  \author David H. Ackley.
  \date (C) 2014-2016 All rights reserved.
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
#include "UVPass.h"
#include "BitVector.h"

namespace MFM{

  enum EVALS { EVAL_RHS, EVAL_LHS, EVAL_SIDEEFFECTS};
  enum EvalStatus {ERROR, NOTREADY, NORMAL, RETURN, BREAK, CONTINUE, UNEVALUABLE};

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

    virtual TBOOL getStoreIntoAble();

    void setStoreIntoAble(TBOOL s);

    Locator getNodeLocation();

    virtual void setNodeLocation(Locator loc);

    void printNodeLocation(File * fp);

    std::string getNodeLocationAsString();

    virtual bool getSymbolPtr(Symbol *& symptrref);

    virtual bool isAConstant();

    virtual bool isReadyConstant();

    virtual bool isFunctionCall();

    virtual bool isExplicitReferenceCast(); //only NodeCast may return true

    virtual FORECAST safeToCastTo(UTI newType);

    virtual UTI checkAndLabelType();

    virtual void countNavHzyNoutiNodes(u32& ncnt, u32& hcnt, u32& nocnt);

    virtual void constantFoldAToken(Token tok);

    virtual UTI constantFold();

    virtual bool buildDefaultQuarkValue(u32& dqref);

    virtual bool buildDefaultValue(u32 wlen, BV8K& dvref);

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

    virtual void printUnresolvedVariableDataMembers();

    virtual void printUnresolvedLocalVariables(u32 fid);

    virtual void calcMaxDepth(u32& depth, u32& maxdepth, s32 base);

    virtual void genCode(File * fp, UVPass& uvpass);

    virtual void genCodeToStoreInto(File * fp, UVPass& uvpass);

    virtual void genCodeReadIntoATmpVar(File * fp, UVPass& uvpass);

    virtual void genCodeWriteFromATmpVar(File * fp, UVPass& luvpass, UVPass& ruvpass);

    virtual void generateUlamClassInfo(File * fp, bool declOnly, u32& dmcount);

    virtual void genCodeExtern(File * fp, bool declOnly);

    /**
     * Returns converted const argument to all capital letters as a string
     */
    static std::string allCAPS(const char * s);

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

    void assignReturnValuePtrToStack(UlamValue rtnUVptr);

    virtual void genMemberNameOfMethod(File * fp, bool endingdot = true); //helper method to read/write into/from tmpvar
    virtual void genModelParameterMemberNameOfMethod(File * fp, s32 epi);

    virtual void genLocalMemberNameOfMethod(File * fp);

    //i.e. an immediate (right-justified); not a data member or self;
    bool isCurrentObjectALocalVariableOrArgument();

    //index of last "static" MP object; o.w.-1
    s32 isCurrentObjectsContainingAModelParameter();

    s32 calcPosOfCurrentObjectClasses();
    s32 calcPosOfCurrentObjects(bool onlyClasses = false);

    //false means its the entire array or not an array at all
    bool isCurrentObjectAnArrayItem(UTI cosuti, UVPass uvpass);

    bool isCurrentObjectACustomArrayItem(UTI cosuti, UVPass uvpass);

    bool isCurrentObjectAnUnpackedArray(UTI cosuti, UVPass uvpass);

    bool isHandlingImmediateType();

    u32 adjustedImmediateArrayItemPassPos(UTI cosuti, UVPass uvpass);

    void genCodeConvertATmpVarIntoBitVector(File * fp, UVPass & uvpass);

    void genCodeConvertABitVectorIntoATmpVar(File * fp, UVPass & uvpass);

    //e.g. when lhs of member select is an array item of class type
    void genCodeConvertATmpVarIntoAutoRef(File * fp, UVPass & uvpass);

    //e.g. when lhs of member select is an array item of class type, rhs data member
    void genCodeARefFromARefStorage(File * fp, UVPass stguvpass, UVPass uvpass);

    virtual void checkForSymbol();

    void genCodeReadElementTypeField(File * fp, UVPass & uvpass);

    void restoreElementTypeForAncestorCasting(File * fp, UVPass & uvpass);

    //common helpers for safe casting
    NodeFunctionCall * buildCastingFunctionCallNode(Node * node, UTI tobeType);
    Node * buildToIntCastingNode(Node * node);

  private:
    TBOOL m_storeIntoAble;
    UTI m_utype;
    Locator m_loc;
    NNO m_parentNo;
    NNO m_no;

    void genCodeReadSelfIntoATmpVar(File * fp, UVPass & uvpass);
    void genCodeWriteToSelfFromATmpVar(File * fp, UVPass& luvpass, UVPass& ruvpass);

    void genCodeWriteToAtomofRefFromATmpVar(File * fp, UVPass& luvpass, UVPass& ruvpass);

    void genCodeReadAutorefIntoATmpVar(File * fp, UVPass& uvpass);
    void genCodeWriteToAutorefFromATmpVar(File * fp, UVPass& luvpass, UVPass& ruvpass);

    void genCodeReadArrayItemIntoATmpVar(File * fp, UVPass& uvpass);
    void genCodeReadCustomArrayItemIntoATmpVar(File * fp, UVPass & uvpass);

    void genCodeWriteArrayItemFromATmpVar(File * fp, UVPass& luvpass, UVPass& ruvpass);
    void genCodeWriteCustomArrayItemFromATmpVar(File * fp, UVPass& luvpass, UVPass& ruvpass);

    virtual void genModelParameterHiddenArgs(File * fp, s32 epi);

    void genCustomArrayMemberNameOfMethod(File * fp);
    void genCustomArrayHiddenArgs(File * fp, u32 urtmpnum);
    std::string genHiddenArg2ForCustomArray(u32& urtmpnumref);

    void genLocalMemberNameOfMethodByUsTypedef(File * fp);
    void genCustomArrayLocalMemberNameOfMethod(File * fp);

    const std::string localStorageTypeAsString(UTI nuti);

    const std::string tmpStorageTypeForRead(UTI nuti, UVPass uvpass);
    const std::string tmpStorageTypeForReadArrayItem(UTI nuti, UVPass uvpass);

    const std::string readMethodForCodeGen(UTI nuti, UVPass uvpass);
    const std::string readArrayItemMethodForCodeGen(UTI nuti, UVPass uvpass);

    const std::string writeMethodForCodeGen(UTI nuti, UVPass uvpass);
    const std::string writeArrayItemMethodForCodeGen(UTI nuti, UVPass uvpass);
  };

}

#endif //end NODE_H
