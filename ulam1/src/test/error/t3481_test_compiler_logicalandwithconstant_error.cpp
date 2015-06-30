#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3481_test_compiler_logicalandwithconstant_error)
  {
    std::string GetAnswerKey()
    {
      //./A.ulam:6:6: ERROR: Cannot cast Int(32) to type: Bool(3).
      //SHOULDN"T MATCH!! error to cast a number as a Bool
      return std::string("Exit status: 1\nUe_A { Bool(3) d(true);  Int(32) test() {  Bool(3) a;  Bool(3) b;  a true cast = b true cast = d 7 cast b && = d cast return } }\n");
    }

    std::string PresetTest(FileManagerString * fms)
    {
      //informed by t3242
      bool rtn1 = fms->add("A.ulam","element A {\nBool(3) d;\nInt test(){Bool(3) a,b;\na = true;\nb = true;\nd = (Bool(3)) 7 && b;\nreturn d;\n }\n }\n");


      if(rtn1)
	return std::string("A.ulam");

      return std::string("");
    }
  }

  ENDTESTCASECOMPILER(t3481_test_compiler_logicalandwithconstant_error)

} //end MFM
