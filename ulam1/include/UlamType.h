/**                                        -*- mode:C++ -*-
 * UlamType.h -  Basic handling of UlamTypes for ULAM
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
  \file UlamType.h -  Basic handling of UlamTypes for ULAM
  \author Elenas S. Ackley.
  \author David H. Ackley.
  \date (C) 2014 All rights reserved.
  \gpl
*/


#ifndef ULAMTYPE_H
#define ULAMTYPE_H

#include <string>
#include <assert.h>
#include "itype.h"
#include "UlamKeyTypeSignature.h"

namespace MFM{


#define XX(a,b) a,

  enum ULAMTYPE
  {
#include "UlamType.inc"
    LASTTYPE
  };
#undef XX

  struct UlamValue; //forward


#define UTI u32

  class CompilerState; //forward

  class UlamType
  {    
  public: 
    UlamType(const UlamKeyTypeSignature key, const UTI uti);
    ~UlamType(){}

    virtual void newValue(UlamValue & val) = 0;
    
    virtual void deleteValue(UlamValue * val) = 0;

    /** returns a pointer to UlamType */
    UlamType * getUlamType();
    
    const std::string getUlamTypeName();

    const char * getUlamTypeNameBrief();

    UTI getUlamTypeIndex();

    UlamKeyTypeSignature getUlamKeyTypeSignature();

    virtual bool cast(UlamValue& val) = 0;

    virtual void getUlamValueAsString(const UlamValue & val, char * valstr, CompilerState& state) = 0;

    virtual bool isZero(const UlamValue & val) = 0;

    virtual ULAMTYPE getUlamTypeEnum() = 0;

    static const char * getUlamTypeEnumAsString(ULAMTYPE etype);

    static ULAMTYPE getEnumFromUlamTypeString(const char * typestr);

    //for name by index see CompilerState::getUlamTypeNameByIndex

    bool isScalar(); //arraysize == 0 is scalar

    u32 getArraySize();

    u32 getBitSize();
  protected:
    UlamKeyTypeSignature m_key;
    UTI m_index;

  private:
  };

}

#endif //end ULAMTYPE_H
