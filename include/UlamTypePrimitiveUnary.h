/**                                        -*- mode:C++ -*-
 * UlamTypePrimitiveUnary.h -  Basic handling of the Unary Primitive UlamType for ULAM
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
  \file UlamTypePrimitiveUnary.h -  Basic handling of the Unary Primitive UlamType for ULAM
  \author Elena S. Ackley.
  \author David H. Ackley.
  \date (C) 2014-2017 All rights reserved.
  \gpl
*/


#ifndef ULAMTYPEPRIMITIVEUNARY_H
#define ULAMTYPEPRIMITIVEUNARY_H

#include "UlamTypePrimitive.h"

namespace MFM{

  class CompilerState; //forward

  class UlamTypePrimitiveUnary : public UlamTypePrimitive
  {
  public:

    UlamTypePrimitiveUnary(const UlamKeyTypeSignature key, CompilerState& state);

    virtual ~UlamTypePrimitiveUnary(){}

    virtual ULAMTYPE getUlamTypeEnum();

    virtual bool isNumericType();

    virtual bool cast(UlamValue & val, UTI typidx);

    virtual FORECAST safeCast(UTI typidx);

    virtual void getDataAsString(const u32 data, char * valstr, char prefix);

    virtual void getDataLongAsString(const u64 data, char * valstr, char prefix);

    virtual s32 getDataAsCs32(const u32 data);

    virtual u32 getDataAsCu32(const u32 data);

    virtual s64 getDataAsCs64(const u64 data);

    virtual u64 getDataAsCu64(const u64 data);

    virtual s32 bitsizeToConvertTypeTo(ULAMTYPE tobUT);

  private:

    virtual bool castTo32(UlamValue & val, UTI typidx);

    virtual bool castTo64(UlamValue & val, UTI typidx);


  };

} //MFM
#endif //end ULAMTYPEPRIMITIVEUNARY_H
