/**                                        -*- mode:C++ -*-
 * UlamTypeUnary.h -  Basic handling of the Unary UlamType for ULAM
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
  \file UlamTypeUnary.h -  Basic handling of the Unary UlamType for ULAM
  \author Elenas S. Ackley.
  \author David H. Ackley.
  \date (C) 2014 All rights reserved.
  \gpl
*/


#ifndef ULAMTYPEUNARY_H
#define ULAMTYPEUNARY_H

#include "UlamType.h"

namespace MFM{

  class CompilerState; //forward

  class UlamTypeUnary : public UlamType
  {
  public:

    UlamTypeUnary(const UlamKeyTypeSignature key, const UTI uti);
    virtual ~UlamTypeUnary(){}

    virtual ULAMTYPE getUlamTypeEnum();

    virtual const std::string getUlamTypeAsStringForC();

    virtual const char * getUlamTypeAsSingleLowercaseLetter();

    virtual bool cast(UlamValue & val, CompilerState& state);
    virtual bool castBitSize(UlamValue & val, CompilerState& state);

    virtual void getUlamValueAsString(const UlamValue & val, char * valstr, CompilerState& state);

  private:

  };

}

#endif //end ULAMTYPEUNARY_H
