/**                                        -*- mode:C++ -*-
 * NodeBlockInvisible.h - Node for handling Invisible Blocks for ULAM
 *
 * Copyright (C) 2014-2017 The Regents of the University of New Mexico.
 * Copyright (C) 2014-2017 Ackleyshack LLC.
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
  \file NodeBlockInvisible.h - Node for handling Invisible Blocks for ULAM
  \author Elena S. Ackley.
  \author David H. Ackley.
  \date (C) 2014-2017 All rights reserved.
  \gpl
*/


#ifndef NODEBLOCKINVISIBLE_H
#define NODEBLOCKINVISIBLE_H

#include "NodeBlock.h"


namespace MFM{

  //block created to assist with parsing single statements;
  //no need to show off their curly's {}
  class NodeBlockInvisible : public NodeBlock
  {
  public:

    NodeBlockInvisible(NodeBlock * prevBlockNode, CompilerState & state);
    NodeBlockInvisible(const NodeBlockInvisible& ref);
    virtual ~NodeBlockInvisible();

    virtual Node * instantiate();

    virtual void print(File * fp);

    virtual void printPostfix(File * fp);

    virtual const char * getName();

    virtual const std::string prettyNodeName();

    virtual void genCode(File * fp, UVPass& uvpass);


  protected:


  private:

  };

}

#endif //end NODEBLOCKINVISIBLE_H
