/**                                        -*- mode:C++ -*-
 * NodeBlockSwitch.h - Node for handling Switch Blocks for ULAM
 *
 * Copyright (C) 2017-2019 The Regents of the University of New Mexico.
 * Copyright (C) 2017-2021 Ackleyshack LLC.
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
  \file NodeBlockSwitch.h - Node for handling Switch Blocks for ULAM
  \author Elena S. Ackley.
  \author David H. Ackley.
  \date (C) 2017-2021 All rights reserved.
  \gpl
*/


#ifndef NODEBLOCKSWITCH_H
#define NODEBLOCKSWITCH_H

#include "NodeBlock.h"


namespace MFM{

  class NodeBlockSwitch : public NodeBlock
  {
  public:

    NodeBlockSwitch(NodeBlock * prevBlockNode, u32 switchnum, CompilerState & state);
    NodeBlockSwitch(const NodeBlockSwitch& ref);
    virtual ~NodeBlockSwitch();

    virtual Node * instantiate();

    virtual const std::string prettyNodeName();

    virtual UTI checkAndLabelType(Node * thisparentnode);

    virtual void genCode(File * fp, UVPass& uvpass);

    virtual bool isASwitchBlock();

    u32 getSwitchNumber();

    NNO getDefaultCaseNodeNo();

    void setDefaultCaseNodeNo(NNO dnno);


  protected:


  private:

    u32 m_switchnum; //condition variable
    NNO m_defaultcaseNodeNo;

  };

}

#endif //end NODEBLOCKSWITCH_H
