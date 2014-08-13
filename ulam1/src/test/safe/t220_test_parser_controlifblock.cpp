#include "TestCase_EndToEndParser.h"

namespace MFM {

  BEGINTESTCASEPARSER(t220_test_parser_controlifblock)
  {
    std::string GetAnswerKey()
    {
      return std::string(" { Int a(6);  Int b(1);  Int main() {  a 7 = b 0 = a cast cond { b 1 b +b = a a 1 -b = } ifthen b a = else a } }\n");
    }
    
    std::string PresetTest(FileManagerString * fms)
    {
      bool rtn1 = fms->add("a.ulam","ulam { Int a, b; Int main() {a = 7; b = 0; if( a ){ b = 1 + b; a = a - 1; } else b = a; a;} }");
      
      if(rtn1)
	return std::string("a.ulam");
      
      return std::string("");
    }      
  }
  
  ENDTESTCASEPARSER(t220_test_parser_controlifblock)
  
} //end MFM


