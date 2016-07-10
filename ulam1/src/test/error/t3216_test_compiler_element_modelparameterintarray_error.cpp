#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3216_test_compiler_element_modelparameterintarray_error)
  {
    std::string GetAnswerKey()
    {
      /*
	./Foo.ulam:4:24: ERROR: Array size specified for model parameter.
	./Foo.ulam:4:18: ERROR: Invalid Model Parameter: Int chance.
      */
      return std::string("Exit status: -1\nUe_Foo { System s();  Bool(7) sp(false);  Bool(1) last(false);  Int(32) test() {  Foo f;  s ( chance 1 [] )print . f chance 1 [] . cast return } }\nUq_System { <NOMAIN> }\n");
    }

    std::string PresetTest(FileManagerString * fms)
    {
      //EP cannot be an array, since arrays cannot be initialized
      bool rtn1 = fms->add("Foo.ulam","ulam 1;\n element Foo {\nBool(7) sp;\nparameter Int(3) chance[2];\nBool(1) last;\nInt test() {\n Foo f;\nreturn f.chance[1];\n }\n }\n");

      if(rtn1)
	return std::string("Foo.ulam");

      return std::string("");
    }
  }

  ENDTESTCASECOMPILER(t3216_test_compiler_element_modelparameterintarray_error)

} //end MFM
