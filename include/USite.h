/**                                        -*- mode:C++ -*-
 * USite.h -  Basic handling of a Site for ULAM
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
  \file USite.h -  Basic handling of a Site for ULAM
  \author Elena S. Ackley.
  \author David H. Ackley.
  \date (C) 2014-2017 All rights reserved.
  \gpl
*/


#ifndef USITE_H
#define USITE_H

#include "itype.h"
#include "UlamValue.h"

namespace MFM
{

  class CompilerState;  //forward

  //single site storage within an event window
  class USite
  {
  public:
    USite();
    ~USite();

    void init();

    UTI getElementTypeNumber(); //passes through to AtomValue at site
    void setElementTypeNumber(UTI type);

    void setElementTypeAndDefaults(UTI type, CompilerState& state);

    bool isSiteLive();
    void setSiteLive(bool b);

    UlamValue getSiteUlamValue();
    void setSiteUlamValue(UlamValue uv);

  private:
    bool m_live;
    UlamValue m_site;  //only of type UlamTypeAtom, contains Atom (bitVector(96))
  };

}

#endif //USITE_H
