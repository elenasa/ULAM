/**                                      -*- mode:C++ -*- */

//#include <iostream> //needed locale??? for cout
#include <stdio.h>   //for printf
#include <stdlib.h>  //for abort

namespace MFM{

  template<class CC, u32 POS>
  void Uq_10106System<CC, POS>::Uf_5print(T& Uv_4self, Ui_Ut_10143Int Uv_3arg) //native
  {    
    s32 tmp = Uv_3arg.read();
    printf("Arg: 0x%x\n", tmp);
  } 

  template<class CC, u32 POS>
  void Uq_10106System<CC, POS>::Uf_5print(T& Uv_4self, Ui_Ut_102323Int Uv_3arg) //native
  {    
    s32 tmp = Uv_3arg.read();
    printf("Arg: %d\n", tmp);
  } 

  template<class CC, u32 POS>
  void Uq_10106System<CC, POS>::Uf_6assert(T& Uv_4self, Ui_Ut_10114Bool Uv_1b) //native
  {
    bool btmp = Uv_1b.read();
    if(!btmp)
      {
	abort();
      }
  }

} //MFM
