/**                                        -*- mode:C++ -*-
 * UlamTypeInt.h -  Basic handling of the Integer UlamType for ULAM
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
  \file UlamTypeInt.h -  Basic handling of the Integer UlamType for ULAM
  \author Elenas S. Ackley.
  \author David H. Ackley.
  \date (C) 2014-2015 All rights reserved.
  \gpl
*/


#ifndef ULAMTYPEINT_H
#define ULAMTYPEINT_H

#include "UlamType.h"

namespace MFM{

  class CompilerState; //forward

  class UlamTypeInt : public UlamType
  {
  public:

    UlamTypeInt(const UlamKeyTypeSignature key, CompilerState& state);

    virtual ~UlamTypeInt(){}

    virtual ULAMTYPE getUlamTypeEnum();

    virtual bool isNumericType();

    virtual const std::string getUlamTypeVDAsStringForC();

    virtual bool cast(UlamValue & val, UTI typidx);

    virtual FORECAST safeCast(UTI typidx);

    virtual void getDataAsString(const u32 data, char * valstr, char prefix);

    virtual void getDataLongAsString(const u64 data, char * valstr, char prefix);

    virtual s32 getDataAsCs32(const u32 data);

    virtual u32 getDataAsCu32(const u32 data);

    virtual s64 getDataAsCs64(const u64 data);

    virtual u64 getDataAsCu64(const u64 data);

  private:

    virtual bool castTo32(UlamValue & val, UTI typidx);

    virtual bool castTo64(UlamValue & val, UTI typidx);

  };

}

#endif //end ULAMTYPEINT_H
