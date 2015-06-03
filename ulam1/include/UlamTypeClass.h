/**                                        -*- mode:C++ -*-
 * UlamTypeClass.h -  Basic handling of the Class UlamType for ULAM
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
  \file UlamTypeClass.h -  Basic handling of the Class UlamType for ULAM
  \author Elenas S. Ackley.
  \author David H. Ackley.
  \date (C) 2014-2015 All rights reserved.
  \gpl
*/


#ifndef ULAMTYPECLASS_H
#define ULAMTYPECLASS_H

#include "UlamType.h"

namespace MFM{

  class CompilerState; //forward

  class UlamTypeClass : public UlamType
  {
  public:

    UlamTypeClass(const UlamKeyTypeSignature key, CompilerState& state, ULAMCLASSTYPE type = UC_UNSEEN);

    virtual ~UlamTypeClass(){}

    virtual ULAMTYPE getUlamTypeEnum();

    virtual bool cast(UlamValue& val, UTI typidx);

    virtual const char * getUlamTypeAsSingleLowercaseLetter();

    virtual const std::string getUlamTypeVDAsStringForC();

    virtual const std::string getUlamTypeMangledType();

    virtual  const std::string getUlamTypeMangledName();

    virtual const std::string getUlamTypeAsStringForC();

    virtual const std::string getUlamTypeUPrefix();

    virtual const std::string getUlamTypeNameBrief();

    virtual void getDataAsString(const u32 data, char * valstr, char prefix);

    virtual void getDataLongAsString(const u64 data, char * valstr, char prefix);

    virtual ULAMCLASSTYPE getUlamClass();

    void setUlamClass(ULAMCLASSTYPE type);

    virtual bool isScalar();

    virtual bool isCustomArray();

    void setCustomArray();

    UTI getCustomArrayType();

    virtual s32 getBitSize();

    virtual bool isHolder();

    virtual bool isComplete();

    virtual bool isMinMaxAllowed();

    virtual PACKFIT getPackable();

    virtual bool needsImmediateType();

    virtual const std::string getUlamTypeImmediateMangledName();

    virtual const std::string getUlamTypeImmediateAutoMangledName();

    virtual const std::string getTmpStorageTypeAsString();

    virtual const std::string getArrayItemTmpStorageTypeAsString();

    virtual const std::string getImmediateStorageTypeAsString();

    virtual void genUlamTypeReadDefinitionForC(File * fp);

    virtual void genUlamTypeWriteDefinitionForC(File * fp);

    virtual const std::string readArrayItemMethodForCodeGen();

    virtual const std::string writeArrayItemMethodForCodeGen();

    virtual const std::string castMethodForCodeGen(UTI nodetype);

    virtual void genUlamTypeMangledDefinitionForC(File * fp);

    virtual void genUlamTypeMangledAutoDefinitionForC(File * fp);

   private:

    ULAMCLASSTYPE m_class;
    bool m_customArray;

    void genCustomArrayMangledDefinitionForC(File * fp);

    void genUlamTypeQuarkMangledDefinitionForC(File * fp);
    void genUlamTypeQuarkReadDefinitionForC(File * fp);
    void genUlamTypeQuarkWriteDefinitionForC(File * fp);

    void genUlamTypeElementMangledDefinitionForC(File * fp);
    void genUlamTypeElementReadDefinitionForC(File * fp);
    void genUlamTypeElementWriteDefinitionForC(File * fp);

  };

}

#endif //end ULAMTYPECLASS_H
