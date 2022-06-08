/**                                        -*- mode:C++ -*-
 * NodeTypeDescriptorArray.h - Node Type Array descriptor for ULAM
 *
 * Copyright (C) 2015-2017 The Regents of the University of New Mexico.
 * Copyright (C) 2015-2021 Ackleyshack LLC.
 * Copyright (C) 2020-2021 The Living Computation Foundation
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
  \file NodeTypeDescriptorArray.h - Node Type Array descriptor for ULAM
  \author Elena S. Ackley.
  \author David H. Ackley.
  \date (C) 2015-2021 All rights reserved.
  \gpl
*/


#ifndef NODETYPEARRAYDESCRIPTOR_H
#define NODETYPEARRAYDESCRIPTOR_H

#include "NodeTypeDescriptor.h"
#include "NodeSquareBracket.h"

namespace MFM{

  class NodeTypeDescriptorArray : public NodeTypeDescriptor
  {
  public:

    NodeTypeDescriptorArray(const Token& tokarg, UTI auti, NodeTypeDescriptor * scalarnode, CompilerState & state);

    NodeTypeDescriptorArray(const NodeTypeDescriptorArray& ref);

    virtual ~NodeTypeDescriptorArray();

    virtual Node * instantiate();

    virtual void updateLineage(NNO pno);

    virtual bool findNodeNo(NNO n, Node *& foundNode);

    virtual void printPostfix(File * f);

    virtual const char * getName();

    virtual const std::string prettyNodeName();

    virtual UTI resetGivenUTI(UTI guti);

    virtual UTI getScalarType();

    virtual bool isEmptyArraysizeDecl();

    void linkConstantExpressionArraysize(NodeSquareBracket * ceForArraySize);

    virtual void setReferenceType(ALT refarg, UTI referencedUTI, UTI refUTI);

    virtual UTI checkAndLabelType(Node * thisparentnode);

    virtual void countNavHzyNoutiNodes(u32& ncnt, u32& hcnt, u32& nocnt);

  private:
    NodeTypeDescriptor * m_nodeScalar;
    NodeSquareBracket * m_unknownArraysizeSubtree;

    virtual bool resolveType(UTI& rtnuti);

    bool resolveTypeArraysize(UTI& rtnuti, UTI scuti);
    bool attemptToResolveHolderArrayType(UTI auti, UTI buti);
    void checkAndMatchClassTypes(UTI auti, UTI scuti);
    void checkAndMatchBaseUlamTypes(UTI auti, UTI scuti);
  };

} //MFM

#endif //NODETYPEARRAYDESCRIPTOR_H
