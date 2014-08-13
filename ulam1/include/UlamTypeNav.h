/* -*- c++ -*- */

#ifndef ULAMTYPENAV_H
#define ULAMTYPENAV_H

#include "UlamType.h"

namespace MFM{

  class CompilerState; //forward

  class UlamTypeNav : public UlamType
  {
  public:
    
    UlamTypeNav(const UlamKeyTypeSignature key, const UTI uti);
    virtual ~UlamTypeNav(){}
    
    virtual void newValue(UlamValue & val);

    virtual void deleteValue(UlamValue * val);

    virtual ULAMTYPE getUlamTypeEnum();
        
    virtual bool cast(UlamValue& val);
    
    virtual void getUlamValueAsString(const UlamValue & val, char * valstr, CompilerState& state);
    
    virtual bool isZero(const UlamValue & val);
    

  private:
    
  };
  
}

#endif //end ULAMTYPENAV_H
