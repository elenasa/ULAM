/**                                      -*- mode:C++ -*- */

#include <stdio.h>   //for printf
#include <stdlib.h>  //for abort
#include "GlobalStringPool.h"
#include <inttypes.h>

namespace MFM{


  template<class CC>
  void Uq_10108SystemU510<CC>::Uf_5print(const UlamContext<CC>& uc, UlamRef<CC>& ur, Ui_Ut_102641i<CC>& Uv_3arg) const //native
  {
    s64 tmp = Uv_3arg.read();
    //std::string str = ToSignedDecimal(tmp); //UlamUtil not available to native
    printf("Int Long Arg: %" PRId64 "\n", tmp);
  }


  template<class CC>
  void Uq_10108SystemU510<CC>::Uf_5print(const UlamContext<CC>& uc, UlamRef<CC>& ur, Ui_Ut_102641u<CC>& Uv_3arg) const //native
  {
    u64 tmp = Uv_3arg.read();
    //std::string str = UlamUtil::ToUnsignedDecimal(tmp); //UlamUtil not available to native
    printf("Unsigned Long Arg: %" PRIu64 "\n", tmp);
  }


  template<class CC>
  void Uq_10108SystemU510<CC>::Uf_5print(const UlamContext<CC>& uc, UlamRef<CC>& ur, Ui_Ut_102181s<CC>& Uv_3arg) const //native
  {
    const u32 sidx = Uv_3arg.getStringIndex();
    printf("String(%u) Arg: \"%s\"\n", sidx, GetStringPointerFromGlobalStringPool(sidx));
  }

}
