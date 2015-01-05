/**                                        -*- mode:C++ -*-
 * UlamTypeClass.h -  Basic handling of the Class UlamType for ULAM
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
  \file UlamTypeClass.h -  Basic handling of the Class UlamType for ULAM
  \author Elenas S. Ackley.
  \author David H. Ackley.
  \date (C) 2014 All rights reserved.
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

    UlamTypeClass(const UlamKeyTypeSignature key, const UTI uti, ULAMCLASSTYPE type = UC_INCOMPLETE);

    virtual ~UlamTypeClass(){}

    virtual ULAMTYPE getUlamTypeEnum();

    virtual bool cast(UlamValue& val, CompilerState& state);

    virtual const std::string getUlamTypeImmediateMangledName(CompilerState * state);

    virtual const std::string getUlamTypeImmediateAutoMangledName(CompilerState * state);

    virtual bool needsImmediateType();

    virtual bool isMinMaxAllowed();

    virtual PACKFIT getPackable();

    virtual const std::string getTmpStorageTypeAsString(CompilerState * state);

    virtual const std::string getArrayItemTmpStorageTypeAsString(CompilerState * state);

    virtual const std::string getImmediateStorageTypeAsString(CompilerState * state);

    virtual const char * getUlamTypeAsSingleLowercaseLetter();

    virtual void genUlamTypeMangledDefinitionForC(File * fp, CompilerState * state);

    virtual void genUlamTypeReadDefinitionForC(File * fp, CompilerState * state);

    virtual void genUlamTypeWriteDefinitionForC(File * fp, CompilerState * state);

    virtual const std::string getUlamTypeVDAsStringForC();

    virtual const std::string getUlamTypeAsStringForC();

    virtual const std::string getUlamTypeUPrefix();

    virtual void getDataAsString(const u32 data, char * valstr, char prefix, CompilerState& state);

    virtual ULAMCLASSTYPE getUlamClass();

    void setUlamClass(ULAMCLASSTYPE type);

    virtual bool isConstant();

    virtual bool isScalar();

    virtual bool isCustomArray();

    void setCustomArrayType(UTI type);

    UTI getCustomArrayType();

    virtual s32 getBitSize();

    virtual const std::string getUlamTypeNameBrief(CompilerState * state);

    virtual const std::string readArrayItemMethodForCodeGen();

    virtual const std::string writeArrayItemMethodForCodeGen();

    virtual const std::string castMethodForCodeGen(UTI nodetype, CompilerState& state);

    virtual void genUlamTypeMangledAutoDefinitionForC(File * fp, CompilerState * state);

   private:

    ULAMCLASSTYPE m_class;
    bool m_customArray;
    UTI m_customArrayType;

    void genCustomArrayMangledDefinitionForC(File * fp, CompilerState * state);

    void genUlamTypeQuarkMangledDefinitionForC(File * fp, CompilerState * state);
    void genUlamTypeQuarkReadDefinitionForC(File * fp, CompilerState * state);
    void genUlamTypeQuarkWriteDefinitionForC(File * fp, CompilerState * state);

    void genUlamTypeElementMangledDefinitionForC(File * fp, CompilerState * state);
    void genUlamTypeElementReadDefinitionForC(File * fp, CompilerState * state);
    void genUlamTypeElementWriteDefinitionForC(File * fp, CompilerState * state);

  };

}

#endif //end ULAMTYPECLASS_H
