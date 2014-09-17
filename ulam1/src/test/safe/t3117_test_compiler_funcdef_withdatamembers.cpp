#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3117_test_compiler_funcdef_withdatamembers)
  {
    std::string GetAnswerKey()
    {
      return std::string("Ue_A { Int(32) a(4);  Int(32) b(5);  Int(32) test() {  a 4 = b 5 = a b +b return } }\nExit status: 9");
    }
    
    std::string PresetTest(FileManagerString * fms)
    {
      bool rtn1 = fms->add("A.ulam","element A {  Int test(){ a=4; b=5; return (a + b); } Int a,b; }"); //allows data member var defined after use; return an expression
      
      if(rtn1)
	return std::string("A.ulam");
      
      return std::string("");
    }
  }
  
  ENDTESTCASECOMPILER(t3117_test_compiler_funcdef_withdatamembers)
  
} //end MFM


