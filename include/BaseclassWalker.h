/**                                        -*- mode:C++ -*-
 * BaseclassWalker.h - Traverse a family tree for ULAM
 *
 * Copyright (C) 2019 The Regents of the University of New Mexico.
 * Copyright (C) 2019 Ackleyshack LLC.
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
  \file BaseclassWalker.h -  Traverse a family tree for ULAM
  \author Elenas S. Ackley.
  \author David H. Ackley.
  \date (C) 2019 All rights reserved.
  \gpl
*/

#ifndef BASECLASSWALKER_H
#define BASECLASSWALKER_H

#include <deque>
#include <set>
#include "SymbolClass.h"

namespace MFM
{
  class SymbolClass; //forward
  class BaseclassWalker
  {
  public:

    BaseclassWalker();
    BaseclassWalker(bool breadthfirst);

    ~BaseclassWalker();

    void init(UTI cuti);
    bool isDone();
    void addAncestorsOf(SymbolClass * csymptr);
    bool getNextBase(UTI& nextbase);

  private:

    std::deque<UTI> m_bases;
    std::set<UTI> m_seenset;
    bool m_breadthfirst;


  };

}
#endif  /* BASECLASSWALKER_H */
