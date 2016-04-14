/**                                        -*- mode:C++ -*-
 * UlamTypeClassQuark.h -  Basic handling of the Quark Class UlamType for ULAM
 *
 * Copyright (C) 2014-2016 The Regents of the University of New Mexico.
 * Copyright (C) 2014-2016 Ackleyshack LLC.
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
  \file UlamTypeClassQuark.h -  Basic handling of the Quark Class UlamType for ULAM
  \author Elenas S. Ackley.
  \author David H. Ackley.
  \date (C) 2014-2016 All rights reserved.
  \gpl
*/


#ifndef ULAMTYPECLASSQUARK_H
#define ULAMTYPECLASSQUARK_H

#include "UlamTypeClass.h"

namespace MFM{

  class CompilerState; //forward

  class UlamTypeClassQuark : public UlamTypeClass
  {
  public:

    UlamTypeClassQuark(const UlamKeyTypeSignature key, CompilerState& state);

    virtual ~UlamTypeClassQuark(){}

    virtual bool isNumericType();

    virtual ULAMCLASSTYPE getUlamClassType();

    virtual bool cast(UlamValue& val, UTI typidx);

    virtual const char * getUlamTypeAsSingleLowercaseLetter();

    virtual const std::string getUlamTypeUPrefix();

    virtual const std::string readMethodForCodeGen();

    virtual const std::string writeMethodForCodeGen();

    virtual bool needsImmediateType();

    virtual const std::string getLocalStorageTypeAsString();

    virtual STORAGE getTmpStorageTypeForTmpVar();

    virtual const std::string castMethodForCodeGen(UTI nodetype);

    virtual void genUlamTypeMangledAutoDefinitionForC(File * fp);

    virtual void genUlamTypeAutoReadDefinitionForC(File * fp);

    virtual void genUlamTypeAutoWriteDefinitionForC(File * fp);

    virtual void genUlamTypeMangledDefinitionForC(File * fp);

    virtual void genUlamTypeReadDefinitionForC(File * fp);

    virtual void genUlamTypeWriteDefinitionForC(File * fp);

    virtual void genUlamTypeMangledUnpackedArrayAutoDefinitionForC(File * fp);

    virtual void genUlamTypeMangledUnpackedArrayDefinitionForC(File * fp);

    virtual bool genUlamTypeDefaultQuarkConstant(File * fp, u32& dqref);

   private:

  };

}

#endif //end ULAMTYPECLASSQUARK_H
