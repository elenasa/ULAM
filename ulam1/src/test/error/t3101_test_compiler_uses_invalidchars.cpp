#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3101_test_compiler_uses_invalidchars)
  {
    std::string GetAnswerKey()
    {
      return std::string(" { Int test() { Int a(5);  a 3 2 +b = } }\n");
    }
    
    std::string PresetTest(FileManagerString * fms)
    {
      //foo.ulam:1:8: ERROR: Weird! Lexer could not find match for: <@>.
      //foo.ulam:1:1: ERROR: No parse tree.
      bool rtn1 = fms->add("a.ulam","use foo; Int a; use test;  a = @3 + @2; return a; use bar; load bar; ");
      bool rtn2 = fms->add("foo.ulam","ulam { @ ");  //recovers from invalid character
      bool rtn3 = fms->add("bar.ulam","} \n ");
      bool rtn4 = fms->add("test.ulam","Int test() { ");
      
      if(rtn1 && rtn2 && rtn3 && rtn4)
	return std::string("a.ulam");
      
      return std::string("");
    }
  }
  
  ENDTESTCASECOMPILER(t3101_test_compiler_uses_invalidchars)
  
} //end MFM


