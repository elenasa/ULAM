#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3281_test_compiler_castelement2element)
  {
    std::string GetAnswerKey()
    {
      return std::string("Exit status: 0\nUe_Tu { System s();  Int(32) me(0);  Int(32) test() {  Atom(96) a;  Tu t;  Tu u;  u self = t u = a u cast cast = t a cast = me return } }\nUq_System { <NOMAIN> }\n");
    }

    std::string PresetTest(FileManagerString * fms)
    {
      bool rtn1 = fms->add("Tu.ulam", "ulam 1;\nuse System;\n element Tu {\nSystem s;\nInt me;\nInt test(){\nAtom a;\n Tu t;\nTu u;\n u = self;\n t = u;\n a =  (Tu) u;\n t = (Tu) a;\n return me;\n}\n}\n");

      // test system quark with native overloaded print funcs; assert
      bool rtn3 = fms->add("System.ulam", "ulam 1;\nquark System {\nVoid print(Unsigned arg) native;\nVoid print(Int arg) native;\nVoid print(Int(4) arg) native;\nVoid print(Int(3) arg) native;\nVoid print(Unary(3) arg) native;\nVoid print(Bool(3) arg) native;\nVoid assert(Bool b) native;\n}\n");

      if(rtn1 && rtn3)
	return std::string("Tu.ulam");

      return std::string("");
    }
  }

  ENDTESTCASECOMPILER(t3281_test_compiler_castelement2element)

} //end MFM
