/**                                      -*- mode:C++ -*- */

#include "UlamDefs.h"
/*#include "Uq_10106UrSelf10.h"*/
#include "UlamByteWrappers.h"
#include <stdarg.h>  /* For ... */

namespace MFM{

  template<class EC>
  Ui_Uq_r10109216ByteStreamWriter10<EC> Uq_10109216ByteStreamWriter10<EC>::Uf_6printf(const UlamContext<EC>& uc, UlamRef<EC>& ur, Ui_Ut_102321s<EC>& Uv_3fmt, ...) const
  {
    VfuncPtr writeByte;
    UlamRef<EC> vfur(ur, Uq_10109216ByteStreamWriter10<EC>::VOWNED_IDX_Uf_919writeByte1110181u, Uq_10109216ByteStreamWriter10<EC>::THE_INSTANCE, writeByte);

    Uq_10109216ByteStreamWriter10<EC>::Uf_919writeByte1110181u writeByteFunc
      = (Uq_10109216ByteStreamWriter10<EC>::Uf_919writeByte1110181u) writeByte;

    _UlamByteSinkWrapper<EC> ubsw(uc, vfur, writeByteFunc);

    const u32 strval = Uv_3fmt.read();
    const u8 * p = GetStringPointerFromGlobalStringPool(strval);

    va_list ap;
    va_start(ap, Uv_3fmt);
    u32 ch;
    while ((ch = *p++)) {
      if (ch != '%') {
        ubsw.WriteByte((u8) ch);
      } else {
        if ((ch = *p++)==0)
          FAIL(ILLEGAL_ARGUMENT);

        // Non-arg cases first
        switch (ch) {
        case '%':
          ubsw.WriteByte('%');
          continue;
        }

        // Now go for an arg
        BitStorage<EC>* arg = va_arg(ap, BitStorage<EC>*);
        if (!arg)
          FAIL(OUT_OF_BOUNDS);

        const char * mangled = arg->GetUlamTypeMangledName();
        // ubsw.Printf("[%s]",mangled);
        UlamTypeInfo uti;
        if (!uti.InitFrom(mangled))
          FAIL(ILLEGAL_STATE);
        const UlamTypeInfoPrimitive * asprim = uti.AsPrimitive();

        Format::Type fmt;
        switch (ch) {
        case 'x': fmt = Format::HEX; goto printinbase;
        case 'b': fmt = Format::BIN; goto printinbase;
        case 'o': fmt = Format::OCT; goto printinbase;
        case 'd': fmt = Format::DEC; goto printinbase;

        printinbase:
          {
            if (!asprim) FAIL(ILLEGAL_ARGUMENT);
            u32 bits = asprim->GetBitSize();
            if (bits <= 32) {
              const u32 val = arg->Read(0,bits);
              if (asprim->GetPrimType() == UlamTypeInfoPrimitive::INT) {
                const s32 sval = _Int32ToCs32(val,bits);
                ubsw.Print(sval,fmt);
              } else ubsw.Print(val,fmt);
            } else {
              const u64 val = arg->ReadLong(0,bits);
              if (asprim->GetPrimType() == UlamTypeInfoPrimitive::INT) {
                const s64 sval = _Int64ToCs64(val,bits);
                ubsw.Print(sval,fmt);
              } else ubsw.Print(val,fmt);
            }
          }
          continue;

        case 'c':
          {
            if (!asprim) FAIL(ILLEGAL_ARGUMENT);
            const u32 val = arg->Read(0,asprim->GetBitSize());
            ubsw.Print(val,Format::BYTE);
          }
          continue;

        case 's':
          {
            if (!asprim) FAIL(ILLEGAL_ARGUMENT);

            if (asprim->GetPrimType() != UlamTypeInfoPrimitive::STRING) FAIL(ILLEGAL_ARGUMENT);

            const u32 strIdx = ((Ui_Ut_102321s<EC> *) arg)->read();
            const u8 * str = GetStringPointerFromGlobalStringPool(strIdx);
            if (!str) FAIL(ILLEGAL_STATE);
            u32 strlen = GetStringLengthFromGlobalStringPool(strIdx);

            ubsw.Print(str, strlen);
          }
          continue;

        default:
          FAIL(INCOMPLETE_CODE);
        }
      }
    }

    va_end(ap);
    return Ui_Uq_r10109216ByteStreamWriter10<EC>(ur);
  } // Uf_6printf

} //MFM
