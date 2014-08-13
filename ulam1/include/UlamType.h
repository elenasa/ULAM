/* -*- c++ -*- */

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
