/**                                        -*- mode:C++ -*-
 * UlamType.h -  Basic handling of UlamTypes for ULAM
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
  \file UlamType.h -  Basic handling of UlamTypes for ULAM
  \author Elenas S. Ackley.
  \author David H. Ackley.
  \date (C) 2014-2017 All rights reserved.
  \gpl
*/


#ifndef ULAMTYPEPRIMITIVE_H
#define ULAMTYPEPRIMITIVE_H

#include "UlamType.h"

namespace MFM{


  class CompilerState; //forward

  class UlamTypePrimitive : public UlamType
  {
  public:
    UlamTypePrimitive(const UlamKeyTypeSignature key, CompilerState& state);

    virtual ~UlamTypePrimitive(){}

    virtual bool isNumericType();

    virtual bool isPrimitiveType();

    virtual bool cast(UlamValue& val, UTI typidx) = 0;

    virtual FORECAST safeCast(UTI typidx);

    virtual FORECAST explicitlyCastable(UTI typidx);

    virtual void getDataAsString(const u32 data, char * valstr, char prefix) = 0;

    virtual void getDataLongAsString(const u64 data, char * valstr, char prefix) = 0;

    virtual s32 getDataAsCs32(const u32 data) = 0;

    virtual u32 getDataAsCu32(const u32 data) = 0;

    virtual s64 getDataAsCs64(const u64 data) = 0;

    virtual u64 getDataAsCu64(const u64 data) = 0;

    virtual s32 bitsizeToConvertTypeTo(ULAMTYPE tobUT);

    virtual ULAMTYPE getUlamTypeEnum() = 0;

    virtual const std::string getUlamTypeMangledName();

    virtual const std::string getUlamTypeUPrefix();

    virtual bool needsImmediateType();

    virtual const std::string getLocalStorageTypeAsString();

    virtual const std::string getImmediateModelParameterStorageTypeAsString();

    virtual TMPSTORAGE getTmpStorageTypeForTmpVar();

    virtual const std::string castMethodForCodeGen(UTI nodetype);

    virtual void genUlamTypeMangledAutoDefinitionForC(File * fp);

    virtual void genUlamTypeAutoReadDefinitionForC(File * fp);

    virtual void genUlamTypeAutoWriteDefinitionForC(File * fp);

    virtual ULAMCLASSTYPE getUlamClassType();

    virtual bool isMinMaxAllowed();

    virtual u64 getMax();

    virtual s64 getMin();

    virtual u64 getMax(UlamValue& rtnUV, UTI uti);

    virtual s64 getMin(UlamValue& rtnUV, UTI uti);

    virtual PACKFIT getPackable();

    virtual void genUlamTypeMangledDefinitionForC(File * fp);

    virtual void genUlamTypeReadDefinitionForC(File * fp);

    virtual void genUlamTypeWriteDefinitionForC(File * fp);

    virtual void genUlamTypeMangledImmediateModelParameterDefinitionForC(File * fp);

  protected:
    u64 m_max;
    s64 m_min;

  private:

    virtual bool castTo32(UlamValue & val, UTI typidx) = 0;

    virtual bool castTo64(UlamValue & val, UTI typidx) = 0;

  };

}

#endif //end ULAMTYPE_H
