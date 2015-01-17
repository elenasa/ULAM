#include <iostream>
#include <sstream>
#include <assert.h>
#include "Compiler.h"
#include "SourceStream.h"
#include "Tokenizer.h"
#include "Lexer.h"
#include "Preparser.h"
#include "Parser.h"
#include "Token.h"
#include "FileManagerStdio.h"

namespace MFM {

  Compiler::Compiler()
  {  }

  Compiler::~Compiler()
  {  }


  std::string Compiler::getMangledTarget()
  {
    return m_state.getFileNameForThisClassCPP();
  }


  //compile one set of related Ulam files, as before.
  u32 Compiler::compileProgram(FileManager * infm, std::string startstr, FileManager * outfm, File * errput)
  {
    std::vector<std::string> afileToCompile;
    afileToCompile.push_back(startstr);
    return compileFiles(infm, afileToCompile, outfm, errput);
  }


  //compiles many unrelated Ulam files
  u32 Compiler::compileFiles(FileManager * infm, std::vector<std::string> filesToCompile, FileManager * outfm, File * errput)
  {
    SourceStream ss(infm, m_state);

    Lexer * Lex = new Lexer(ss, m_state);
    Preparser * PP =  new Preparser(Lex, m_state);
    Parser * P = new Parser(PP, m_state);
    u32 perrs = 0;

    std::vector<std::string>::iterator it = filesToCompile.begin();

    while(it != filesToCompile.end())
      {
	std::string startstr = *it;

	if(ss.isPushed(startstr))
	  {
	    it++;
	    continue;  //next..
	  }

	if(!ss.push(startstr))
	  {
	    std::ostringstream msg;
	    msg << "Compilation initialization FAILURE: <" << startstr.c_str() << ">\n";
	    errput->write(msg.str().c_str());
	  }


	// continue with Parser's parseProgram
	perrs += P->parseProgram(startstr, errput); //will be compared to answer
	if (perrs)
	  {
	    std::ostringstream msg;
	    msg << "Unrecoverable Program Parse FAILURE: <" << startstr.c_str() << ">\n";
	    errput->write(msg.str().c_str());
	  }
	it++;
      } //while, parse all files

    if(!perrs)
      {
	//across ALL parsed files
	if(checkAndTypeLabelProgram(errput) == 0)
	  {
	    m_state.m_programDefST.genCodeForTableOfClasses(outfm);
	  }
	else
	  {
	    std::ostringstream msg;
	    errput->write("Unrecoverable Program Type Label FAILURE.\n");
	  }
      }

    delete P;
    delete PP;
    delete Lex;
    return m_state.m_err.getErrorCount();
  } //compileFiles


  u32 Compiler::parseProgram(FileManager * fm, std::string startstr, File * output)
  {
    SourceStream ss(fm, m_state);

    Lexer * Lex = new Lexer(ss, m_state);
    Preparser * PP =  new Preparser(Lex, m_state);
    Parser * P = new Parser(PP, m_state);
    u32 perrs = 0;

    if (ss.push(startstr))
      {
	perrs = P->parseProgram(startstr, output); //will be compared to answer
      }
    else
      {
	output->write("parseProgram failed to start SourceStream.");
      }

    delete P;
    delete PP;
    delete Lex;

    return perrs;
  } //parseProgram


