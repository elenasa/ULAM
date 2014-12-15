/**                                      -*- mode:C++ -*- */

//#include "itype.h"
//#include "BitVector.h"
//#include "BitField.h"

#include "UlamDefs.h"

#ifndef Ud_Ui_Ut_102323Int
#define Ud_Ui_Ut_102323Int
namespace MFM{
  struct Ui_Ut_102323Int
  {
    typedef BitField<BitVector<32>, VD::S32, 32, 0> BF;
    BitVector<32> m_stg;
    Ui_Ut_102323Int() : m_stg() { }
    Ui_Ut_102323Int(const s32 d) : m_stg(d) {}
    Ui_Ut_102323Int(const Ui_Ut_102323Int& d) : m_stg(d.m_stg) {}
    ~Ui_Ut_102323Int() {}
    const s32 read() const { return BF::Read(m_stg); }
    void write(const s32 v) { BF::Write(m_stg, v); }
  };
} //MFM
#endif /*Ud_Ui_Ut_102323Int */

#ifndef Ud_Ui_Ut_102328Unsigned
#define Ud_Ui_Ut_102328Unsigned
namespace MFM{
  struct Ui_Ut_102328Unsigned
  {
    typedef BitField<BitVector<32>, VD::U32, 32, 0> BF;
    BitVector<32> m_stg;
    Ui_Ut_102328Unsigned() : m_stg() { }
    Ui_Ut_102328Unsigned(const u32 d) : m_stg(d) {}
    Ui_Ut_102328Unsigned(const Ui_Ut_102328Unsigned& d) : m_stg(d.m_stg) {}
    ~Ui_Ut_102328Unsigned() {}
    const u32 read() const { return BF::Read(m_stg); }
    void write(const u32 v) { BF::Write(m_stg, v); }
  };
} //MFM
#endif /*Ud_Ui_Ut_102328Unsigned */

#ifndef Ud_Ui_Ut_10114Bool
#define Ud_Ui_Ut_10114Bool
namespace MFM{
  struct Ui_Ut_10114Bool
  {
    typedef BitField<BitVector<32>, VD::BOOL, 1, 31> BF;
    BitVector<32> m_stg;
    Ui_Ut_10114Bool() : m_stg() { }
    Ui_Ut_10114Bool(const u32 d) : m_stg(d) {}
    Ui_Ut_10114Bool(const Ui_Ut_10114Bool& d) : m_stg(d.m_stg) {}
    ~Ui_Ut_10114Bool() {}
    const u32 read() const { return BF::Read(m_stg); }
    void write(const u32 v) { BF::Write(m_stg, v); }
  };
} //MFM
#endif /*Ud_Ui_Ut_10114Bool */

#ifndef Ud_Ui_Ut_102325Unary
#define Ud_Ui_Ut_102325Unary
namespace MFM{
  struct Ui_Ut_102325Unary
  {
    typedef BitField<BitVector<32>, VD::UNARY, 32, 0> BF;
    BitVector<32> m_stg;
    Ui_Ut_102325Unary() : m_stg() { }
    Ui_Ut_102325Unary(const u32 d) : m_stg(d) {}
    Ui_Ut_102325Unary(const Ui_Ut_102325Unary& d) : m_stg(d.m_stg) {}
    ~Ui_Ut_102325Unary() {}
    const u32 read() const { return BF::Read(m_stg); }
    void write(const u32 v) { BF::Write(m_stg, v); }
  };
} //MFM
#endif /*Ud_Ui_Ut_102325Unary */

#ifndef Ud_Ui_Ut_102324Bits
#define Ud_Ui_Ut_102324Bits
namespace MFM{
  struct Ui_Ut_102324Bits
  {
    typedef BitField<BitVector<32>, VD::BITS, 32, 0> BF;
    BitVector<32> m_stg;
    Ui_Ut_102324Bits() : m_stg() { }
    Ui_Ut_102324Bits(const u32 d) : m_stg(d) {}
    Ui_Ut_102324Bits(const Ui_Ut_102324Bits& d) : m_stg(d.m_stg) {}
    ~Ui_Ut_102324Bits() {}
    const u32 read() const { return BF::Read(m_stg); }
    void write(const u32 v) { BF::Write(m_stg, v); }
  };
} //MFM
#endif /*Ud_Ui_Ut_102324Bits */

#ifndef Ud_Ui_Ut_102964Atom
#define Ud_Ui_Ut_102964Atom
namespace MFM{

  template<class CC>
  struct Ui_Ut_102964Atom
  {
    typedef typename CC::ATOM_TYPE T;
    typedef typename CC::PARAM_CONFIG P;
    enum { BPA = P::BITS_PER_ATOM };
    T m_stg;  //storage here!
    Ui_Ut_102964Atom() : m_stg() { }
    Ui_Ut_102964Atom(const T d) : m_stg(d) {}
    Ui_Ut_102964Atom(const Ui_Ut_102964Atom<CC>& d) : m_stg(d.m_stg) {}
    ~Ui_Ut_102964Atom() {}
    const T read() const { return m_stg; }
    void write(const T v) { m_stg = v; }
    T& getRef() { return m_stg; }
  };
} //MFM
#endif /*Ud_Ui_Ut_102964Atom */

