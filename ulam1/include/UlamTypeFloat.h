/* -*- c++ -*- */

#ifndef ULAMTYPEFLOAT_H
#define ULAMTYPEFLOAT_H

#include "UlamType.h"

namespace MFM{

  class CompilerState; //forward

  class UlamTypeFloat : public UlamType
  {
  public:

    UlamTypeFloat(const UlamKeyTypeSignature key, const UTI uti);
    virtual ~UlamTypeFloat(){}

    virtual void newValue(UlamValue & val);

    virtual void deleteValue(UlamValue * val);

    virtual ULAMTYPE getUlamTypeEnum();

    virtual bool cast(UlamValue& val);

    virtual void getUlamValueAsString(const UlamValue & val, char * valstr, CompilerState& state);

    virtual bool isZero(const UlamValue & val);
 

  private:

  };

}

#endif //end ULAMTYPEFLOAT_H
