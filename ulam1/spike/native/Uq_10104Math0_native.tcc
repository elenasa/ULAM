/**                                      -*- mode:C++ -*- */

namespace MFM{

  template<class CC, u32 POS>
  Ui_Ut_102323Int Uq_10104Math0<CC, POS>::Uf_3max(UlamContext<CC>& uc, T& Uv_4self, ...) //native
  {
    va_list ap;
    va_start(ap, Uv_4self);
    s32 max = S32_MIN;
    while(true)
      {
	Ui_Ut_102323Int * aptr = va_arg(ap, Ui_Ut_102323Int*);
	if(!aptr) break;
	s32 a = aptr->read();
	if(a > max) max = a;
      }
    va_end(ap);
    return Ui_Ut_102323Int(max);
  }

} //MFM
