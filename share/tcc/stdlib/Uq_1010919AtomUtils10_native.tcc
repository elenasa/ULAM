/* -*- C++ -*- */

#include "UlamDefs.h"

namespace MFM {

  //! AtomUtils.ulam:21:   Bool hasAtomof(UrSelf& ref) native;
  template<class EC>
  Ui_Ut_10111b<EC> Uq_1010919AtomUtils10<EC>::Uf_919hasAtomof(const UlamContext<EC>& uc, UlamRef<EC>& ur, Ui_Uq_r10106UrSelf10<EC>& Ur_3ref) const
  {
    const u32 hasRealType = (Ur_3ref.GetType() != T::ATOM_UNDEFINED_TYPE);
    Ui_Ut_10111b<EC> ubool(hasRealType);
    return ubool;
  } // Uf_919hasAtomof


  //! AtomUtils.ulam:10:   Type getType(Atom a) {
  template<class EC>
  Ui_Ut_102321i<EC> Uq_1010919AtomUtils10<EC>::Uf_7getType(const UlamContext<EC> & uc, UlamRef<EC>& ur, Ui_Ut_102961a<EC>& Uv_1a) const
  {
    return Ui_Ut_102321i<EC>(Uv_1a.GetType());
  } // Uf_7getType


  //! AtomUtils.ulam:13:   Atom new(Type t) {
  template<class EC>
  Ui_Ut_102961a<EC> Uq_1010919AtomUtils10<EC>::Uf_3new(const UlamContext<EC> & uc, UlamRef<EC>& ur, Ui_Ut_102321i<EC>& Uv_1t) const
  {
    const s32 t = Uv_1t.read();
    if (t < 0 || t > U16_MAX)
      FAIL(ILLEGAL_ARGUMENT);
    T p3atom((u32) t);
    const Ui_Ut_102961a<EC> tmp(p3atom);
    return tmp;
  } // Uf_3new

//! AtomUtils.ulam:56:   Bits read(Atom a, Unsigned stateIndex, Unsigned fieldLength) { Bits b; return b; }//native;
  template<class EC>
  Ui_Ut_102321t<EC> Uq_1010919AtomUtils10<EC>::Uf_4read(const UlamContext<EC>& uc, UlamRef<EC>& ur, 
                                                        Ui_Ut_102961a<EC>& Uv_1a, 
                                                        Ui_Ut_102321u<EC>& Uv_9210stateIndex, 
                                                        Ui_Ut_102321u<EC>& Uv_9211fieldLength) const
  {
    T atom = Uv_1a.read();
    u32 bits = atom.GetBits().Read(Uv_9210stateIndex.read(), Uv_9211fieldLength.read());
    Ui_Ut_102321t<EC> ret(bits);
    return ret;
  } // Uf_4read


  template<class EC>
  Ui_Ut_102961a<EC> Uq_1010919AtomUtils10<EC>::Uf_919writeCopy(const UlamContext<EC>& uc, UlamRef<EC>& ur, 
                                                               Ui_Ut_102961a<EC>& Uv_1a, 
                                                               Ui_Ut_102321t<EC>& Uv_4bits, 
                                                               Ui_Ut_102321u<EC>& Uv_9210stateIndex, 
                                                               Ui_Ut_102321u<EC>& Uv_9211fieldLength) const
  {
    T atom = Uv_1a.read(); 
    atom.GetBits().Write(Uv_9210stateIndex.read(), Uv_9211fieldLength.read(), Uv_4bits.read());
    Ui_Ut_102961a<EC> ret(atom);
    return ret;
  } // Uf_919writeCopy

} //MFM
