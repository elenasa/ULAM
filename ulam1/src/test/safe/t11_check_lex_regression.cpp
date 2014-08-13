#include <string>
#include <iostream>
#include <errno.h>
#include "TestCase_EndToEndIO.h"


namespace MFM {

  BEGINTESTCASEIO(t11_check_lex_regression)
  {  
    std::string GetAnswerKey()
    {
      std::string answerkey = "a.ulam:1.1:TOK_IDENTIFIER(AAA)\na.ulam:1.3:TOK_EOF\n";
      return answerkey;
    }

    std::string PresetTest(FileManagerString * fms)
    {
      bool rtn1 = fms->add("a.ulam","AAA");
      
      if(rtn1)
	return std::string("a.ulam");

      return std::string("");
    }
    

    bool CheckResults(FileManagerString * fms, File * errorOutput)
    {    
      return CompareResultsWithAnswerKey(fms,errorOutput);
    }

    
  } //end test struct
  
  ENDTESTCASEIO(t11_check_lex_regression)

} //end MFM


