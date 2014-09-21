#include "TestCase_EndToEndParser.h"

namespace MFM {

  BEGINTESTCASEPARSER(t220_test_parser_controlifblock)
  {
    std::string GetAnswerKey()
    {
      return std::string("Ue_A { Int(32) a(6);  Int(32) b(1);  Int(32) test() {  a 7 cast = b 0 cast = a cast cond { b 1 b cast +b cast = a a cast 1 -b cast = } if b a = else a return } }\n");
    }
    
    std::string PresetTest(FileManagerString * fms)
    {
      bool rtn1 = fms->add("A.ulam","element A { Int a, b; Int test() {a = 7; b = 0; if( a ){ b = 1 + b; a = a - 1; } else b = a; return a;} }");
      
      if(rtn1)
	return std::string("A.ulam");
      
      return std::string("");
    }      
  }
  
  ENDTESTCASEPARSER(t220_test_parser_controlifblock)
  
} //end MFM


