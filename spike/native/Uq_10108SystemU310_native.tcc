/**                                      -*- mode:C++ -*- */

#include <stdio.h>   //for printf
#include <stdlib.h>  //for abort

namespace MFM{


  template<class CC>
  void Uq_10108SystemU310<CC>::Uf_5print(const UlamContext<CC>& uc, UlamRef<CC>& ur, Ui_Ut_102321s<CC>& Uv_3arg) const //native
  {
    const u32 sidx = Uv_3arg.read();
    if(sidx == 0)
      FAIL(UNINITIALIZED_VALUE);
    if(sidx >= USERSTRINGPOOLSIZE)
      FAIL(ARRAY_INDEX_OUT_OF_BOUNDS);
    printf("String(%u) Arg: \"%s\"\n", sidx, Ug_9214UserStringPool + sidx + 1);
  }

}
