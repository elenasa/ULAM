#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3503_test_compiler_arraywithMPindex)
  {
    std::string GetAnswerKey()
    {
      return std::string("Exit status: 1\nUe_A { Bool(1) a[5](false,false,false,false,true);  Int(32) test() {  a mpi(4) [] true = a mpi(4) [] cast return } }\n");
    }

    std::string PresetTest(FileManagerString * fms)
    {
      //informed by t3311
      // return needs explicit cast to Int.
      bool rtn1 = fms->add("A.ulam","element A {\n Bool a[5];\n parameter Int(6) mpi = 4;\n Int test() {\n a[mpi] = true;\n return (Int) a[mpi];\n}\n }\n");

      if(rtn1)
	return std::string("A.ulam");

      return std::string("");
    }
  }

  ENDTESTCASECOMPILER(t3503_test_compiler_arraywithMPindex)

} //end MFM
