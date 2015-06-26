/**                                        -*- mode:C++ -*-
 * UlamTypeBits.h -  Basic handling of the Bits UlamType for ULAM
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
  \file UlamTypeBits.h -  Basic handling of the Bits UlamType for ULAM
  \author Elenas S. Ackley.
  \author David H. Ackley.
  \date (C) 2014-2015 All rights reserved.
  \gpl
*/


#ifndef ULAMTYPEBITS_H
#define ULAMTYPEBITS_H

#include "UlamType.h"

namespace MFM{

  class CompilerState; //forward

  class UlamTypeBits : public UlamType
  {
  public:

    UlamTypeBits(const UlamKeyTypeSignature key, CompilerState& state);
    virtual ~UlamTypeBits(){}

    virtual ULAMTYPE getUlamTypeEnum();

    virtual const std::string getUlamTypeVDAsStringForC();

    virtual bool isMinMaxAllowed();

    virtual bool cast(UlamValue & val, UTI typidx);

    virtual SAFECAST safeCast(UTI typidx);

    virtual void getDataAsString(const u32 data, char * valstr, char prefix);

    virtual void getDataLongAsString(const u64 data, char * valstr, char prefix);

  private:

    virtual bool castTo32(UlamValue & val, UTI typidx);

    virtual bool castTo64(UlamValue & val, UTI typidx);


  };

}

#endif //end ULAMTYPEBITS_H
