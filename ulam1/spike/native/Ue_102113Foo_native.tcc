/**                                      -*- mode:C++ -*- */

//#include <iostream>
#include <stdio.h>

namespace MFM{

#if 0
  template<class CC>
  void Ue_102113Foo<CC>::Uf_5print(T& Uv_4self, BitVector<32> Uv_3arg)
  {    
       s32 tmp = Ui_Ut_10143Int::Read(Uv_3arg);
       // std::cout << "Arg: " << tmp << std::endl;
       printf("Arg: 0x%x\n", tmp);
  }	
#endif

  template<class CC>
  void Ue_102113Foo<CC>::Uf_5print(T& Uv_4self, Ui_Ut_10143Int Uv_3arg) //native
  {    
    //s32 tmp = Ui_Ut_10143Int::bf::Read(Uv_3arg.bv);
    s32 tmp = Uv_3arg.read();
    // std::cout << "Arg: " << tmp << std::endl;
    printf("Arg: 0x%x\n", tmp);
  } 

  template<class CC>
  void Ue_102113Foo<CC>::Uf_5print(T& Uv_4self, Ui_Ut_102323Int Uv_3arg) //native
  {    
    //s32 tmp = Ui_Ut_102323Int::bf::Read(Uv_3arg.bv);
    s32 tmp = Uv_3arg.read();
    // std::cout << "Arg: " << tmp << std::endl;
    printf("Arg: %d\n", tmp);
  } 

} //MFM
