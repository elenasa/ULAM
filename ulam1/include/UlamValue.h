/**                                        -*- mode:C++ -*-
 * UlamValue.h -  Basic handling of UlamValues for ULAM
 *
 * Copyright (C) 2014 The Regents of the University of New Mexico.
 * Copyright (C) 2014 Ackleyshack LLC.
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
  \file UlamValue.h -  Basic handling of UlamValues for ULAM
  \author Elenas S. Ackley.
  \author David H. Ackley.
  \date (C) 2014 All rights reserved.
  \gpl
*/


#ifndef ULAMVALUE_H
#define ULAMVALUE_H

#include "itype.h"

namespace MFM{

  enum STORAGE { IMMEDIATE = 0, ATOM, STACK, EVALRETURN};

  class UlamType;      //forward
  class CompilerState; //forward

  struct UlamValue
  {
    UlamType * m_utype;
    union{
      s32 m_valInt;
      bool m_valBool;
      float m_valFloat;
      s32 m_baseArraySlotIndex;
    };
    STORAGE m_storage;
    
    UlamValue();   //requires init to avoid Null ptr for type
    UlamValue(UlamType * utype, s32 v, STORAGE place);
    UlamValue(UlamType * utype, bool v, STORAGE place);
    UlamValue(UlamType * utype, float v, STORAGE place);
    UlamValue(UlamType * arrayUType, s32 slot, bool headerOnly, STORAGE place);

    ~UlamValue();

    void init(UlamType * utype, s32 v);
    void init(UlamType * utype, bool v);
    void init(UlamType * utype, float v);
    void init(UlamType * arrayUType, s32 baseArrayIndex, bool headerOnly, STORAGE place);

    UlamType * getUlamValueType();
    void getUlamValueAsString(char * valstr, CompilerState * state);
    bool isZero();
    u32 isArraySize();

    UlamValue getValAt(s32 arrayindex, CompilerState * state) const;
  };

}

#endif //end ULAMVALUE_H
