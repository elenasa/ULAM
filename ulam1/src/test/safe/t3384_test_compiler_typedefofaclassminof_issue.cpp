#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3384_test_compiler_typedefofaclassminof_issue)
  {
    std::string GetAnswerKey()
    {
      //test bug fix: typedef of a class' typedef
      //Ish2.ulam:6:16: ERROR: Invalid Class Type: <Tail>; KEYWORD should be: 'element', 'quark', or 'union'.
      //							    ERROR: No parse tree for This Class: Ish2.
      return std::string("Exit status: 0\nUe_R { Ish3 ish( Colony(3u) colony( constant Unsigned(32) widthc = 3;  typedef Telomeree(3u) Telo; ); );  Int(32) test() {  0 return } }\nUq_Ish3 { Colony(3u) colony( constant Unsigned(32) widthc = 3;  typedef Telomeree(3u) Telo; );  <NOMAIN> }\nUq_Colony { constant Unsigned(32) widthc = NONREADYCONST;  typedef Telomeree(width) Telo;  <NOMAIN> }\nUq_Telomeree { constant Unsigned(32) width = NONREADYCONST;  typedef Unsigned(UNKNOWN) Tail;  Unsigned(UNKNOWN) age(0);  <NOMAIN> }\n");
    }

    std::string PresetTest(FileManagerString * fms)
    {
      // likes Telomeree first!
      //bool rtn3 = fms->add("R.ulam", "use Telomeree;\n use Colony;\n use Ish3;\n  element R{\nIsh3 ish;\n Int test() { return 0;\n}\n}");
      bool rtn3 = fms->add("R.ulam", "use Ish3;\n  element R{\nIsh3 ish;\n Int test() { return 0;\n}\n}");

      //bool rtn1 = fms->add("Colony.ulam","quark Colony(Unsigned widthc){\n typedef Telomeree(widthc) Telo; \n  Bool foo() {\n Telo tail;\n if (tail.age >= Telomeree(widthc).Tail.maxof) {\n return true;\n }\n  /* if (tail.age >= Telo.Tail.maxof) {\n return true;\n }\n */ }\n }");

      //rtn1 simplified for debugging..
      //bool rtn1 = fms->add("Colony.ulam","quark Colony(Unsigned widthc){\n typedef Telomeree(widthc) Telo; \n Unsigned foo() {\n Telo tail;\n return Telomeree(widthc).Tail.maxof;\n }\n }");

      bool rtn1 = fms->add("Colony.ulam","use Telomeree;\n quark Colony(Unsigned widthc){\n typedef Telomeree(widthc) Telo; \n Unsigned foo() {\n return Telo.Tail.maxof;\n }\n }");

      bool rtn2 = fms->add("Ish3.ulam","use Colony;\nquark Ish3{\nColony(3) colony;\n }");

      bool rtn4 = fms->add("Telomeree.ulam","quark Telomeree(Unsigned width){\n typedef Unsigned(width) Tail;\n Tail age;\n }");

      if(rtn1 && rtn2 && rtn3 && rtn4)
	return std::string("R.ulam");

      return std::string("");
    }
  }

  ENDTESTCASECOMPILER(t3384_test_compiler_typedefofaclassminof_issue)

} //end MFM
