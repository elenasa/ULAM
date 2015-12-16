/**                                      -*- mode:C++ -*- */

namespace MFM{

  template<class CC, u32 POS>
  Ui_Ut_102321i<CC> Uq_10104Math10<CC, POS>::Uf_3max(const UlamContext<CC>& uc, T& Uv_4self, ...) const //native
  {
    va_list ap;
    va_start(ap, Uv_4self);
    s32 max = S32_MIN;
    while(true)
      {
	Ui_Ut_102321i<CC> * aptr = va_arg(ap, Ui_Ut_102321i<CC>*);
	if(!aptr) break;
	s32 a = aptr->read();
	if(a > max) max = a;
      }
    va_end(ap);
    return Ui_Ut_102321i<CC>(max);
  }

} //MFM
