#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3105_test_compiler_bitwisefloaterror)
  {
    std::string GetAnswerKey()
    {
      /*
	./F.ulam:5:6: ERROR: Unsupported Number Type, Float <3.2>.
	./F.ulam:5:4: ERROR: RHS of binary operator= is missing, Assignment deleted.
	./F.ulam:6:6: ERROR: Unsupported Number Type, Float <2.1>.
	./F.ulam:6:4: ERROR: RHS of binary operator= is missing, Assignment deleted.
      */
      return std::string("F.ulam:1:59: ERROR: Invalid Operands of Types: Float and Float to binary operator^.\nF.ulam:1:1: fyi: 1 TOO MANY TYPELABEL ERRORS.\n { Int test () { Float a(3.200000);  Float b(2.100000);  Float c(Nav.8.0);  Bool d(Nav.8.0);  a 3.2 = b 2.1 = d c a b ^ cast = cast = d return } }\n");
    }

    std::string PresetTest(FileManagerString * fms)
    {
      bool rtn1 = fms->add("F.ulam","element F {\n use test;\n Float a, b, c;\n Bool d;\n a = 3.2;\n b = 2.1;\n d = c = a ^ b;\n return d;\n}\n }\n");
      bool rtn2 = fms->add("test.ulam", "Int test() {\n");

      if(rtn1 & rtn2)
	return std::string("F.ulam");

      return std::string("");
    }
  }

  ENDTESTCASECOMPILER(t3105_test_compiler_bitwisefloaterror)

} //end MFM
