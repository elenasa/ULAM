/**                                      -*- mode:C++ -*- */

//#include <iostream> //needed locale??? for cout
//#include <stdio.h>   //for printf
#include <stdlib.h>  //for abort

namespace MFM{


  template<class EC, u32 POS>
  void Uq_10109210DebugUtils<EC, POS>::Uf_5print(UlamContext<EC> & uc, T& Uv_4self, Ui_Ut_10133Int Uv_3arg) //native
  {
    s32 tmp = Uv_3arg.read();
    tmp &= _GetNOnes32(3); //mask
    LOG.Message("print: Int(3) 0x%x", tmp);
  }


  template<class EC, u32 POS>
  void Uq_10109210DebugUtils<EC, POS>::Uf_5print(UlamContext<EC> & uc, T& Uv_4self, Ui_Ut_10143Int Uv_3arg) //native
  {
    s32 tmp = Uv_3arg.read();
    tmp &= _GetNOnes32(4); //mask
    LOG.Message("print: Int(4) 0x%x", tmp);
  }


  template<class EC, u32 POS>
  void Uq_10109210DebugUtils<EC, POS>::Uf_5print(UlamContext<EC> & uc, T& Uv_4self, Ui_Ut_102323Int Uv_3arg) //native
  {
    s32 tmp = Uv_3arg.read();
    LOG.Message("print: Int: %d", tmp);
  }


  template<class EC, u32 POS>
  void Uq_10109210DebugUtils<EC, POS>::Uf_5print(UlamContext<EC> & uc, T& Uv_4self, Ui_Ut_102328Unsigned Uv_3arg) //native
  {
    u32 tmp = Uv_3arg.read();
    LOG.Message("print: Unsigned: %u", tmp);
  }


  template<class EC, u32 POS>
  void Uq_10109210DebugUtils<EC, POS>::Uf_5print(UlamContext<EC> & uc, T& Uv_4self, Ui_Ut_10135Unary Uv_3arg) //native
  {
    u32 tmp = Uv_3arg.read();
    tmp &= _GetNOnes32(3); //mask
    u32 count1s = PopCount(tmp);
    LOG.Message("print: Unary(3) Arg: 0x%x", count1s);
  }


  template<class EC, u32 POS>
  void Uq_10109210DebugUtils<EC, POS>::Uf_5print(UlamContext<EC> & uc, T& Uv_4self, Ui_Ut_10134Bool Uv_3arg) //native
  {
    u32 tmp = Uv_3arg.read();
    tmp &= _GetNOnes32(3); //mask
    bool b = _Bool32ToCbool(tmp, 3);
    LOG.Message("print: Bool(3) 0x%x (%s)", tmp, b ? "true" : "false");
  }


  template<class EC, u32 POS>
  void Uq_10109210DebugUtils<EC, POS>::Uf_5print(UlamContext<EC> & uc, T& Uv_4self, Ui_Ut_102964Atom<EC> Uv_3arg) //native
  {
    T atom = Uv_3arg.read();
    typedef typename EC::ATOM_CONFIG AC;
    AtomSerializer<AC> as(atom);
    LOG.Message("print: Atom %@",&as);
  }


  template<class EC, u32 POS>
  void Uq_10109210DebugUtils<EC, POS>::Uf_6assert(UlamContext<EC> & uc, T& Uv_4self, Ui_Ut_10114Bool Uv_1b) //native
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
