/**                                        -*- mode:C++ -*-
 * UlamTypeNav.h -  Basic handling of the Nav UlamType for ULAM
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
  \file UlamTypeNav.h -  Basic handling of the Nav UlamType for ULAM
  \author Elenas S. Ackley.
  \author David H. Ackley.
  \date (C) 2014 All rights reserved.
  \gpl
*/


#ifndef ULAMTYPENAV_H
#define ULAMTYPENAV_H

#include "UlamType.h"

namespace MFM{

  class CompilerState; //forward

  class UlamTypeNav : public UlamType
  {
  public:

    UlamTypeNav(const UlamKeyTypeSignature key, CompilerState& state);
    virtual ~UlamTypeNav(){}

    virtual ULAMTYPE getUlamTypeEnum();

    virtual bool needsImmediateType();

    virtual const std::string getUlamTypeAsStringForC();

    virtual const std::string getImmediateStorageTypeAsString();

    virtual const std::string castMethodForCodeGen(UTI nodetype);

    virtual bool isComplete();  //neither bitsize nor arraysize is "unknown"

  private:

  };

}

#endif //end ULAMTYPENAV_H
