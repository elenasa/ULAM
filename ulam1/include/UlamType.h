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


#define UTI u16

  class CompilerState; //forward

  enum ULAMCLASSTYPE { UC_INCOMPLETE, UC_QUARK, UC_ELEMENT, UC_NOTACLASS };


  class UlamType
  {
  public:
    UlamType(const UlamKeyTypeSignature key, const UTI uti);
    virtual ~UlamType(){}

    /** returns a pointer to UlamType */
    UlamType * getUlamType();

    const std::string getUlamTypeName(CompilerState * state);

    virtual const std::string getUlamTypeNameBrief(CompilerState * state);

    UTI getUlamTypeIndex();

    UlamKeyTypeSignature getUlamKeyTypeSignature();

    virtual bool cast(UlamValue& val, CompilerState& state);

    virtual void getDataAsString(const u32 data, char * valstr, char prefix, CompilerState& state);

    virtual ULAMCLASSTYPE getUlamClass();

    virtual ULAMTYPE getUlamTypeEnum() = 0;

    virtual const std::string getUlamTypeAsStringForC();

    virtual const std::string getUlamTypeVDAsStringForC();

    virtual const std::string getUlamTypeMangledName(CompilerState * state);

    virtual const std::string getUlamTypeUPrefix();

    virtual const std::string getUlamTypeImmediateMangledName(CompilerState * state);

    virtual bool needsImmediateType();

    virtual const std::string getImmediateStorageTypeAsString(CompilerState * state);

    virtual const std::string getTmpStorageTypeAsString(CompilerState * state);

    virtual const char * getUlamTypeAsSingleLowercaseLetter();

    virtual void genUlamTypeMangledDefinitionForC(File * fp, CompilerState * state);

    virtual void genUlamTypeMangledImmediateDefinitionForC(File * fp, CompilerState * state);

    virtual void genUlamTypeReadDefinitionForC(File * fp, CompilerState * state);

    virtual void genUlamTypeWriteDefinitionForC(File * fp, CompilerState * state);


    static const char * getUlamTypeEnumAsString(ULAMTYPE etype);

    static ULAMTYPE getEnumFromUlamTypeString(const char * typestr);

    //for name by index see CompilerState::getUlamTypeNameByIndex
    virtual bool isConstant();

    bool isScalar();   //arraysize == NOTARRAYSIZE is scalar

    s32 getArraySize();

    virtual s32 getBitSize();  //'class' type calculated after type labeling; default size for constants

    u32 getTotalBitSize();  //bitsize * arraysize, accounting for constants and scalars

    /** Number of bits (rounded up to nearest 32 bits) required to
    hold the total bit size  */
    u32 getTotalWordSize();

    const std::string readMethodForCodeGen();

    const std::string writeMethodForCodeGen();

    virtual const std::string castMethodForCodeGen(UTI nodetype, CompilerState& state);

    virtual void genCodeAfterReadingIntoATmpVar(File * fp, UlamValue & uvpass, CompilerState& state);


  protected:
    UlamKeyTypeSignature m_key;
    UTI m_uti;
    u32 m_wordLength;

  private:
  };

}

#endif //end ULAMTYPE_H
