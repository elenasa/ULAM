#include "Ue_0026511A.h"

namespace MFM{

  Ue_0026511A::Ue_0026511A(){}

  Ue_0026511A::~Ue_0026511A(){}

  Ut_0023213Int Ue_0026511A::Uf_14test()
  {
    Um_11a = 3;
    Um_11b = 2;
    Um_11d = (Ut_001114Bool) (Um_11a = Um_11a ^ Um_11b);
    return (Um_11a);
  }

} //MFM