  // call before eval parse tree; return zero when no errors
  u32 Compiler::checkAndTypeLabelProgram(File * output)
  {
    m_state.m_err.setFileOutput(output);

    m_state.m_err.clearCounts();

    m_state.m_programDefST.updateLineageForTableOfClasses();

    // type set at parse time (needed for square bracket checkandlabel);
    // so, here we just check for matching arg types.
    m_state.m_programDefST.checkCustomArraysForTableOfClasses();

    // label all the class; sets "current" m_currentClassSymbol in CS
    m_state.m_programDefST.labelTableOfClasses();

    if(m_state.m_err.getErrorCount() == 0)
      {
	u32 infcounter = 0;
	// size all the class; sets "current" m_currentClassSymbol in CS
	while(!m_state.m_programDefST.setBitSizeOfTableOfClasses())
	  {
	    if(++infcounter > MAX_ITERATIONS)
	      {
		std::ostringstream msg;
		msg << "Possible INCOMPLETE class detected during type labeling class <";
		msg << m_state.m_pool.getDataAsString(m_state.m_compileThisId);
		msg << ">, after " << infcounter << " iterations";
		MSG("", msg.str().c_str(), ERR);
		break;
	      }
	  }

	u32 statcounter = 0;
	while(!m_state.statusUnknownBitsizeUTI())
	  {
	    if(++statcounter > MAX_ITERATIONS)
	      {
		std::ostringstream msg;
		msg << "Before bit packing, UNKNOWN types remain in class <";
		msg << m_state.m_pool.getDataAsString(m_state.m_compileThisId);
		msg << ">, after " << statcounter << " iterations";
		MSG("", msg.str().c_str(), ERR);
		break;
	      }
	  }

	statcounter = 0;
	while(!m_state.statusUnknownArraysizeUTI())
	  {
	    if(++statcounter > MAX_ITERATIONS)
	      {
		std::ostringstream msg;
		msg << "Before bit packing, types with UNKNOWN arraysizes remain in class <";
		msg << m_state.m_pool.getDataAsString(m_state.m_compileThisId);
		msg << ">, after " << statcounter << " iterations";
		MSG("", msg.str().c_str(), ERR);
		break;
	      }
	  }

	// count Nodes with illegal Nav types; walk each class' data members and funcdefs.
	m_state.m_programDefST.countNavNodesAcrossTableOfClasses();

	// must happen after type labeling and before code gen; separate pass.
	m_state.m_programDefST.packBitsForTableOfClasses();

	// let Ulam programmer know the bits used/available (needs infoOn)
	m_state.m_programDefST.printBitSizeOfTableOfClasses();
      }


    u32 warns = m_state.m_err.getWarningCount();
    if(warns > 0)
      {
	std::ostringstream msg;
	msg << warns << " warnings during type labeling";
	MSG("", msg.str().c_str(), INFO);
      }

    u32 errs = m_state.m_err.getErrorCount();
    if(errs > 0)
      {
	std::ostringstream msg;
	msg << errs << " TOO MANY TYPELABEL ERRORS";
	MSG("", msg.str().c_str(), INFO);
      }

    return m_state.m_err.getErrorCount();
  } //checkAndTypeLabelProgram


  bool Compiler::hasTheTestMethod()
  {
    return m_state.thisClassHasTheTestMethod();
  }

  bool Compiler::targetIsAQuark()
  {
    return m_state.thisClassIsAQuark();
  }

  // after checkAndTypeLabelProgram
  //u32 Compiler::testProgram(Node * root, File * output, s32& rtnValue)
  u32 Compiler::testProgram(File * output)
  {
    m_state.m_err.setFileOutput(output);
    m_state.m_programDefST.testForTableOfClasses(output);
    return m_state.m_err.getErrorCount();
  } //testProgram


  void Compiler::printPostFix(File * output)
  {
    m_state.m_err.setFileOutput(output);
    m_state.m_programDefST.printPostfixForTableOfClasses(output);
  } //printPostFix


  void Compiler::printProgramForDebug(File * output)
  {
    m_state.m_err.setFileOutput(output);
    m_state.m_programDefST.printForDebugForTableOfClasses(output);
  } //printProgramForDebug


  void Compiler::generateCodedProgram(File * errorOutput)
  {
    m_state.m_err.setFileOutput(errorOutput);

    FileManagerStdio * fm = new FileManagerStdio("./src/test/bin"); //temporary!!!
    if(!fm)
      {
	errorOutput->write("Error in making new file manager for code generation...aborting");
	return;
      }

    m_state.m_programDefST.genCodeForTableOfClasses(fm);
    delete fm;
  } //generateCodedProgram

} //MFM
