/* -*- c++ -*- */

#ifndef TESTCASEENDTOENDIO_H
#define TESTCASEENDTOENDIO_H

#include <string.h>
#include "File.h"
#include "FileString.h"
#include "FileManagerString.h"
#include "TestCase.h"

#define BEGINTESTCASEIO(n)				\
  struct TestCase_##n : public TestCase_EndToEndIO

#define ENDTESTCASEIO(n) ; static TestCase_##n the_instance;	\
  TestCase * n = &the_instance;

namespace MFM{

  class TestCase_EndToEndIO : public TestCase
  {
  public:

    virtual ~TestCase_EndToEndIO(){}

    /** called by driver to setup test, run test and check test results */
    virtual bool RunTests(File * errorOutput);
    
    /** setup test; return the string to start with */
    virtual std::string PresetTest(FileManagerString * fms) = 0;

    /** runs the test; returns true with results in output; false when error in output  */
    virtual bool GetTestResults(FileManager * fm, std::string startstr, File * output);    
    
    /** checks the results using the answer key to compare with */
    virtual bool CheckResults(FileManagerString * fms, File * output) = 0;


  protected:  

    /** helper method does simple compare of "results" with answerkey */
    bool CompareResultsWithAnswerKey(FileManagerString * fms, File * errorOutput);

    virtual std::string GetAnswerKey() = 0;

    private:    
  };

}

#endif //end TESTCASEENDTOENDIO_H
