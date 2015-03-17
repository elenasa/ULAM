#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3152_test_compiler_fortyonebools)
  {
    std::string GetAnswerKey()
    {
      // before error about data members limited to 32-bits
      return std::string("Exit status: 1\nUe_B { Int(32) test() {  Bool(1) m_bSites[41];  m_bSites 20 [] true = m_bSites 20 [] cast return } }\n");
    }

    std::string PresetTest(FileManagerString * fms)
    {
      bool rtn1 = fms->add("B.ulam","ulam 1;\n\n element B {\nInt test() {\n Bool m_bSites[41];\n m_bSites[20] = true;\n return m_bSites[20];\n}\n }\n");

      if(rtn1)
	return std::string("B.ulam");

      return std::string("");
    }
  }

  ENDTESTCASECOMPILER(t3152_test_compiler_fortyonebools)

} //end MFM
