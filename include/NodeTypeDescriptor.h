/**                                        -*- mode:C++ -*-
 * NodeTypeDescriptor.h - Basic Node Type descriptor for ULAM
 *
 * Copyright (C) 2015-2018 The Regents of the University of New Mexico.
 * Copyright (C) 2015-2018 Ackleyshack LLC.
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
  \file NodeTypeDescriptor.h - Basic Node Type descriptor for ULAM
  \author Elena S. Ackley.
  \author David H. Ackley.
  \date (C) 2015-2018 All rights reserved.
  \gpl
*/


#ifndef NODETYPEDESCRIPTOR_H
#define NODETYPEDESCRIPTOR_H

#include "Node.h"
#include "NodeTypeBitsize.h"

namespace MFM{

  class NodeTypeDescriptor : public Node
  {
  public:

    NodeTypeDescriptor(const Token& typetoken, UTI auti, CompilerState & state);

    NodeTypeDescriptor(const Token& typetoken, UTI auti, CompilerState & state, ALT refarg, UTI referencedUTIarg);

    NodeTypeDescriptor(const NodeTypeDescriptor& ref);

    NodeTypeDescriptor(const NodeTypeDescriptor& ref, bool keepType);

    virtual ~NodeTypeDescriptor();

    virtual Node * instantiate();

    virtual void updateLineage(NNO pno);

    virtual bool findNodeNo(NNO n, Node *& foundNode);

    virtual void printPostfix(File * f);

    virtual const char * getName();

    virtual u32 getTypeNameId();

    virtual const std::string prettyNodeName();

    void linkConstantExpressionBitsize(NodeTypeBitsize * ceForBitSize);

    bool isReadyType();

    UTI givenUTI();

    void resetGivenUTI(UTI guti);

    UTI getReferencedUTI();

    ALT getReferenceType();

    void setReferenceType(ALT refarg, UTI referencedUTI);

    virtual void setReferenceType(ALT refarg, UTI referencedUTI, UTI refUTI);

    UTI getContextForPendingArgType();

    void setContextForPendingArgType(UTI context);

    virtual UTI checkAndLabelType();

    virtual void countNavHzyNoutiNodes(u32& ncnt, u32& hcnt, u32& nocnt);

    virtual bool assignClassArgValueInStubCopy();

    virtual EvalStatus eval();

  protected:
    Token m_typeTok;
    UTI m_uti;
    bool m_ready;
    UTI m_contextForPendingArgType;

  private:
    NodeTypeBitsize * m_unknownBitsizeSubtree;
    ALT m_refType;
    UTI m_referencedUTI;

    virtual bool resolveType(UTI& rtnuti);
    bool resolveReferenceType(UTI& rtnuti);
    bool resolveClassType(UTI& rtnuti);
    bool resolvePrimitiveOrArrayType(UTI& rtnuti);
    bool resolveTypeBitsize(UTI& rtnuti);
  };

} //MFM

#endif //NODETYPEDESCRIPTOR_H
