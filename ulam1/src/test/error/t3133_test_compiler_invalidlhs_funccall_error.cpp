#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3133_test_compiler_invalidlhs_funccall_error)
  {
    std::string GetAnswerKey()
    {
      //a.ulam:1:45: ERROR: (3) <foo> has 1 arguments for a defined function, that has 0 parameters.
      //a.ulam:1:51: ERROR: Not storeIntoAble: <foo>, is type: <Nav.8.0>.
      //a.ulam:1:32: Warning: Function 'test''s Return type's <Int.32.0> base type: <Int> does not match resulting type's <Nav.8.0> base type: <Nav>.
      //a.ulam:1:1: fyi: 1 warnings during type labeling.
      //a.ulam:1:1: fyi: 2 TOO MANY TYPELABEL ERRORS.
      //labelParseTree unrecoverable TYPE LABEL FAILURE.

      return std::string(" \n");
    }
    
    std::string PresetTest(FileManagerString * fms)
    {
      bool rtn1 = fms->add("D.ulam","quark D { Int(8) a; Int(8) foo() { return 1; } Int test() { foo() = a; return foo(); } }");
            
      if(rtn1)
	return std::string("D.ulam");
      
      return std::string("");
    }
  }
  
  ENDTESTCASECOMPILER(t3133_test_compiler_invalidlhs_funccall_error)
  
} //end MFM


