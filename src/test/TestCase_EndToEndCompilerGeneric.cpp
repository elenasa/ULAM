#include <iostream>
#include <stdio.h>
#include "Compiler.h"
#include "FileManagerStdio.h"
#include "TestCase_EndToEndCompilerGeneric.h"

namespace MFM {

  TestCase_EndToEndCompilerGeneric::TestCase_EndToEndCompilerGeneric(File * input)
    : m_state(START)
  {
    if (!input) die("Null input pointer");
    parseTest(*input);
  }

  void TestCase_EndToEndCompilerGeneric::parseTest(File & input)
  {
    /*
      Look for lines matching:
      ## ANYTHING\n -> always discarded comment (even inside #:files etc)
      #=TESTNAME\n  -> test name
      #>FILE\n      -> next line begins ulam input file FILE which is the start file
      #:FILE\n      -> next line begins ulam input file FILE
      #!\n          -> next line begins answer key
      #.\n          -> end of test file (implied by EOF if omitted)
     */
    while (true) {

      readLine(input);

      if (getInput(0) != '#') {
        switch (m_state) {
        default:
          die("Impossible");
        case START:
          die("Non '#' content with no file or answer active");
        case IN_FILE:
          newFileData(m_currentLine);
          break;
        case ANSWER:
          newAnswerKeyData(m_currentLine);
          break;
        }
        continue;
      }

      // Line begins with '#'
      switch (getInput(1)) {

      default: die("Not [>=#:!.] after '#'");

      case '#': continue; // comment

      case '=':           // name of test
        if (m_testName.length() > 0)
          die("Multiple '#=' in test");
        if (m_currentLine.length() < 3)
          die("No name after '#=' in test");
        m_testName = m_currentLine.substr(2);
        continue;

      case '>':           // name of next input and start file
        if (m_startFileName.length() > 0)
          die("Multiple '#>' in test");
        m_startFileName = m_currentLine.substr(2);
        // FALL THROUGH

      case ':':           // name of next input file
        m_state = IN_FILE;
        newInputFile(m_currentLine.substr(2));
        continue;

      case '!':           // begin answer key
        m_state = ANSWER;
        continue;

      case '.':           // end of test input
        return;
      }
    }
  }

  void TestCase_EndToEndCompilerGeneric::readLine(File & input)
  {
    std::ostringstream os;

    for (s32 ch = input.consume(); ch >= 0; ch = input.consume()) {

      if (ch == '\n') {
        m_currentLine = os.str();
        return;
      }
      os << (char) ch;
    }

    if (os.str().length() != 0 && os.str() != "#.")
      die("Missing newline at end of input, after " + os.str());
    m_currentLine = "#.";
  }

  void TestCase_EndToEndCompilerGeneric::newInputFile(std::string str)
  {
    InputFile in;
    in.m_fileName = str;
    m_inputFiles.push_back(in);
  }

  TestCase_EndToEndCompilerGeneric::InputFile & TestCase_EndToEndCompilerGeneric::getLast()
  {
    if (m_inputFiles.size() == 0)
      die("getLast internal error");
    return m_inputFiles[m_inputFiles.size() - 1];
  }

  void TestCase_EndToEndCompilerGeneric::newFileData(std::string str)
  {
    InputFile & in = getLast();
    in.m_contents << str << '\n';
  }

  void TestCase_EndToEndCompilerGeneric::newAnswerKeyData(std::string str)
  {
    m_answerKey << str << '\n';
  }

  void TestCase_EndToEndCompilerGeneric::die(std::string msg)
  {
    std::cerr << "TestCase_EndToEndCompilerGeneric: TEST FILE FORMAT FAILURE" << std::endl;
    std::cerr << msg << std::endl;
    assert(0);
  }

