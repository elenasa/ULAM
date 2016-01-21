/**                                        -*- mode:C++ -*-
 * NodeTypeDescriptorSelect.h - Node Descriptor for Type Selection for ULAM
 *
 * Copyright (C) 2015 The Regents of the University of New Mexico.
 * Copyright (C) 2015 Ackleyshack LLC.
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
  \file NodeTypeDescriptorSelect.h - Node Descriptor for Type Selection for ULAM
  \author Elenas S. Ackley.
  \author David H. Ackley.
  \date (C) 2015 All rights reserved.
  \gpl
*/


#ifndef NODETYPESELECTDESCRIPTOR_H
#define NODETYPESELECTDESCRIPTOR_H

#include "NodeTypeDescriptor.h"

namespace MFM{

  class NodeTypeDescriptorSelect : public NodeTypeDescriptor
  {
  public:

    NodeTypeDescriptorSelect(Token tokarg, UTI auti, NodeTypeDescriptor * node, CompilerState & state);

    NodeTypeDescriptorSelect(const NodeTypeDescriptorSelect& ref);

    virtual ~NodeTypeDescriptorSelect();

    virtual Node * instantiate();

    virtual void updateLineage(NNO pno);

    virtual bool findNodeNo(NNO n, Node *& foundNode);

    virtual void printPostfix(File * f);

    virtual const char * getName();

    virtual const std::string prettyNodeName();

    virtual UTI checkAndLabelType();

    virtual void countNavHzyNoutiNodes(u32& ncnt, u32& hcnt, u32& nocnt);

  private:
    NodeTypeDescriptor * m_nodeSelect; //selected from, or null

    virtual bool resolveType(UTI& rtnuti);

  };

} //MFM

#endif //NODETYPESELECTDESCRIPTOR_H
