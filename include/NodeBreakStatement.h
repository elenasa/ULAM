/**                                        -*- mode:C++ -*-
 * NodeBreakStatement.h - Node handling the Break Statement for ULAM
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
  \file NodeBreakStatement.h - Node handling the Break Statement for ULAM
  \author Elena S. Ackley.
  \author David H. Ackley.
  \date (C) 2014-2017 All rights reserved.
  \gpl
*/


#ifndef NODEBREAKSTATEMENT_H
#define NODEBREAKSTATEMENT_H

#include "File.h"
#include "Node.h"

namespace MFM{

  class NodeBreakStatement : public Node
  {
  public:

    NodeBreakStatement(CompilerState & state);
    NodeBreakStatement(s32 gotolabelnum, CompilerState & state);
    NodeBreakStatement(const NodeBreakStatement& ref);
    virtual ~NodeBreakStatement();

    virtual Node * instantiate();

    virtual void print(File * fp);

    virtual void printPostfix(File * fp);

    virtual UTI checkAndLabelType();

    virtual EvalStatus eval();

    virtual const char * getName();

    virtual const std::string prettyNodeName();

    virtual void genCode(File * fp, UVPass& uvpass);

  protected:

  private:
    s32 m_gotolabelnum;
  };

}

#endif //end NODEBREAKSTATEMENT_H
