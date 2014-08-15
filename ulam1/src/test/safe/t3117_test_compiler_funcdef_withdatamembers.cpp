#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3117_test_compiler_funcdef_withdatamembers)
  {
    std::string GetAnswerKey()
    {
      return std::string(" { Int a(4);  Int b(5);  Int c(0);  Int test() {  a 4 = b 5 = a b +b return } }\nExit status: 9");
    }
    
    std::string PresetTest(FileManagerString * fms)
    {
      bool rtn1 = fms->add("a.ulam","ulam {  Int a; Int test(){ a=4; b=5; return (a + b); } Int b,c; }"); //allows data member var defined after use; return an expression
      
      if(rtn1)
	return std::string("a.ulam");
      
      return std::string("");
    }
  }
  
  ENDTESTCASECOMPILER(t3117_test_compiler_funcdef_withdatamembers)
  
} //end MFM


