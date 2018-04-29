/**                                      -*- mode:C++ -*- */

#include <stdlib.h>  //for abort
#include "DebugTools.h"  /* for DebugPrint */

namespace MFM{

  template<class EC>
  Ui_Ut_10111b<EC> Uq_10109210DebugUtils10<EC>::Uf_9214hasEventWindow(const UlamContext<EC> & uc, UlamRef<EC>& ur) const //native
  {
    Ui_Ut_10111b<EC> ret(uc.HasEventWindow());
    return ret;
  }


  template<class EC>
  void Uq_10109210DebugUtils10<EC>::Uf_9212printContext(const UlamContext<EC>& uc,
                                                             UlamRef<EC>& ur,
                                                             Ui_Ut_102321t<EC>& Uv_5flags) const
  {
    const EventWindow<EC> & ew = uc.GetEventWindow();
    const Tile<EC> & tile = ew.GetTile();
    SPoint ctr = ew.GetCenterInTile();

    OString512 buff;
    u32 flags = Uv_5flags.read();
    T atdirect = ew.GetCenterAtomDirect();
    Uq_10109210DebugUtils10_printAtom(uc, atdirect, flags, buff);

    LOG.Message("@(%2d,%2d) of %s: %s",
                ctr.GetX(), ctr.GetY(),
                tile.GetLabel(),
                buff.GetZString());
  } // Uf_9212printContext

  template<class EC>
  void Uq_10109210DebugUtils10<EC>::Uf_5print(const UlamContext<EC> & uc, UlamRef<EC>& ur, Ui_Uq_102323C2D10<EC>& Uv_5coord) const //native
  {
    enum { R = EC::EVENT_WINDOW_RADIUS };
    const EventWindow<EC> & ew = uc.GetEventWindow();

    const s32 x = _SignExtend32(UlamRef<EC>(0u, 16u, Uv_5coord, NULL, UlamRef<EC>::PRIMITIVE, uc).Read(), 16);
    const s32 y = _SignExtend32(UlamRef<EC>(16u, 16u, Uv_5coord, NULL, UlamRef<EC>::PRIMITIVE, uc).Read(), 16);
    const SPoint loc(x,y);
    LOG.Message("print: (%d,%d)", loc.GetX(), loc.GetY());
  }

  template<class EC>
  void Uq_10109210DebugUtils10<EC>::Uf_5print(const UlamContext<EC> & uc, UlamRef<EC>& ur, Ui_Ut_102321s<EC>& Uv_3arg) const //native
  {
    u32 strval = Uv_3arg.read();
    const u8 * p = 
      uc
      .GetUlamClassRegistry()
      .GetUlamClassByIndex(Ui_Ut_102321s<EC>::getRegNum(strval))
      ->GetString(Ui_Ut_102321s<EC>::getStrIdx(strval));
    LOG.Message("print: %S", p);
  }

  template<class EC>
  void Uq_10109210DebugUtils10<EC>::Uf_919printChar(const UlamContext<EC> & uc, UlamRef<EC>& ur, Ui_Ut_10181u<EC>& Uv_3arg) const //native
  {
    u32 uval = Uv_3arg.read();
    if (isprint(uval))
      LOG.Message("print: char \'%c\' (0%03o,%d,0x%02x)", uval,uval,uval,uval);
    else
      LOG.Message("print: char \'\\%03o'", uval);
  }

  template<class EC>
  void Uq_10109210DebugUtils10<EC>::Uf_5print(const UlamContext<EC> & uc, UlamRef<EC>& ur, Ui_Ut_10131i<EC>& Uv_3arg) const //native
  {
    s32 tmp = Uv_3arg.read();
    tmp &= _GetNOnes32(3); //mask
    LOG.Message("print: Int(3) 0x%x", tmp);
  }


  template<class EC>
  void Uq_10109210DebugUtils10<EC>::Uf_5print(const UlamContext<EC> & uc, UlamRef<EC>& ur, Ui_Ut_10141i<EC>& Uv_3arg) const //native
  {
    s32 tmp = Uv_3arg.read();
    tmp &= _GetNOnes32(4); //mask
    LOG.Message("print: Int(4) 0x%x", tmp);
  }


  template<class EC>
  void Uq_10109210DebugUtils10<EC>::Uf_5print(const UlamContext<EC> & uc, UlamRef<EC>& ur, Ui_Ut_102321i<EC>& Uv_3arg) const //native
  {
    s32 tmp = Uv_3arg.read();
    LOG.Message("print: Int: %d", tmp);
  }


  template<class EC>
  void Uq_10109210DebugUtils10<EC>::Uf_5print(const UlamContext<EC> & uc, UlamRef<EC>& ur, Ui_Ut_102321u<EC>& Uv_3arg) const //native
  {
    u32 tmp = Uv_3arg.read();
    LOG.Message("print: Unsigned: %u", tmp);
  }