  void TestCase_EndToEndCompilerGeneric::addUrSelf(FileManagerString & fms)
  {
#ifdef ULAM_SHARE_DIR  /* UrSelf lives in stdlib */
#define YY(s) XX(s)    /* expand */
#define XX(s) #s       /* stringify */
    const char * urSelfFile = YY(ULAM_SHARE_DIR) "/ulam/stdlib/UrSelf.ulam";
    FILE * fp = fopen(urSelfFile, "r");
    if (!fp) die("Can't load UrSelf.ulam");
    std::string content;
    int ch;
    while ((ch = fgetc(fp)) >= 0) content += (char) ch;
    fclose(fp);
    bool ret = fms.add("UrSelf.ulam",content.c_str());
    if (!ret) die("FileManagerString::LoadUrSelf failed");
#undef XX
#undef YY
#else  /* !ULAM_SHARE_DIR */
    die("ULAM_SHARE_DIR not configured");
#endif /* ULAM_SHARE_DIR */
  }

  void TestCase_EndToEndCompilerGeneric::addEmpty(FileManagerString & fms)
  {
#ifdef ULAM_SHARE_DIR  /* UrSelf lives in stdlib */
#define YY(s) XX(s)    /* expand */
#define XX(s) #s       /* stringify */
    const char * urSelfFile = YY(ULAM_SHARE_DIR) "/ulam/stdlib/Empty.ulam";
    FILE * fp = fopen(urSelfFile, "r");
    if (!fp) die("Can't load Empty.ulam");
    std::string content;
    int ch;
    while ((ch = fgetc(fp)) >= 0) content += (char) ch;
    fclose(fp);
    bool ret = fms.add("Empty.ulam",content.c_str());
    if (!ret) die("FileManagerString::LoadEmpty failed");
#undef XX
#undef YY
#else  /* !ULAM_SHARE_DIR */
    die("ULAM_SHARE_DIR not configured");
#endif /* ULAM_SHARE_DIR */
  }

  std::string TestCase_EndToEndCompilerGeneric::PresetTest(FileManagerString * fms)
  {
    if (!fms) die("Null FileManagerString");

    if (m_inputFiles.size() == 0) die("No input files in test");

    addUrSelf(*fms);
    addEmpty(*fms);

    for (u32 i = 0; i < m_inputFiles.size(); ++i) {
      InputFile & in = m_inputFiles[i];
      bool ret = fms->add(in.m_fileName, in.m_contents.str());
      if (!ret) die("FileManagerString::add failed");
    }

    if (m_startFileName.length() > 0)
      return m_startFileName;

    // Default start file is first file
    return m_inputFiles[0].m_fileName;
  } //presetTest

  bool TestCase_EndToEndCompilerGeneric::GetTestResults(FileManager * fm, std::string startstr, File * output)
  {
    Compiler C;
    FileManagerStdio * outfm = new FileManagerStdio("./src/test/bin"); //temporary!!!
    if(!outfm)
      {
	output->write("Error in making new file manager for test code generation...aborting");
	return false;
      }

    std::vector<std::string> filesToCompile;
    filesToCompile.push_back("UrSelf.ulam");
    filesToCompile.push_back("Empty.ulam");

    for (u32 i = 0; i < m_inputFiles.size(); ++i)
      {
	InputFile & in = m_inputFiles[i];
	if(C.ulamfilename(in.m_fileName))
	  {
	    filesToCompile.push_back(in.m_fileName);
	  }
	//else no .ulam suffix, do nothing
      }

    // error messages appended to output are compared to answer
    if(C.compileFiles(fm, filesToCompile, outfm, output) == 0)
      {
	//also available in NodeBlockClass::eval
	//#define SKIP_EVAL
#ifndef SKIP_EVAL
	if(C.testProgram(output) == 0)
	  {
	    C.printPostFix(output);
	  }
	else
	  output->write("Unrecoverable Program Test FAILURE.\n");
#endif
      }

    delete outfm;
    return true;
  } //GetTestResults

  std::string TestCase_EndToEndCompilerGeneric::GetAnswerKey()
  {
    std::string answer = m_answerKey.str();
    return answer;
  }

} //end MFM
