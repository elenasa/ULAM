/**                                        -*- mode:C++ -*-
 * ClassContextStack.h - Class Context Stack handling for ULAM
 *
 * Copyright (C) 2015-2017 The Regents of the University of New Mexico.
 * Copyright (C) 2015-2017 Ackleyshack LLC.
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
  \file ClassContextStack.h - Class Context Stack handling for ULAM
  \author Elena S. Ackley.
  \author David H. Ackley.
  \date (C) 2015-2017 All rights reserved.
  \gpl
*/

#ifndef CLASSCONTEXTSTACK_H
#define CLASSCONTEXTSTACK_H

#include <vector>
#include "itype.h"
#include "ClassContext.h"

namespace MFM
{
  class ClassContextStack
  {
  public:

    ClassContextStack();
    ~ClassContextStack();

    bool getCurrentClassContext(ClassContext & contextref); //no change to stack

    void pushClassContext(ClassContext context);

    bool popClassContext(ClassContext & contextref);

    void popClassContext();

    u32 getClassContextStackSize();

    u32 countClassContextOnStack(const NodeBlockClass * cblock);

  private:
    std::vector<ClassContext> m_contexts;
  };

} //MFM

#endif  /* CLASSCONTEXTSTACK_H */
