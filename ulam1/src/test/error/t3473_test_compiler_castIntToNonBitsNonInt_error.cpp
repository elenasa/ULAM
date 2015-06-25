#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3473_test_compiler_castIntToNonBitsNonInt_error)
  {
    std::string GetAnswerKey()
    {
      /*
	./A.ulam:7:5: ERROR: Converting Int Int(3) to Unary(3) requires explicit casting.
	./A.ulam:8:4: ERROR: Converting from Unsigned(4) to Unary(3) requires explicit casting for binary operator=.
	./A.ulam:9:4: ERROR: Attempting to mix signed and unsigned types, LHS: Unsigned(32), RHS: Int(3), of an unsafe sized variable, to: Unsigned(32) for binary operator= without casting.
	./A.ulam:10:3: ERROR: Attempting to mix signed and unsigned types, LHS: Int(3), RHS: Unsigned(32), of an unsafe sized variable, to: Int(3) for binary operator= without casting.
	./A.ulam:11:4: ERROR: Attempting to mix signed and unsigned types, LHS: Int(3), RHS: Unsigned(3), of an unsafe sized variable, to: Int(3) for binary operator= without casting.
      */
      return std::string("Exit status: -1\n");
    }

    std::string PresetTest(FileManagerString * fms)
    {
      // no matter how big the unsigned, can't do it.
      bool rtn1 = fms->add("A.ulam","element A {\nInt test() {\nBits(3) t;\n Unsigned g = 1;\n Unary(3) u = 2; //constant ok\n Int(3) i = 3;\n  u = i;//int to anything non.. \n u = (Unsigned(4)) g; //3 bits ok\n g = i; //Int to anything non..\ni = g;\n i = (Unsigned(3)) g;\n /* same size bad cast */  t = i; //ok to bits\n return i;\n }\n }\n");

      if(rtn1)
	return std::string("A.ulam");

      return std::string("");
    }
  }

  ENDTESTCASECOMPILER(t3473_test_compiler_castIntToNonBitsNonInt_error)

} //end MFM
