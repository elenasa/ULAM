/**                                        -*- mode:C++ -*-
 * ParameterListOfNodes.h - Vector for handling parameter nodes for ULAM
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
  \file ParameterListOfNodes.h - Vector for handling parameter nodes for ULAM
  \author Elenas S. Ackley.
  \author David H. Ackley.
  \date (C) 2015 All rights reserved.
  \gpl
*/


#ifndef PARAMETERLISTOFNODES_H
#define PARAMETERLISTOFNODES_H

#include "Node.h"
#include "UlamType.h"
#include <vector>

namespace MFM{

  struct CompilerState; //forward

  class ParameterListOfNodes
  {
  public:

    ParameterListOfNodes();

    ParameterListOfNodes(const ParameterListOfNodes& ref);

    ~ParameterListOfNodes();

    ParameterListOfNodes * clone();

    void addParameterNode(Node * argNode);

    u32 getNumberOfParameterNodes() const;

    Node * getParameterNodePtr (u32 n) const;

    void updateParameterLineage(NNO pno);

    bool findParameterNodeNo(NNO n, Node *& foundNode);

    //return true when all parameter types are complete
    bool checkAndLabelTypesOfParameterNodes(CompilerState& state);

    UTI getParameterNodeType(u32 n);

    void countNavNodeTypes(u32& cnt);

  protected:

  private:
    std::vector<Node *> m_parameterNodes; //variable or function can be an args
  };

} //MFM

#endif //PARAMETERLISTOFNODES_H
