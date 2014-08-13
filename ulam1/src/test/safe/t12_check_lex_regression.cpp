#include <string>
#include <iostream>
#include <errno.h>
#include "TestCase_EndToEndIO.h"


namespace MFM {

  BEGINTESTCASEIO(t12_check_lex_regression)
  {  
    std::string GetAnswerKey()
    {
      //std::string answerkey = "bang/b.ulam:1.1:TOK_IDENTIFIER(bar)\nno/c_ake/c.ulam:1.1:TOK_IDENTIFIER(CCC)\nbang/b.ulam:2.15:TOK_SEMICOLON\na.ulam:1.11:TOK_SEMICOLON\na.ulam:2.11:TOK_SEMICOLON\nbang/b.ulam:1.1:TOK_IDENTIFIER(bar)\nbang/b.ulam:2.15:TOK_SEMICOLON\na.ulam:2.24:TOK_SEMICOLON\na.ulam:2.36:TOK_SEMICOLON\na.ulam:3.0:TOK_EOF\n";
      std::string answerkey = "bang/b.ulam:1.1:TOK_IDENTIFIER(bar)\nno/c_ake/c.ulam:1.1:TOK_IDENTIFIER(CCC)\nbang/b.ulam:1.1:TOK_IDENTIFIER(bar)\na.ulam:3.0:TOK_EOF\n";
      return answerkey;
    }

    std::string PresetTest(FileManagerString * fms)
    {
      bool rtn1 = fms->add("a.ulam","use bang.b;\nuse bang.b; load bang.b; use bang.b;\n");
      
      bool rtn2 = fms->add("bang/b.ulam", "bar\nuse no.c_ake.c;");
      
      bool rtn3 = fms->add("no/c_ake/c.ulam", "CCC");
      
      if(rtn1 && rtn2 && rtn3)
	return std::string("a.ulam");

      return std::string("");
    }
    

    bool CheckResults(FileManagerString * fms, File * errorOutput)
    {    
      return CompareResultsWithAnswerKey(fms,errorOutput);
    }

    
  } //end test struct
  
  ENDTESTCASEIO(t12_check_lex_regression)

} //end MFM


