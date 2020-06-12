/**                                      -*- mode:C++ -*- */

#include "Uq_10106UrSelf10.h"
#include "Uq_10109210ByteStream10.h"
#include "Logger.h"

namespace MFM{

  template<class EC>
  void Uq_10156Logger10<EC>::Uf_3log(const UlamContext<EC>& uc, UlamRef<EC>& ur, Ui_Uq_r10109210ByteStream10<EC>& Ur_2bs) const
  {
    UlamRef<EC> urbs(Ur_2bs, 0, 0u); //gcnl:NodeFunctionCall.cpp:965

    VfuncPtr readByteFunc;
    UlamRef<EC> vfur(urbs, Uq_10109216ByteStreamReader10<EC>::VOWNED_IDX_Uf_8readByte10, Uq_10109216ByteStreamReader10<EC>::THE_INSTANCE, readByteFunc);

    _UlamByteSourceWrapper<EC> ubsw(uc, vfur, (typename Uq_10109216ByteStreamReader10<EC>::Uf_8readByte10) (readByteFunc));

    Ui_Ut_10141u<EC> level = Uf_8getLevel(uc,ur);
    u32 clevel = level.read();

    const u32 mShowSite = UlamRef<EC>(ur, 4u, 1u, NULL, UlamRef<EC>::PRIMITIVE).Read(); //gcnl:Node.cpp:664
    if(_Bool32ToCbool(mShowSite, 1) && uc.HasEventWindow()) //gcnl:NodeControl.cpp:213
    {
      SPoint ctr = uc.GetEventWindow().GetCenterInTile();
      SPointSerializer sctr(ctr);
      LOG.Log((Logger::Level) clevel,"s%@ t%s: %< ",
              &sctr, uc.GetContextLabel(), &ubsw);
    }
    else
    {
      LOG.Log((Logger::Level) clevel,"%<", &ubsw);
    }
  }

} //MFM
