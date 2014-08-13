/* -*- c++ -*- */

#ifndef ULAMTYPEVOID_H
#define ULAMTYPEVOID_H

#include "UlamType.h"

namespace MFM{

  class CompilerState; //forward

  class UlamTypeVoid : public UlamType
  {
  public:

    UlamTypeVoid(const UlamKeyTypeSignature key, const UTI uti);
    virtual ~UlamTypeVoid(){}

    virtual void newValue(UlamValue & val);

    virtual void deleteValue(UlamValue * val);

    virtual ULAMTYPE getUlamTypeEnum();

    virtual bool cast(UlamValue & val);

    virtual void getUlamValueAsString(const UlamValue & val, char * valstr, CompilerState& state);

    virtual bool isZero(const UlamValue & val);
    
  private:

  };

}

#endif //end ULAMTYPEVOID_H