#ifndef Ud_Ui_Ut_10143Int
#define Ud_Ui_Ut_10143Int
namespace MFM{
  struct Ui_Ut_10143Int
  {
    typedef BitField<BitVector<32>, VD::S32, 4, 28> BF;
    BitVector<32> m_stg;
    Ui_Ut_10143Int() : m_stg() { }
    Ui_Ut_10143Int(const s32 d) : m_stg(d) {}
    Ui_Ut_10143Int(const Ui_Ut_10143Int& d) : m_stg(d.m_stg) {}
    ~Ui_Ut_10143Int() {}
    const s32 read() const { return BF::Read(m_stg); }
    void write(const s32 v) { BF::Write(m_stg, v); }
  };
} //MFM
#endif /*Ud_Ui_Ut_10143Int */

#ifndef Ud_Ui_Ut_10133Int
#define Ud_Ui_Ut_10133Int
namespace MFM{
  struct Ui_Ut_10133Int
  {
    typedef BitField<BitVector<32>, VD::S32, 3, 29> BF;
    BitVector<32> m_stg;
    Ui_Ut_10133Int() : m_stg() { }
    Ui_Ut_10133Int(const s32 d) : m_stg(d) {}
    Ui_Ut_10133Int(const Ui_Ut_10133Int& d) : m_stg(d.m_stg) {}
    ~Ui_Ut_10133Int() {}
    const s32 read() const { return BF::Read(m_stg); }
    void write(const s32 v) { BF::Write(m_stg, v); }
  };
} //MFM
#endif /*Ud_Ui_Ut_10133Int */

#ifndef Ud_Ui_Ut_10135Unary
#define Ud_Ui_Ut_10135Unary
namespace MFM{
  struct Ui_Ut_10135Unary
  {
    typedef BitField<BitVector<32>, VD::UNARY, 3, 29> BF;
    BitVector<32> m_stg;
    Ui_Ut_10135Unary() : m_stg() { }
    Ui_Ut_10135Unary(const u32 d) : m_stg(d) {}
    Ui_Ut_10135Unary(const Ui_Ut_10135Unary& d) : m_stg(d.m_stg) {}
    ~Ui_Ut_10135Unary() {}
    const u32 read() const { return BF::Read(m_stg); }
    void write(const u32 v) { BF::Write(m_stg, v); }
  };
} //MFM
#endif /*Ud_Ui_Ut_10135Unary */

#ifndef Ud_Ui_Ut_10134Bool
#define Ud_Ui_Ut_10134Bool
namespace MFM{
  struct Ui_Ut_10134Bool
  {
    typedef BitField<BitVector<32>, VD::BOOL, 3, 29> BF;
    BitVector<32> m_stg;
    Ui_Ut_10134Bool() : m_stg() { }
    Ui_Ut_10134Bool(const u32 d) : m_stg(d) {}
    Ui_Ut_10134Bool(const Ui_Ut_10134Bool& d) : m_stg(d.m_stg) {}
    ~Ui_Ut_10134Bool() {}
    const u32 read() const { return BF::Read(m_stg); }
    void write(const u32 v) { BF::Write(m_stg, v); }
  };
} //MFM
#endif /*Ud_Ui_Ut_10134Bool */

#ifndef Ud_Ui_Uq_10106System
#define Ud_Ui_Uq_10106System
namespace MFM{

  template<class CC, u32 POS> class Uq_10106System;  //forward

  template<class CC>
  struct Ui_Uq_10106System
  {
    typedef typename CC::ATOM_TYPE T;
    typedef typename CC::PARAM_CONFIG P;
    enum { BPA = P::BITS_PER_ATOM };

    typedef Uq_10106System<CC, 96> Us;
    typedef AtomicParameterType<CC, VD::BITS, 0, 96>  Up_Us;
    T m_stg;  //storage here!
    Ui_Uq_10106System() : m_stg() { }
    Ui_Uq_10106System(const u32 d) { write(d); }
    Ui_Uq_10106System(const Ui_Uq_10106System<CC>& d) : m_stg(d.m_stg) {}
    ~Ui_Uq_10106System() {}
    const u32 read() const { return Up_Us::Read(m_stg.GetBits() ); }
    void write(const u32 v) { Up_Us::Write(m_stg.GetBits(), v); }
    BitVector<BPA>& getBits() { return m_stg.GetBits(); }
    T& getRef() { return m_stg; }
  };
} //MFM
#endif /*Ud_Ui_Uq_10106System */

#ifndef Ud_Ui_Uq_10106System4auto
#define Ud_Ui_Uq_10106System4auto
namespace MFM{

  template<class CC>
  struct Ui_Uq_10106System4auto : public Ui_Uq_10106System<CC>
  {
    typedef typename CC::ATOM_TYPE T;
    typedef typename CC::PARAM_CONFIG P;
    enum { BPA = P::BITS_PER_ATOM };

    T& m_stgToChange;  //ref to storage here!
    const u32 m_pos;   //pos in atom
    Ui_Uq_10106System4auto(T & realStg, u32 idx) : m_stgToChange(realStg), m_pos(idx) {}
    ~Ui_Uq_10106System4auto() { m_stgToChange.GetBits().Write(m_pos + T::ATOM_FIRST_STATE_BIT, 0, Ui_Uq_10106System<CC>::read()); }
  };
} //MFM
#endif /*Ud_Ui_Uq_10106System4auto */

