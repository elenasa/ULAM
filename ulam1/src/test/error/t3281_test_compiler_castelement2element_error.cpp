#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3281_test_compiler_castelement2element_error)
  {
    std::string GetAnswerKey()
    {
      return std::string("Exit status: 0\nUe_Tu { System s();  Int(32) me(0);  Int(32) test() {  Tu t;  Tu u;  u t cast = me return } }\nUq_System { <NOMAIN> }\n");
    }

    std::string PresetTest(FileManagerString * fms)
    {
      //can't cast to self
      bool rtn1 = fms->add("Tu.ulam", "ulam 1;\nuse System;\n element Tu {\nSystem s;\nInt me;\nInt test(){\nTu t;\nTu u;\n u = (Tu) t;\n return me;\n}\n}\n");

      // test system quark with native overloaded print funcs; assert
      bool rtn3 = fms->add("System.ulam", "ulam 1;\nquark System {\nVoid print(Unsigned arg) native;\nVoid print(Int arg) native;\nVoid print(Int(4) arg) native;\nVoid print(Int(3) arg) native;\nVoid print(Unary(3) arg) native;\nVoid print(Bool(3) arg) native;\nVoid assert(Bool b) native;\n}\n");

      if(rtn1 && rtn3)
	return std::string("Tu.ulam");

      return std::string("");
    }
  }

  ENDTESTCASECOMPILER(t3281_test_compiler_castelement2element_error)

} //end MFM
