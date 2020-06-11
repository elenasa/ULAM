/**                                        -*- mode:C++ -*-
 * ParsingLoopsSwitchStack.h - Parsing Control Loops and Switches in ULAM
 *
 * Copyright (C) 2017 The Regents of the University of New Mexico.
 * Copyright (C) 2017 Ackleyshack LLC.
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
  \file ParsingLoopsSwitchStack.h -  Parsing Control Loops and Switches in ULAM
  \author Elena S. Ackley.
  \author David H. Ackley.
  \date (C) 2017 All rights reserved.
  \gpl
*/

#ifndef PARSINGLOOPSSWITCHSTACK_H
#define PARSINGLOOPSSWITCHSTACK_H

#include <vector>
#include "Token.h"

namespace MFM
{

  struct LoopSwitchDesc
  {
    Token m_token;
    u32 m_tmpvarexitnum;
  };

  class ParsingLoopsSwitchStack
  {

  public:

    ParsingLoopsSwitchStack();
    ~ParsingLoopsSwitchStack();

    void push(Token & tok, u32 exitnum);
    bool okToParseABreak();
    bool okToParseAContinue();
    u32 getLastExitNumber();
    u32 getNearestBreakExitNumber();
    u32 getNearestContinueExitNumber();
    void pop();

  private:
    std::vector<struct LoopSwitchDesc> m_stack;
  };

} //MFM

#endif  /* PARSINGLOOPSSWITCHSTACK_H */
