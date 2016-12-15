/**                                      -*- mode:C++ -*- */

#include <stdio.h>   //for printf
#include <stdlib.h>  //for abort
#include <assert.h>

namespace MFM{


  template<class CC>
  void Uq_10108SystemU310<CC>::Uf_5print(const UlamContext<CC>& uc, UlamRef<CC>& ur, Ui_Ut_102321s<CC>& Uv_3arg) const //native
  {
    const u32 sidx = Uv_3arg.read();
    assert((sidx > 0) && (sidx + 1 < USERSTRINGPOOLSIZE));
    printf("String(%u) Arg: \"%s\"\n", sidx, Ug_9214UserStringPool + sidx + 1);
  }

}