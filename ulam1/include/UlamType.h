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


#define XY(a,b,c,d) a,

  enum ULAMTYPE
  {
#include "UlamType.inc"
    LASTTYPE
  };
#undef XY

#ifndef PtrAbs
#define PtrAbs (Ptr + 2)
#endif //PtrAbs

#ifndef UAtomRef
#define UAtomRef (Ptr + 3)
#endif //UAtomRef

  struct UlamValue; //forward

  class CompilerState; //forward

  enum ULAMCLASSTYPE { UC_UNSEEN, UC_QUARK, UC_ELEMENT, UC_NOTACLASS, UC_ATOM, UC_ERROR};


  class UlamType
  {
  public:
    UlamType(const UlamKeyTypeSignature key, CompilerState& state);
    virtual ~UlamType(){}

    UlamType * getUlamType(); //returns a pointer to self

    const std::string getUlamTypeName();

    virtual const std::string getUlamTypeNameBrief();

    virtual const std::string getUlamTypeNameOnly();

    UlamKeyTypeSignature getUlamKeyTypeSignature();

    virtual bool isNumericType();

    virtual bool isPrimitiveType();

    virtual bool cast(UlamValue& val, UTI typidx);

    virtual FORECAST safeCast(UTI typidx);

    virtual FORECAST explicitlyCastable(UTI typidx);

    virtual void getDataAsString(const u32 data, char * valstr, char prefix);

    virtual void getDataLongAsString(const u64 data, char * valstr, char prefix);

    virtual s32 getDataAsCs32(const u32 data);

    virtual u32 getDataAsCu32(const u32 data);

    virtual s64 getDataAsCs64(const u64 data);

    virtual u64 getDataAsCu64(const u64 data);

    virtual s32 bitsizeToConvertTypeTo(ULAMTYPE tobUT);

    virtual ULAMTYPE getUlamTypeEnum() = 0;

    virtual const std::string getUlamTypeAsStringForC(bool useref);

    virtual const std::string getUlamTypeVDAsStringForC();

    virtual const std::string getUlamTypeMangledType();

    virtual const std::string getUlamTypeMangledName();

    virtual const std::string getUlamTypeUPrefix();

    virtual bool needsImmediateType();

    virtual const std::string getUlamTypeImmediateMangledName();

    virtual const std::string getUlamTypeImmediateAutoMangledName();

    virtual const std::string getLocalStorageTypeAsString();

    virtual const std::string getImmediateModelParameterStorageTypeAsString();

    virtual const std::string getArrayItemTmpStorageTypeAsString();

    virtual const std::string getTmpStorageTypeAsString();

    virtual const std::string getTmpStorageTypeAsString(s32 sizebyints);

    virtual STORAGE getTmpStorageTypeForTmpVar();

    virtual const char * getUlamTypeAsSingleLowercaseLetter();

    virtual const std::string castMethodForCodeGen(UTI nodetype);

    virtual void genUlamTypeMangledAutoDefinitionForC(File * fp);

    virtual void genUlamTypeMangledImmediateDefinitionForC(File * fp);

    virtual void genUlamTypeReadDefinitionForC(File * fp);

    virtual void genUlamTypeWriteDefinitionForC(File * fp);

    static const char * getUlamTypeEnumCodeChar(ULAMTYPE etype);

    static const char * getUlamTypeEnumAsString(ULAMTYPE etype);

    static ULAMTYPE getEnumFromUlamTypeString(const char * typestr);

    virtual ULAMCLASSTYPE getUlamClass();

    virtual bool isScalar(); //arraysize == NOTARRAYSIZE is scalar

    virtual bool isCustomArray();

    s32 getArraySize();

    virtual s32 getBitSize(); //except for default size constants, known after type labeling

    u32 getTotalBitSize(); //bitsize * arraysize, accounting for constants and scalars

    ALT getReferenceType();

    bool isReference();

    virtual bool isHolder();

    virtual bool isComplete(); //neither bitsize nor arraysize is "unknown"

    static ULAMTYPECOMPARERESULTS compare(UTI u1, UTI u2, CompilerState& state);

    static ULAMTYPECOMPARERESULTS compareForArgumentMatching(UTI u1, UTI u2, CompilerState& state);
    static ULAMTYPECOMPARERESULTS compareForMakingCastingNode(UTI u1, UTI u2, CompilerState& state);

    static ULAMTYPECOMPARERESULTS compareForUlamValueAssignment(UTI u1, UTI u2, CompilerState& state);

    /** Number of bits (rounded up to nearest 32 bits) required to
    hold the total bit size  */
    u32 getTotalWordSize();

    u32 getItemWordSize();

    void setTotalWordSize(u32 tw);

    void setItemWordSize(u32 iw);

    virtual bool isMinMaxAllowed();

    u64 getMax();

    s64 getMin();

    u64 getMax(UlamValue& rtnUV, UTI uti);

    s64 getMin(UlamValue& rtnUV, UTI uti);

    virtual PACKFIT getPackable();

    virtual const std::string readMethodForCodeGen();
    virtual const std::string writeMethodForCodeGen();

    virtual const std::string readArrayItemMethodForCodeGen();
    virtual const std::string writeArrayItemMethodForCodeGen();

    virtual void genUlamTypeMangledDefinitionForC(File * fp);

    virtual void genUlamTypeMangledUnpackedArrayDefinitionForC(File * fp);

    virtual void genUlamTypeMangledUnpackedArrayAutoDefinitionForC(File * fp);

    virtual void genUlamTypeMangledImmediateModelParameterDefinitionForC(File * fp);

    virtual bool genUlamTypeDefaultQuarkConstant(File * fp, u32& dqref);

  protected:
    UlamKeyTypeSignature m_key;
    CompilerState& m_state;
    u32 m_wordLengthTotal;
    u32 m_wordLengthItem;
    u64 m_max;
    s64 m_min;

  private:

    static ULAMTYPECOMPARERESULTS compareWithWildArrayItemReferenceType(UTI u1, UTI u2, CompilerState& state);
    static ULAMTYPECOMPARERESULTS compareWithWildReferenceType(UTI u1, UTI u2, CompilerState& state);


    virtual bool castTo32(UlamValue & val, UTI typidx);

    virtual bool castTo64(UlamValue & val, UTI typidx);

    bool checkArrayCast(UTI typidx);
    bool checkReferenceCast(UTI typidx);
  };

}

#endif //end ULAMTYPE_H
