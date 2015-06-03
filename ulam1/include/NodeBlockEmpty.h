/**                                        -*- mode:C++ -*-
 * NodeBlockEmpty.h - Node for handling Empty Blocks for ULAM
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
  \file NodeBlockEmpty.h - Node for handling Empty Blocks for ULAM
  \author Elenas S. Ackley.
  \author David H. Ackley.
  \date (C) 2014-2015 All rights reserved.
  \gpl
*/


#ifndef NODEBLOCKEMPTY_H
#define NODEBLOCKEMPTY_H

#include "NodeBlock.h"


namespace MFM{

  class NodeBlockEmpty : public NodeBlock
  {
  public:

    NodeBlockEmpty(NodeBlock * prevBlockNode, CompilerState & state);
    NodeBlockEmpty(const NodeBlockEmpty& ref);
    virtual ~NodeBlockEmpty();

    virtual Node * instantiate();

    virtual void print(File * fp);

    virtual void printPostfix(File * fp);

    virtual const char * getName();

    virtual const std::string prettyNodeName();

    virtual UTI checkAndLabelType();

    virtual void countNavNodes(u32& cnt);

    virtual EvalStatus eval();

    virtual void calcMaxDepth(u32& depth, u32& maxdepth, s32 base);

    virtual void genCode(File * fp, UlamValue& uvpass);


  protected:


  private:

  };

}

#endif //end NODEBLOCKEMPTY_H
