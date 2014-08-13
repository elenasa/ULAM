/* -*- c++ -*- */

#ifndef ULAMTYPEATOM_H
#define ULAMTYPEATOM_H

#include "UlamType.h"

namespace MFM{

  class CompilerState; //forward

  class UlamTypeAtom : public UlamType
  {
  public:
    
    UlamTypeAtom(const UlamKeyTypeSignature key, const UTI uti);
    virtual ~UlamTypeAtom(){}

    virtual void newValue(UlamValue & val);

    virtual void deleteValue(UlamValue * val);

    virtual ULAMTYPE getUlamTypeEnum();
    
    virtual bool cast(UlamValue& val);
    
    virtual void getUlamValueAsString(const UlamValue & val, char * valstr, CompilerState& state);
    
    virtual bool isZero(const UlamValue & val);
        
  private:
    
  };
  
}

#endif //end ULAMTYPEATOM_H
