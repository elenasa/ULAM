/* -*- c++ -*- */

#ifndef ULAMTYPEQUARK_H
#define ULAMTYPEQUARK_H

#include "UlamType.h"

namespace MFM{

  class CompilerState; //forward

  class UlamTypeQuark : public UlamType
  {
  public:
    
    UlamTypeQuark(const UlamKeyTypeSignature key, const UTI uti);
    virtual ~UlamTypeQuark(){}

    virtual void newValue(UlamValue & val);
    
    virtual void deleteValue(UlamValue * val);

    virtual ULAMTYPE getUlamTypeEnum();
        
    virtual bool cast(UlamValue & val);
    
    virtual void getUlamValueAsString(const UlamValue & val, char * valstr, CompilerState& state);
    
    virtual bool isZero(const UlamValue & val);

   private:
    
  };
  
}

#endif //end ULAMTYPEQUARK_H
