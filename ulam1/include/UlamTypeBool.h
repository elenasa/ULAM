/* -*- c++ -*- */

#ifndef ULAMTYPEBOOL_H
#define ULAMTYPEBOOL_H

#include "UlamType.h"

namespace MFM{

  class CompilerState; //forward

  class UlamTypeBool : public UlamType
  {
  public:

    UlamTypeBool(const UlamKeyTypeSignature key, const UTI uti);
    virtual ~UlamTypeBool(){}

    virtual void newValue(UlamValue & val);

    virtual void deleteValue(UlamValue * val);

    virtual ULAMTYPE getUlamTypeEnum();

    virtual bool cast(UlamValue& val);

    virtual void getUlamValueAsString(const UlamValue & val, char * valstr, CompilerState& state);

    virtual bool isZero(const UlamValue & val);

  private:

  };

}

#endif //end ULAMTYPEBOOL_H
