/**                                      -*- mode:C++ -*- */

#include "UlamDefs.h"
#include "DebugTools.h"         /* For DebugPrint */
/*#include "Uq_10106UrSelf10.h"*/
#include "UlamByteWrappers.h"
#include <stdarg.h>  /* For ... */

#ifndef Ud_Ui_Ut_r102961a
#define Ud_Ui_Ut_r102961a
namespace MFM{
  template<class EC>
  struct Ui_Ut_r102961a : public UlamRef<EC>
  {
    typedef typename EC::ATOM_CONFIG AC;
    typedef typename AC::ATOM_TYPE T;
    enum { BPA = AC::BITS_PER_ATOM };

    T read() const { return UlamRef<EC>::ReadAtom(); /* read entire atom */ } //gcnl:UlamTypeAtom.cpp:259
    void  read(T& rtnbv) const { rtnbv = UlamRef<EC>::ReadAtom(); /* read entire atom ref */ } //gcnl:UlamTypeAtom.cpp:272
    void write(const T& v) { UlamRef<EC>::WriteAtom(v); /* write entire atom */ } //gcnl:UlamTypeAtom.cpp:290
    void write(const AtomRefBitStorage<EC>& v) { UlamRef<EC>::WriteAtom(v.ReadAtom()); /* write entire atom */ } //gcnl:UlamTypeAtom.cpp:298
    Ui_Ut_r102961a(BitStorage<EC>& targ, u32 startidx, const UlamContext<EC>& uc) : UlamRef<EC>(startidx, BPA, targ, uc.LookupUlamElementTypeFromContext(targ.ReadAtom(startidx).GetType()), UlamRef<EC>::ATOMIC, uc) { } //gcnl:UlamTypeAtom.cpp:209
    Ui_Ut_r102961a(const UlamRef<EC>& arg, s32 incr) : UlamRef<EC>(arg, incr, BPA, NULL, UlamRef<EC>::ATOMIC) { } //gcnl:UlamTypeAtom.cpp:217
    Ui_Ut_r102961a(const Ui_Ut_r102961a& arg) : UlamRef<EC>(arg, 0, arg.GetLen(), arg.GetEffectiveSelf(), UlamRef<EC>::ATOMIC) { } //gcnl:UlamTypeAtom.cpp:225
    Ui_Ut_r102961a& operator=(const Ui_Ut_r102961a& rhs); //declare away //gcnl:UlamTypeAtom.cpp:234
  };
} //MFM
#endif /*Ud_Ui_Ut_r102961a */

namespace MFM{

//! ByteStream.ulam:69:   Self & print(UrSelf & urs)
  template<class EC>
  Ui_Uq_r10109216ByteStreamWriter10<EC> Uq_10109216ByteStreamWriter10<EC>::Uf_5print(const UlamContext<EC>& uc, UlamRef<EC>& ur, Ui_Uq_r10106UrSelf10<EC>& Ur_3urs) const
  {
    Ui_Uq_r10109216ByteStreamWriter10<EC> ret(ur);

    OString4096 buff;
    const bool isElementType = (Ur_3urs.GetType() != T::ATOM_UNDEFINED_TYPE); 
    if(isElementType) {
      Ui_Ut_r102961a<EC> atomref(Ur_3urs, -Ur_3urs.GetPosToEffectiveSelf() - T::ATOM_FIRST_STATE_BIT); 
      DebugPrint<EC>(uc, atomref, buff);
    } else {
      DebugPrint<EC>(uc, Ur_3urs, buff); // Else better be primitive I guess
    }

    {
      VfuncPtr writeByte;
      UlamRef<EC> vfur(ur, Uq_10109216ByteStreamWriter10<EC>::VOWNED_IDX_Uf_919writeByte1110181u, Uq_10109216ByteStreamWriter10<EC>::THE_INSTANCE, writeByte);

      Uq_10109216ByteStreamWriter10<EC>::Uf_919writeByte1110181u writeByteFunc
        = (Uq_10109216ByteStreamWriter10<EC>::Uf_919writeByte1110181u) writeByte;

      _UlamByteSinkWrapper<EC> ubsw(uc, vfur, writeByteFunc);

      for (const char * p = buff.GetZString(); *p != 0; ++p) {
          ubsw.WriteByte(*p);
      }

    }

    return ret;
  } // Uf_5print

  template<class EC>
  Ui_Uq_r10109216ByteStreamWriter10<EC> Uq_10109216ByteStreamWriter10<EC>::Uf_6printf(const UlamContext<EC>& uc, UlamRef<EC>& ur, Ui_Ut_102201s<EC>& Uv_3fmt, ...) const
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

#if 0 /*NOT READY FOR PRIME TIME (NEED VARARG REFS)*/
        case 'v': {
          const UlamTypeInfoClass * asclass = uti.AsClass();
          if (!asclass) {
            if (asprim) { // Let %v == decimal for primitives
              fmt = Format::DEC;
              goto printinbase;
            }
            continue;
          }

          if (!uc.HasUlamClassRegistry()) {
            ubsw.Printf("<<NO UCR (%s)>>", mangled);
            continue;
          }
          const UlamClassRegistry<EC>& ucr = uc.GetUlamClassRegistry();

          const UlamClass<EC> * argclass = ucr.GetUlamClassByMangledName(mangled); 
          if (!argclass) {
            ubsw.Printf("<<NO ARGCLASS (%s)>>", mangled);
            continue;
          }

          const bool iselt = uti.IsElement();
          const typename UlamRef<EC>::UsageType argtype =
            iselt ? UlamRef<EC>::ELEMENTAL : UlamRef<EC>::CLASSIC;
          const u32 pos = iselt ? 25u : 0u; // ???
          const u32 len = argclass->GetClassLength();

          UlamRef<EC> effref(pos, len, *arg, argclass, argtype, uc);
          OString4096 buff;
          DebugPrint<EC>(uc, effref, buff);

          ubsw.Print(buff.GetBuffer(), buff.GetLength());
          continue;
        }
#endif

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

            const u32 strIdx = ((Ui_Ut_102201s<EC> *) arg)->read();
            const u8 * str = GetStringPointerFromGlobalStringPool(strIdx);
            if (!str) FAIL(ILLEGAL_STATE);
            u32 strlen = GetStringLengthFromGlobalStringPool(strIdx);

            ubsw.Print(str, strlen);
          }
          continue;

        default:
          ubsw.Printf("<unhandled '%%%c'>",ch);
        }
      }
    }

    va_end(ap);
    return Ui_Uq_r10109216ByteStreamWriter10<EC>(ur);
  } // Uf_6printf

} //MFM