  template<class EC>
  void Uq_10109210DebugUtils10<EC>::Uf_5print(const UlamContext<EC> & uc, UlamRef<EC>& ur, Ui_Ut_10131y<EC>& Uv_3arg) const //native
  {
    u32 tmp = Uv_3arg.read();
    tmp &= _GetNOnes32(3); //mask
    u32 count1s = PopCount(tmp);
    LOG.Message("print: Unary(3) Arg: 0x%x", count1s);
  }


  template<class EC>
  void Uq_10109210DebugUtils10<EC>::Uf_5print(const UlamContext<EC> & uc, UlamRef<EC>& ur, Ui_Ut_10131b<EC>& Uv_3arg) const //native
  {
    u32 tmp = Uv_3arg.read();
    tmp &= _GetNOnes32(3); //mask
    bool b = _Bool32ToCbool(tmp, 3);
    LOG.Message("print: Bool(3) 0x%x (%s)", tmp, b ? "true" : "false");
  }

  // Note this is a template function, not a template class member!
  // It doesn't follow the ulam native function interface rules!
  template<class EC>
  inline void Uq_10109210DebugUtils10_printAtom(const UlamContext<EC>& uc,
                                                typename EC::ATOM_CONFIG::ATOM_TYPE atom, // call by value
                                                u32 flags,
                                                ByteSink & buff)
  {
    typedef typename EC::ATOM_CONFIG AC;
    typedef typename AC::ATOM_TYPE T;
    if (!flags) return;

    AtomSerializer<AC> as(atom);
    u32 type = atom.GetType();
    const Tile<EC> & tile = uc.GetEventWindow().GetTile();
    const UlamClassRegistry<EC> & ucr = tile.GetUlamClassRegistry();
    const Element<EC> * ep = tile.GetElement(type);

    typedef typename EC::ATOM_CONFIG AC;

    if (ep)
    {

      const UlamElement<EC> * uep = ep->AsUlamElement();
      if (uep)
      {
        uep->Print(ucr, buff, atom, flags, 25);
      }
      else
      {
        // Make do with minimal non-UlamElement printing here
        if (flags & UlamElement<EC>::PRINT_SYMBOL)
          buff.Printf("(%s)", ep->GetAtomicSymbol());
        if (flags & UlamElement<EC>::PRINT_FULL_NAME)
          buff.Printf("%s", ep->GetName());
        if (flags & (UlamElement<EC>::PRINT_ATOM_BODY|UlamElement<EC>::PRINT_MEMBER_VALUES))
        {
          if (type != T::ATOM_EMPTY_TYPE || atom != Element_Empty<EC>::THE_INSTANCE.GetDefaultAtom())
            buff.Printf("[%@]", &as);
        }
      }
      return;
    } // else fall back to hex

    buff.Printf("Undefined type [%04x]: %@",type, &as);
  }

  //! DebugUtils.ulam:10:   Void print(Atom a, Unsigned flags)
  template<class EC>
  void Uq_10109210DebugUtils10<EC>::Uf_5print(const UlamContext<EC>& uc, UlamRef<EC>& ur, Ui_Ut_102961a<EC>& Uv_1a, Ui_Ut_102321t<EC>& Uv_5flags) const
  {
    OString512 buff;
    T atom = Uv_1a.ReadAtom();
    u32 flags = Uv_5flags.read();
    Uq_10109210DebugUtils10_printAtom(uc, atom, flags, buff);
    if (buff.GetLength() > 0)
      LOG.Message("%s",buff.GetZString());
  } // Uf_5print

  //! DebugUtils.ulam:10:   Void print(UrSelf & a)
  template<class EC>
  void Uq_10109210DebugUtils10<EC>::Uf_5print(const UlamContext<EC>& uc, UlamRef<EC>& ur, Ui_Uq_r10106UrSelf10<EC>& Ur_1a) const
  {
    OString4096 buff;
    DebugPrint<EC>(uc, Ur_1a, buff);  
    buff.Printf("\n");
    if (buff.GetLength() > 0)
      LOG.Message("%s",buff.GetZString());
  } // Uf_5print

  template<class EC>
  void Uq_10109210DebugUtils10<EC>::Uf_6assert(const UlamContext<EC> & uc, UlamRef<EC>& ur, Ui_Ut_10111b<EC>& Uv_1b) const //native
  {
    bool btmp = Uv_1b.read();
    printf("assert: arg is %d\n",btmp);
    if(!btmp)
      {
	abort();
      }
    printf("after assert's abort: arg is %d\n",btmp);
  }

} //MFM
