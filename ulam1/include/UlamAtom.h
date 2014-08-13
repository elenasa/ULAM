/* -*- c++ -*- */

#ifndef ULAMATOM_H
#define ULAMATOM_H

#include <vector>
#include "itype.h"
#include "UlamValue.h"
#include "UlamType.h"

namespace MFM
{
 
  class CompilerState;  //forward

 //similar to CallStack, except for dataMembers
  class UlamAtom
  {
  public:
    UlamAtom();
    ~UlamAtom();

    void init(UlamType * intType);

    u32 getUlamAtomTypeNumber();
    void setUlamAtomTypeNumber(u32 typenumber);
    
    //returns baseIndex of array, or index of scalar
    u32 pushDataMember(UlamType * arrayType, UlamType * scalarType);

    UlamValue getDataMemberAt(u32 idx);
    void storeDataMemberAt(UlamValue uv, u32 idx);

    void assignUlamValue(UlamValue luv, UlamValue ruv, CompilerState & state);
    void assignUlamValuePtr(UlamValue pluv, UlamValue puv, CompilerState & state);

  private:
    u32 m_elementTypeNumber;
    std::vector<UlamValue> m_dataMembers;

  };

}

#endif //end ULAMATOM_H
