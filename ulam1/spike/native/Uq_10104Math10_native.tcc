/**                                      -*- mode:C++ -*- */

namespace MFM{

  template<class CC>
  Ui_Ut_102321i<CC> Uq_10104Math10<CC>::Uf_3max(const UlamContext<CC>& uc, const UlamRef<CC>& ur, ...) const //native
  {
    va_list ap;
    va_start(ap, ur);
    s32 max = S32_MIN;
    while(true)
      {
	Ui_Ut_102321i<CC> * aptr = va_arg(ap, Ui_Ut_102321i<CC>*);
	if(!aptr) break;
	s32 a = aptr->Read();
	if(a > max) max = a;
      }
    va_end(ap);
    return Ui_Ut_102321i<CC>(max);
  }

} //MFM
