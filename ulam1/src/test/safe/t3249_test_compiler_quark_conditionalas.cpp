#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3249_test_compiler_quark_conditionalas)
  {
    std::string GetAnswerKey()
    {
      return std::string("Ue_Foo { System s();  Bool(1) sp(false);  Bool(3) bi(true);  Bool(3) bh(true);  Int(32) d(3);  Int(32) test() {  Atom(96) a;  Foo f;  Bool(1) b;  a f cast = a Foo is cast cond bi true cast = if f a cast = f System has cast cond bh true cast = if b a System has = s ( b ! )assert . d a System has cast 3 cast +b = d return } }\nExit status: 3");
    }

    std::string PresetTest(FileManagerString * fms)
    {
      bool rtn2 = fms->add("Counter4.ulam", "ulam 1;\nquark Counter4 {\nInt d;\nVoid incr(){\nd+=1;\n}\nInt get(){\nreturn d;\n}\n}\n");

      if(rtn2)
	return std::string("Counter4.ulam");

      return std::string("");
    }
  }

  ENDTESTCASECOMPILER(t3249_test_compiler_quark_conditionalas)

} //end MFM
