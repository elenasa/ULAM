#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3298_test_compiler_sizeofclass_error)
  {
    std::string GetAnswerKey()
    {
      //./Fu.ulam:4:1: ERROR: Incomplete Variable Decl for type: Int(UNKNOWN) used with variable symbol name 'u' (UTI12) while bit packing class: Fu.
      return std::string("Ue_Fu { System s();  Int(32) u(0);  Int(32) v(64);  Int(32) test() {  u 0u cast = s ( u )print . v 64u cast = s ( v )print . v return } }\nExit status: 64");
    }

    std::string PresetTest(FileManagerString * fms)
    {
      bool rtn1 = fms->add("Fu.ulam", "ulam 1;\nelement Fu {\nBool b;\nInt(Fu.sizeof) u;\nInt test(){\n return 0;\n}\n}\n");

      if(rtn1)
	return std::string("Fu.ulam");

      return std::string("");
    }
  }

  ENDTESTCASECOMPILER(t3298_test_compiler_sizeofclass_error)

} //end MFM
