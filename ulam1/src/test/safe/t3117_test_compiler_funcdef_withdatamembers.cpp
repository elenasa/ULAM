#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3117_test_compiler_funcdef_withdatamembers)
  {
    std::string GetAnswerKey()
    {
      return std::string(" { Int a(4);  Int b(5);  Int c(0);  Int main() {  a 4 = b 5 = } }\n");
    }
    
    std::string PresetTest(FileManagerString * fms)
    {
      //bool rtn1 = fms->add("a.ulam","ulam {  Int a; Int main(){ a=4; } }");
      //bool rtn1 = fms->add("a.ulam","ulam {  Int a, b; Int main(){ a=4; b=5; } }");
      //bool rtn1 = fms->add("a.ulam","ulam {  Int a; Int main(){ a=4; b=5; } Int b; }"); //allows data member var defined after use
      bool rtn1 = fms->add("a.ulam","ulam {  Int a; Int main(){ a=4; b=5; } Int b,c; }"); //allows data member var defined after use
      
      if(rtn1)
	return std::string("a.ulam");
      
      return std::string("");
    }
  }
  
  ENDTESTCASECOMPILER(t3117_test_compiler_funcdef_withdatamembers)
  
} //end MFM


