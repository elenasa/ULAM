/**                                        -*- mode:C++ -*-
 * NodeCast.h - Basic Node for handling Type Casting for ULAM
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
  \file NodeCast.h - Basic Node for handling Type Casting for ULAM
  \author Elena S. Ackley.
  \author David H. Ackley.
  \date (C) 2014-2019 All rights reserved.
  \gpl
*/


#ifndef NODECAST_H
#define NODECAST_H

#include "NodeUnaryOp.h"
#include "NodeTypeDescriptor.h"

namespace MFM{

  class NodeCast : public NodeUnaryOp
  {
  public:

    NodeCast(Node * n, UTI typeToBe, NodeTypeDescriptor * nodetype, CompilerState & state);

    NodeCast(const NodeCast& ref);

    virtual ~NodeCast();

    virtual Node * instantiate();

    virtual void updateLineage(NNO pno);

    virtual bool findNodeNo(NNO n, Node *& foundNode);

    virtual void checkAbstractInstanceErrors();

    virtual const char * getName();

    virtual const std::string prettyNodeName();

    virtual TBOOL getStoreIntoAble();

    void setCastType(UTI tobe);

    UTI getCastType();

    void setExplicitCast();

    virtual bool isExplicitCast();

    virtual bool isAConstant();

    virtual bool isReadyConstant();

    virtual bool isNegativeConstant();

    virtual bool isWordSizeConstant();

    virtual bool isFunctionCall();

    virtual bool isAConstructorFunctionCall();

    virtual bool isArrayItem();

    virtual bool isExplicitReferenceCast(); //only NodeCast may return true

    virtual FORECAST safeToCastTo(UTI newType);

    virtual UTI checkAndLabelType();

    virtual void countNavHzyNoutiNodes(u32& ncnt, u32& hcnt, u32& nocnt);

    virtual EvalStatus eval();

    virtual EvalStatus evalToStoreInto();

    virtual void calcMaxDepth(u32& depth, u32& maxdepth, s32 base);

    virtual void genCode(File * fp, UVPass& uvpass);

  virtual void genCodeToStoreInto(File * fp, UVPass& uvpass);

    virtual void genCodeReadIntoATmpVar(File * fp, UVPass& uvpass);
    virtual void genCodeWriteFromATmpVar(File * fp, UVPass& luvpass, UVPass& ruvpass);

  protected:
    virtual UlamValue makeImmediateUnaryOp(UTI type, u32 data, u32 len); //noop
    virtual UlamValue makeImmediateLongUnaryOp(UTI type, u64 data, u32 len); //noop

    virtual UTI calcNodeType(UTI uti); //override

  private:
    UTI m_castToBe; //instead of nodetype used for Nav, Hzy, etc.
    bool m_explicit;   // requested by user (not automatic)
    NodeTypeDescriptor * m_nodeTypeDesc; //can be NULL

    bool needsACast(); // trying to avoid extraneous casting.

    void genCodeReadNonPrimitiveIntoATmpVar(File * fp, UVPass &uvpass);

    void genCodeCastAtomAndElement(File * fp, UVPass & uvpass);
    void genCodeCastAtomToElement(File * fp, UVPass & uvpass, s32 tmpvarstg, bool usePassVal);
    void genCodeCastElementToAtom(File * fp, UVPass & uvpass, s32 tmpvarstg, bool usePassVal);
    void genCodeCastLikeKindElements(File * fp, UVPass & uvpass, s32 tmpvarstg, bool usePassVal);

    void genCodeCastAtomAndQuark(File * fp, UVPass & uvpass);
    void genCodeCastAtomToQuark(File * fp, UVPass & uvpass, s32 tmpvarstg, bool usePassVal);
    void genCodeCastQuarkRefToAtom(File * fp, UVPass & uvpass, s32 tmpvarstg, bool usePassVal);

    void genCodeCastDescendant(File * fp, UVPass & uvpass);
    void genCodeCastAncestorQuarkAsSub(File * fp, UVPass & uvpass);

    void genPositionOfBaseIntoATmpVar(File * fp, u32 tmpvarpos, UVPass & uvpass, Symbol * stgcos, Symbol * cos); //helper

    void genCodeCastAsReference(File * fp, UVPass & uvpass);
    void genCodeCastFromAReference(File * fp, UVPass & uvpass);
    void genCodeToStoreIntoCastAsReference(File * fp, UVPass & uvpass);
    void genCodeToStoreIntoCastFromAReference(File * fp, UVPass & uvpass);
  };

}

#endif //end NODECAST_H
