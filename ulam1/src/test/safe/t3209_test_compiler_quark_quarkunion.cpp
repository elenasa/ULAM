#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3209_test_compiler_quark_quarkunion)
  {
    std::string GetAnswerKey()
    {
      return std::string("Uq_Gah { Bool(3) b(false);  Int(4) a(0);  Unary(7) c(0);  Unsigned(6) d(0);  Bits(2) e(0);  Bool(1) f(false);  <NOMAIN> }\nExit status: -1");
    }
    
    std::string PresetTest(FileManagerString * fms)
    {      
      bool rtn4 = fms->add("Gah.ulam","ulam 1;\nunion Gah{\nBool(3) b;\nInt(4) a;\nUnary(7) c;\nUnsigned(6) d;\nBits(2) e;\nBool f;\nVoid set(Int xarg){\na=xarg;\n}\nVoid set(Unsigned xarg){\nd=xarg;\n}\nVoid set(Unary xarg){\nc=xarg;\n}\nVoid set(Bool xarg){\nb=xarg;\n}\n}\n");

      if(rtn4)
	return std::string("Gah.ulam");
      
      return std::string("");
    }
  }
  
  ENDTESTCASECOMPILER(t3209_test_compiler_quark_quarkunion)
  
} //end MFM


