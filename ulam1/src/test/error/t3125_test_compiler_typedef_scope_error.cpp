#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3125_test_compiler_typedef_scope_error)
  {
    std::string GetAnswerKey()
    {
      //D.ulam:1:68: ERROR: Incomplete Var Decl for type: <Bar.0.-1> used with variable symbol name <f>.  NOPE!
      //D.ulam:1:76: ERROR: Invalid Type: <Bar.0.-1> used with [].
      //D.ulam:1:80: ERROR: Not storeIntoAble: <[]>, is type: <Nav.0.-1>. NOPE!
      //D.ulam:1:118: ERROR: Invalid Type: <Bar.0.-1> used with [].
      //D.ulam:1:118: ERROR: Invalid Type: <Bar.0.-1> used with [].
      // ERROR: Incomplete Class <Bar.0.-1> was never defined, fails labeling.
      //D.ulam:1:11: fyi: 6 TOO MANY TYPELABEL ERRORS.
      //Unrecoverable Program Type Label FAILURE.
      return std::string("Ue_D { Int(32) test() {  { typedef Int(32) Bar[2];  Bar e[2];  e 0 [] 4 = } 3 } }\n");
    }

    std::string PresetTest(FileManagerString * fms)
    {
      // Bar f gave error: a.ulam:1:69: (NodeTerminalIdent.cpp:labelType:61) ERROR: (2) <f> is not defined, and cannot be called.
      bool rtn1 = fms->add("D.ulam","element D { Int test() { { typedef Int Bar[2]; Bar e; e[0] = 4; }  Bar f; f[0] = 3;  /* match return type */ return f[0]; } }");

      if(rtn1)
	return std::string("D.ulam");

      return std::string("");
    }
  }

  ENDTESTCASECOMPILER(t3125_test_compiler_typedef_scope_error)

} //end MFM
