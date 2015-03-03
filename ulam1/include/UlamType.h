/**                                        -*- mode:C++ -*-
 * UlamType.h -  Basic handling of UlamTypes for ULAM
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
  \file UlamType.h -  Basic handling of UlamTypes for ULAM
  \author Elenas S. Ackley.
  \author David H. Ackley.
  \date (C) 2014-2015 All rights reserved.
  \gpl
*/


#ifndef ULAMTYPE_H
#define ULAMTYPE_H

#include <string>
#include <assert.h>
#include "itype.h"
#include "Constants.h"
#include "File.h"
#include "UlamKeyTypeSignature.h"

namespace MFM{


#define XX(a,b,c) a,

  enum ULAMTYPE
  {
#include "UlamType.inc"
    LASTTYPE
  };
#undef XX

  struct UlamValue; //forward

  class CompilerState; //forward

  enum ULAMCLASSTYPE { UC_UNSEEN, UC_QUARK, UC_ELEMENT, UC_NOTACLASS, UC_ATOM };


  class UlamType
  {
  public:
    UlamType(const UlamKeyTypeSignature key, CompilerState& state);
    virtual ~UlamType(){}

    /** returns a pointer to UlamType */
    UlamType * getUlamType();

    const std::string getUlamTypeName();

    virtual const std::string getUlamTypeNameBrief();

    virtual const std::string getUlamTypeNameOnly();

    //    UTI getUlamTypeIndex();

    UlamKeyTypeSignature getUlamKeyTypeSignature();

    virtual bool cast(UlamValue& val, UTI typidx);

    virtual void getDataAsString(const u32 data, char * valstr, char prefix);

    virtual ULAMCLASSTYPE getUlamClass();

    virtual ULAMTYPE getUlamTypeEnum() = 0;

    virtual const std::string getUlamTypeAsStringForC();

    virtual const std::string getUlamTypeVDAsStringForC();

    virtual const std::string getUlamTypeMangledType();

    virtual const std::string getUlamTypeMangledName();

    virtual const std::string getUlamTypeUPrefix();

    virtual const std::string getUlamTypeImmediateMangledName();

    virtual const std::string getUlamTypeImmediateAutoMangledName();

    virtual bool needsImmediateType();

    virtual const std::string getImmediateStorageTypeAsString();

    virtual const std::string getArrayItemTmpStorageTypeAsString();

    virtual const std::string getTmpStorageTypeAsString();

    virtual const std::string getTmpStorageTypeAsString(s32 sizebyints);

    virtual const char * getUlamTypeAsSingleLowercaseLetter();

    virtual void genUlamTypeMangledDefinitionForC(File * fp);

    virtual void genUlamTypeMangledImmediateDefinitionForC(File * fp);

    virtual void genUlamTypeReadDefinitionForC(File * fp);

    virtual void genUlamTypeWriteDefinitionForC(File * fp);


    static const char * getUlamTypeEnumAsString(ULAMTYPE etype);

    static ULAMTYPE getEnumFromUlamTypeString(const char * typestr);

    //for name by index see CompilerState::getUlamTypeNameByIndex
    virtual bool isConstant();

    virtual bool isScalar();   //arraysize == NOTARRAYSIZE is scalar

    virtual bool isCustomArray();

    s32 getArraySize();

    virtual s32 getBitSize();  //'class' type calculated after type labeling; default size for constants

    u32 getTotalBitSize();  //bitsize * arraysize, accounting for constants and scalars

    bool isComplete();  //neither bitsize nor arraysize is "unknown"

    static ULAMTYPECOMPARERESULTS compare(UTI u1, UTI u2, CompilerState& state);

    /** Number of bits (rounded up to nearest 32 bits) required to
    hold the total bit size  */
    u32 getTotalWordSize();

    u32 getItemWordSize();

    void setTotalWordSize(u32 tw);

    void setItemWordSize(u32 iw);

    virtual bool isMinMaxAllowed();

    u32 getMax();

    s32 getMin();

    virtual PACKFIT getPackable();

    virtual const std::string readMethodForCodeGen();
    virtual const std::string writeMethodForCodeGen();

    virtual const std::string readArrayItemMethodForCodeGen();
    virtual const std::string writeArrayItemMethodForCodeGen();

    virtual void genCodeAfterReadingIntoATmpVar(File * fp, UlamValue & uvpass);

    virtual const std::string castMethodForCodeGen(UTI nodetype);

    virtual void genUlamTypeMangledAutoDefinitionForC(File * fp);

  protected:
    UlamKeyTypeSignature m_key;
    CompilerState& m_state;
    u32 m_wordLengthTotal;
    u32 m_wordLengthItem;
    u32 m_max;
    s32 m_min;

  private:
  };

}

#endif //end ULAMTYPE_H
