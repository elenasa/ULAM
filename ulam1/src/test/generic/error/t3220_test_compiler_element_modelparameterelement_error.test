#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3220_test_compiler_element_modelparameterelement_error)
  {
    std::string GetAnswerKey()
    {
      //./Foo.ulam:5:16: ERROR: Model Parameter 'poochance' cannot be based on a class type: Poo.
      //./Foo.ulam:5:16: ERROR: Invalid Model Parameter of Type: <Poo> and Name: <poochance>.
      return std::string("Exit status: -1\n");
    }

    std::string PresetTest(FileManagerString * fms)
    {
      bool rtn1 = fms->add("Foo.ulam","ulam 1;\nuse Poo;\nelement Foo {\nBool(1) sp;\n parameter Poo poochance;\n Bool last;\n Int test() {\n return -1;\n }\n }\n");

      bool rtn2 = fms->add("Poo.ulam"," ulam 1;\n element Poo {\n Bool(3) valb[3];\n  Void reset(Bool b) {\n valb[1] = b;\n }\n }\n");

      if(rtn1 && rtn2)
	return std::string("Foo.ulam");

      return std::string("");
    }
  }

  ENDTESTCASECOMPILER(t3220_test_compiler_element_modelparameterelement_error)

} //end MFM
