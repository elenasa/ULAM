#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3504_test_compiler_arraywithconstantindex)
  {
    std::string GetAnswerKey()
    {
      return std::string("Exit status: 1\nUe_A { Bool(1) a[5](false,false,false,false,true);  constant Int(6) mpi = 4;  Int(32) test() {  a 4 [] true = a 4 [] cast return } }\n");
    }

    std::string PresetTest(FileManagerString * fms)
    {
      //informed by t3503 (MP in test is NULL POINTER!); use a constant here instead.
      // return needs explicit cast to Int.
      bool rtn1 = fms->add("A.ulam","element A {\n Bool a[mpi+1];\n constant Int(6) mpi = 4;\n Int test() {\n a[mpi] = true;\n return (Int) a[mpi];\n}\n }\n");

      if(rtn1)
	return std::string("A.ulam");

      return std::string("");
    }
  }

  ENDTESTCASECOMPILER(t3504_test_compiler_arraywithconstantindex)

} //end MFM
