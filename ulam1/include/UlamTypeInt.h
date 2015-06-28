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

    virtual const std::string getUlamTypeVDAsStringForC();

    virtual const std::string getUlamTypeImmediateMangledName();

    virtual const std::string getArrayItemTmpStorageTypeAsString();

    virtual const std::string getTmpStorageTypeAsString();

    virtual const std::string getTmpStorageTypeAsString(s32 sizebyints);

    const std::string getArrayItemUnsignedTmpStorageTypeAsString();

    const std::string getUnsignedTmpStorageTypeAsString();

    virtual bool cast(UlamValue & val, UTI typidx);

    virtual CASTSTAT safeCast(UTI typidx);

    virtual void genCodeAfterReadingIntoATmpVar(File * fp, UlamValue & uvpass);

    virtual void getDataAsString(const u32 data, char * valstr, char prefix);

    virtual void getDataLongAsString(const u64 data, char * valstr, char prefix);

  private:

    virtual bool castTo32(UlamValue & val, UTI typidx);

    virtual bool castTo64(UlamValue & val, UTI typidx);

    void genCodeAfterReadingArrayItemIntoATmpVar(File * fp, UlamValue & uvpass);

  };

}

#endif //end ULAMTYPEINT_H
