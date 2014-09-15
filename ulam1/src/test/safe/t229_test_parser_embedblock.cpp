#include "TestCase_EndToEndParser.h"

namespace MFM {

  BEGINTESTCASEPARSER(t229_test_parser_embedblock)
  {
    std::string GetAnswerKey()
    {
      //return std::string(" { Int j(5);  j 1 3 +b = { j j 1 +b = Float j(8.400000);  j 2.1 4 cast * = <EMPTYSTMT> Bool b(false);  b j cast ! = } Int i(0);  i<NULLRIGHT> = }\n");
      return std::string("Ue_A { Int j(5);  Bool b(false);  Int i(0);  Int test() {  j 1 3 +b = { j j 1 +b = Int j;  j 2 4 * = b j cast ! = } i b cast = j return } }\n");
    }
    
    std::string PresetTest(FileManagerString * fms)
    {
      //requires new grammar

      // includes statements after embedded block; use of variable
      // inside block defined outside then redefined inside; error
      // using variable defined inside block outside with 4 binary
      // operators; unary op; float;

      //bool rtn1 = fms->add("a.ulam","ulam\nInt j;\nInt test () {\n j = 1 + 3;\n{\nj = j + 1;\nFloat j;\nj = 2.1 * 4;\nInt j;\nBool b;\nb = !j;\n}\ni = b;\ni = 3 + b;\ni = 3 - b;\ni = 3 * b;i = 3 / b;\n} Int i;\n}\n"); 
      bool rtn1 = fms->add("A.ulam","element A{\nInt j;\nBool b;\nInt test () {\n j = 1 + 3;\n{\nj = j + 1;\nInt j;\nj = 2 * 4;\nb = !j;\n}\ni = b; return j;\n} Int i;\n}\n"); 
      
      if(rtn1)
	return std::string("A.ulam");
      
      return std::string("");
    }      
  }
  
  ENDTESTCASEPARSER(t229_test_parser_embedblock)
  
} //end MFM


