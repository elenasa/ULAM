/* -*- c++ -*- */

#ifndef ULAMTYPEELEMENT_H
#define ULAMTYPEELEMENT_H

#include "UlamType.h"

namespace MFM{

  class CompilerState; //forward

  class UlamTypeElement : public UlamType
  {
  public:
    
    UlamTypeElement(const UlamKeyTypeSignature key, const UTI uti);
    virtual ~UlamTypeElement(){}
    virtual void newValue(UlamValue & val);
    
    virtual void deleteValue(UlamValue * val);

    virtual ULAMTYPE getUlamTypeEnum();

    virtual bool cast(UlamValue &val);
    
    virtual void getUlamValueAsString(const UlamValue & val, char * valstr, CompilerState& state);
    
    virtual bool isZero(const UlamValue & val);

    
    
  private:
    
  };
  
}

#endif //end ULAMTYPEELEMENT_H
