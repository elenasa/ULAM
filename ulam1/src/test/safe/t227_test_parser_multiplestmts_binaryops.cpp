#include "TestCase_EndToEndParser.h"

namespace MFM {

  BEGINTESTCASEPARSER(t227_test_parser_multiplestmts_binaryops)
  {
    std::string GetAnswerKey()
    {
      return std::string(" { Int a(1);  Int b(1);  Int c(1);  Int d(1);  Int e(2);  Int f(0);  Int g(1);  Int h(1);  Int test() {  a b c d 1 = = = = e a b +b = f a b -b = g c d * = h d d / = e f +b g +b h +b return } }\n");
    }
    
    std::string PresetTest(FileManagerString * fms)
    {
      // multiple statements, each of the binary ops, where all idents == 1
      // when using the same variable for each result, the final value is returned throughout.
      // to see intermediate results use different variables:
      bool rtn1 = fms->add("a.ulam","ulam{Int a, b, c, d, e, f, g, h; Int test() { a = b = c = d = 1; e = a + b; f = a - b; g = c * d; h = d / d; return (e + f + g + h); } }"); 
      
      if(rtn1)
	return std::string("a.ulam");
      
      return std::string("");
    }      
  }
  
  ENDTESTCASEPARSER(t227_test_parser_multiplestmts_binaryops)
  
} //end MFM


