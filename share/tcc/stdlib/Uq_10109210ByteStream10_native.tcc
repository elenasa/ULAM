/**                                      -*- mode:C++ -*- */

#include "UlamDefs.h"
#include "Uq_10106UrSelf10.h"
#include "UlamByteWrappers.h"
#include <stdarg.h>  /* For ... */

#ifndef Ud_Ui_Ut_r10131i
#define Ud_Ui_Ut_r10131i
namespace MFM{
  template<class EC>
  struct Ui_Ut_r10131i : public UlamRef<EC>
  {
    typedef typename EC::ATOM_CONFIG AC;
    typedef typename AC::ATOM_TYPE T;
    enum { BPA = AC::BITS_PER_ATOM };

    const u32 read() const { return UlamRef<EC>::Read(); /* entire */ } //gcnl:UlamTypePrimitive.cpp:323
    void write(const u32& targ) { UlamRef<EC>::Write(targ); /* entire */ } //gcnl:UlamTypePrimitive.cpp:355
    Ui_Ut_r10131i(BitStorage<EC>& targ, u32 idx, const UlamContext<EC>& uc) : UlamRef<EC>(idx, 3u, targ, NULL, UlamRef<EC>::PRIMITIVE, uc) { } //gcnl:UlamTypePrimitive.cpp:241
    Ui_Ut_r10131i(const UlamRef<EC>& arg, s32 idx) : UlamRef<EC>(arg, idx, 3u, NULL, UlamRef<EC>::PRIMITIVE) { } //gcnl:UlamTypePrimitive.cpp:253
    Ui_Ut_r10131i(const Ui_Ut_r10131i<EC>& arg) : UlamRef<EC>(arg, 0, arg.GetLen(), NULL, UlamRef<EC>::PRIMITIVE) { MFM_API_ASSERT_ARG(arg.GetLen() == 3); } //gcnl:UlamTypePrimitive.cpp:268
    Ui_Ut_r10131i& operator=(const Ui_Ut_r10131i& rhs); //declare away //gcnl:UlamTypePrimitive.cpp:277
  };
} //MFM
#endif /*Ud_Ui_Ut_r10131i */

#ifndef Ud_Ui_Ut_10131i
#define Ud_Ui_Ut_10131i
namespace MFM{

  template<class EC>
  struct Ui_Ut_10131i : public BitVectorBitStorage<EC, BitVector<3u> >
  {
    typedef typename EC::ATOM_CONFIG AC;
    typedef typename AC::ATOM_TYPE T;
    enum { BPA = AC::BITS_PER_ATOM };

    typedef BitVector<3> BV; //gcnl:UlamTypePrimitive.cpp:476
    typedef BitVectorBitStorage<EC, BV> BVS; //gcnl:UlamTypePrimitive.cpp:479

    const u32 read() const { return BVS::Read(0u, 3u); } //gcnl:UlamTypePrimitive.cpp:565
    void write(const u32& v) { BVS::Write(0u, 3u, v); } //gcnl:UlamTypePrimitive.cpp:602
    Ui_Ut_10131i() { } //gcnl:UlamTypePrimitive.cpp:492
    Ui_Ut_10131i(const u32 d) { write(d); } //gcnl:UlamTypePrimitive.cpp:500
    Ui_Ut_10131i(const Ui_Ut_10131i& other) { this->write(other.read()); } //gcnl:UlamTypePrimitive.cpp:523
    Ui_Ut_10131i(const Ui_Ut_r10131i<EC>& d) { this->write(d.read()); } //gcnl:UlamTypePrimitive.cpp:532
  };
} //MFM
#endif /*Ud_Ui_Ut_10131i */

namespace MFM{

  template<class EC>
  Ui_Uq_r10109210ByteStream10<EC> Uq_10109210ByteStream10<EC>::Uf_6printf(const UlamContext<EC>& uc, UlamRef<EC>& ur, Ui_Ut_102321s<EC>& Uv_3fmt, ...) const
  {
    const u32 writeByteFuncIdx = Uq_10109210ByteStream10<EC>::VOWNED_IDX_Uf_919writeByte1110181u;
    const u32 baseclassoffset = ur.GetEffectiveSelf()->GetVTStartOffsetForClassByRegNum(Uq_10109210ByteStream10<EC>::THE_INSTANCE.GetRegistrationNumber());

    VfuncPtr writeByte = ur.GetEffectiveSelf()->getVTableEntry(writeByteFuncIdx + baseclassoffset);
    Uq_10109210ByteStream10<EC>::Uf_919writeByte1110181u writeByteFunc
      = (Uq_10109210ByteStream10<EC>::Uf_919writeByte1110181u) writeByte;

    _UlamByteSinkWrapper<EC> ubsw(uc,ur, writeByteFunc);
    Ui_Uq_r10109210ByteStream10<EC> self(ur, 0u, ur.GetEffectiveSelf());

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
    return self;
  } // Uf_6printf

} //MFM
