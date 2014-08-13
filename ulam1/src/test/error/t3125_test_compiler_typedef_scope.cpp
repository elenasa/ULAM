#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3125_test_compiler_typedef_scope)
  {
    std::string GetAnswerKey()
    {
      return std::string(" { Int main() {  { typedef Int Bar[2];  Bar e[2];  e 0 [] 4 = } 3 } }\n");
    }
    
    std::string PresetTest(FileManagerString * fms)
    {
      //bool rtn1 = fms->add("a.ulam","ulam { Foo foo(Bool b[3]) { Foo m; if(b[0]) m[0]=1; else m[0]=2; m;} Int main() { Bool mybool[3]; mybool[0] = true; mybool[1] = false; mybool[2]= false; d = foo(mybool); } Foo d; }");  // want d[0] == 1.
      //bool rtn1 = fms->add("a.ulam","ulam { typedef Int Bar [2]; Bar bar(Bar b) { typedef Bool L; L e; Bar m; e = b[0]; if(e) m[0]=1; else m[0]=2; m;} Int main() { Bar myb; myb[0] = true; myb[1] = false; d = bar(myb); d[0]; /* match return type */} Bar d; L f;}");  // want d[0] == 1.
      //bool rtn1 = fms->add("a.ulam","ulam { Int main() { Bool b; b = true; { typedef Int Bar[2]; Bar e; e[0] = b; } Bar f; f[0] = b; /* match return type */} }");  // want d[0] == 1.
      bool rtn1 = fms->add("a.ulam","ulam { Int main() { { typedef Int Bar[2]; Bar e; e[0] = 4; }  Bar f; f[0] = 3;  /* match return type */ 3;} }");  // Bar f gave error: a.ulam:1:69: (NodeTerminalIdent.cpp:labelType:61) ERROR: (2) <f> is not defined, and cannot be called.
      
      if(rtn1)
	return std::string("a.ulam");
      
      return std::string("");
    }
  }
  
  ENDTESTCASECOMPILER(t3125_test_compiler_typedef_scope)
  
} //end MFM


