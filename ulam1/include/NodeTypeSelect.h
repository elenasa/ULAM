/**                                        -*- mode:C++ -*-
 * NodeTypeSelect.h - Node for handling Type Selection for ULAM
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
  \file NodeTypeSelect.h - Node for handling Type Selection for ULAM
  \author Elenas S. Ackley.
  \author David H. Ackley.
  \date (C) 2015 All rights reserved.
  \gpl
*/


#ifndef NODETYPESELECT_H
#define NODETYPESELECT_H

#include "NodeType.h"

namespace MFM{

  class NodeTypeSelect : public NodeType
  {
  public:

    NodeTypeSelect(Token typetoken, UTI auti, NodeType * node, CompilerState & state);

    NodeTypeSelect(const NodeTypeSelect& ref);

    virtual ~NodeTypeSelect();

    virtual Node * instantiate();

    virtual void updateLineage(NNO pno);

    virtual bool findNodeNo(NNO n, Node *& foundNode);

    virtual void printPostfix(File * f);

    virtual const char * getName();

    virtual const std::string prettyNodeName();

    virtual UTI checkAndLabelType();

    virtual bool resolveType(UTI& rtnuti);

    virtual void countNavNodes(u32& cnt);

  private:
    NodeType * m_nodeSelect; //selected from, or null

  };

} //MFM

#endif //NODETYPESELECT_H
