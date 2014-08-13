/* -*- c++ -*- */

#ifndef ULAMTYPEINT_H
#define ULAMTYPEINT_H

#include "UlamType.h"

namespace MFM{

  class CompilerState; //forward

  class UlamTypeInt : public UlamType
  {
  public:

    UlamTypeInt(const UlamKeyTypeSignature key, const UTI uti);
    virtual ~UlamTypeInt(){}

    virtual void newValue(UlamValue & val);

    virtual void deleteValue(UlamValue * val);

    virtual ULAMTYPE getUlamTypeEnum();

    virtual bool cast(UlamValue & val);

    virtual void getUlamValueAsString(const UlamValue & val, char * valstr, CompilerState& state);

    virtual bool isZero(const UlamValue & val);
    
  private:

  };

}

#endif //end ULAMTYPEINT_H
