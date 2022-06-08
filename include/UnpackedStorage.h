/**                                        -*- mode:C++ -*-
 * UnpackedStorage.h -  Basic handling of unpacked storage for ULAM
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
  \file UnpackedStorage.h -  Basic handling of unpacked storage for ULAM
  \author Elena S. Ackley.
  \author David H. Ackley.
  \date (C) 2014-2017 All rights reserved.
  \gpl
*/


#ifndef UNPACKEDSTORAGE_H
#define UNPACKEDSTORAGE_H

#include <vector>
#include "itype.h"
#include "UlamValue.h"

namespace MFM
{

  class CompilerState;  //forward

 //similar to CallStack, except for dataMembers
  class UnpackedStorage
  {
  public:
    UnpackedStorage(CompilerState& state);
    ~UnpackedStorage();

    void init(UTI intType);

    //returns baseIndex of array, or index of scalar
    u32 pushDataMember(UTI arrayType, UTI scalarType);

    UlamValue loadDataMemberAt(u32 idx);
    void storeDataMemberAt(UlamValue uv, u32 idx);

    void assignUlamValue(UlamValue luv, UlamValue ruv);
    void assignUlamValuePtr(UlamValue pluv, UlamValue puv);

  private:
    std::vector<UlamValue> m_values;
    CompilerState& m_state;

  };

}

#endif //end UNPACKEDSTORAGE_H
