#include <iostream>
#include <sstream>
#include <assert.h>
#include "Compiler.h"
#include "Tokenizer.h"
#include "Lexer.h"
#include "Preparser.h"
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

  TargetMap Compiler::getMangledTargetsMap()
  {
    TargetMap rtnTargets;
    m_state.m_programDefST.getTargets(rtnTargets);
    return rtnTargets;
  }

  ClassMemberMap Compiler::getMangledClassMembersMap()
  {
    ClassMemberMap rtnMembers;
    m_state.m_programDefST.getClassMembers(rtnMembers);
    return rtnMembers;
  }

  const std::string Compiler::getFullPathLocationAsString(const Locator& loc)
  {
    return m_state.getFullLocationAsString(loc);
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
	perrs += compileFile(startstr, errput, ss, P);
	it++;
      } //while, parse all files

    std::vector<std::string> unseenFileNames;
    while(!perrs && m_state.getUnseenClassFilenames(unseenFileNames))
      {
	std::vector<std::string>::iterator it = unseenFileNames.begin();
	while(it != unseenFileNames.end())
	  {
	    std::string startstr = *it;
	    perrs += compileFile(startstr, errput, ss, P);
	    it++;
	  }
	unseenFileNames.clear();
      }

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

  u32 Compiler::compileFile(std::string startstr, File * errput, SourceStream& ssref, Parser* p)
  {
    if(ssref.isPushed(startstr))
      {
	u32 compileThisId;
	if(m_state.getClassNameFromFileName(startstr, compileThisId))
	  {
	    SymbolClassName * cnsym = NULL;
	    if(m_state.alreadyDefinedSymbolClassName(compileThisId, cnsym))
	      {
		//prevent infinite loop
		if(cnsym->getUlamClass() == UC_UNSEEN)
		  {
		    std::ostringstream msg;
		    msg << "No class '";
		    msg << m_state.m_pool.getDataAsString(compileThisId).c_str();
		    msg << "' in <" << startstr.c_str() << ">";
		    errput->write(msg.str().c_str());
		    return 1;
		  }
	      }
	  }
	return 0;
      }

    u32 perrs = 0;
    u32 pmsg = ssref.push(startstr);
    if( pmsg > 0)
      {
	std::ostringstream msg;
	msg << "Compilation initialization FAILURE " << startstr.c_str();
	msg << ": <" << m_state.m_pool.getDataAsString(pmsg).c_str() << ">";
	errput->write(msg.str().c_str());
	perrs++;
      }

    // continue with Parser's parseProgram (including push error to avoid infinite l)
    perrs += p->parseProgram(startstr, errput); //will be compared to answer

    if (perrs)
      {
	std::ostringstream msg;
	msg << "Unrecoverable Program Parse FAILURE: <" << startstr.c_str() << ">\n";
	errput->write(msg.str().c_str());
      }
    return perrs;
  } //compileFile

  u32 Compiler::parseProgram(FileManager * fm, std::string startstr, File * output)
  {
    SourceStream ss(fm, m_state);
    Lexer * Lex = new Lexer(ss, m_state);
    Preparser * PP =  new Preparser(Lex, m_state);
    Parser * P = new Parser(PP, m_state);
    u32 perrs = 0;
    u32 pmsg = ss.push(startstr);
    if (pmsg == 0)
      {
	perrs = P->parseProgram(startstr, output); //will be compared to answer
      }
    else
      {
	std::ostringstream errmsg;
	errmsg << "parseProgram failed to start SourceStream: <";
	errmsg << m_state.m_pool.getDataAsString(pmsg).c_str() << ">";
	output->write(errmsg.str().c_str());
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

    //for regular classes and templates, only; since NNOs used
    //followed by the first c&l in case of re-orgs
    m_state.m_programDefST.updateLineageForTableOfClasses();

    u32 errCnt = m_state.m_err.getErrorCount();
    bool sumbrtn = (errCnt != 0);
    u32 infcounter = 0;
    while(!sumbrtn)
      {
	// resolve unknowns and size classes; sets "current" m_currentClassSymbol in CS
	m_state.m_err.clearCounts(); //warnings and errors
	sumbrtn = resolvingLoop();
	errCnt = m_state.m_err.getErrorCount();
	if(++infcounter > MAX_ITERATIONS || errCnt > 0)
	  {
	    std::ostringstream msg;
	    msg << errCnt << " Errors found during resolving loop --- ";
	    msg << "possible INCOMPLETE (or Template) class detected --- ";
	    msg << "after " << infcounter << " iterations";
	    MSG(m_state.getClassBlock()->getNodeLocationAsString().c_str(), msg.str().c_str(), DEBUG);
	    //note: not an error because template uses with deferred args remain unresolved
	    break;
	  }
      } //while

    if(!errCnt)
      {
	// type set at parse time (needed for square bracket checkandlabel);
	// so, here we just check for matching arg types (regular and Templates only).
	m_state.m_programDefST.checkCustomArraysForTableOfClasses();

	m_state.m_programDefST.checkDuplicateFunctionsForTableOfClasses();

	// must happen after type labeling and before eval (test)
	m_state.m_programDefST.calcMaxDepthOfFunctionsForTableOfClasses();

	// due to inheritance, might take more than a couple times around..
	infcounter = 0;
	sumbrtn = false;
	while(!sumbrtn)
	  {
	    // must happen after type labeling, check duplicateFunctions, and before eval (test)
	    sumbrtn = m_state.m_programDefST.calcMaxIndexOfVirtualFunctionsForTableOfClasses();
	    if(++infcounter > MAX_ITERATIONS)
	      {
		std::ostringstream msg;
		msg << "Incomplete calc of max index for virtual functions --- ";
		msg << "possible INCOMPLETE Super class detected ---";
		msg << " after " << infcounter << " iterations";
		MSG(m_state.getClassBlock()->getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
		//note: not an error because template uses with deferred args remain unresolved
		break;
	      }
	  } //while

	//after virtual table is set, check for abstract classes used as:
	//local var, data member, or func parameter types.
	m_state.m_programDefST.checkAbstractInstanceErrorsForTableOfClasses();

	// must happen after type labeling and before code gen;
	// separate pass. want UNKNOWNS reported
	m_state.m_programDefST.packBitsForTableOfClasses();

	// let Ulam programmer know the bits used/available (needs infoOn)
	m_state.m_programDefST.printBitSizeOfTableOfClasses();

	// determine any quark default values:
	m_state.m_programDefST.buildDefaultQuarksFromTableOfClasses();
      }

    // count Nodes with illegal Nav types; walk each class' data members and funcdefs.
    // clean up duplicate functions beforehand
    u32 navcount = 0;
    u32 hzycount = 0;
    u32 unsetcount = 0;

    m_state.m_programDefST.countNavNodesAcrossTableOfClasses(navcount, hzycount, unsetcount);
    errCnt = m_state.m_err.getErrorCount(); //latest count?
    if(navcount > 0)
      {
	// not necessarily goAgain, e.g. atom is Empty, where Empty is a quark instead of an element
	// the NodeTypeDescriptor is perfectly fine with a complete quark type, so no need to go again;
	// however, in the context of "is", this is an error and t.f. a Nav node.

	//assert(m_state.goAgain() || errCnt > 0); //sanity check; ran out of iterations
	assert(errCnt > 0); //sanity check; ran out of iterations

	std::ostringstream msg;
	msg << navcount << " Nodes with erroneous types detected after type labeling class: ";
	msg << m_state.getUlamTypeNameBriefByIndex(m_state.getCompileThisIdx()).c_str();
	MSG(m_state.getClassBlock()->getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
      }
    //else
    //assert(errCnt == 0);

    if(hzycount > 0)
      {
	//doesn't include incomplete stubs: if(a is S(x,y))
	//assert(m_state.goAgain()); //sanity check; ran out of iterations

	std::ostringstream msg;
	msg << hzycount << " Nodes with unresolved types detected after type labeling class: ";
	msg << m_state.getUlamTypeNameBriefByIndex(m_state.getCompileThisIdx()).c_str();
	MSG(m_state.getClassBlock()->getNodeLocationAsString().c_str(), msg.str().c_str(), INFO);
      }
    else
      assert(!m_state.goAgain());

    if(unsetcount > 0)
      {
	std::ostringstream msg;
	msg << unsetcount << " Nodes with unset types detected after type labeling class: ";
	msg << m_state.getUlamTypeNameBriefByIndex(m_state.getCompileThisIdx()).c_str();
	MSG(m_state.getClassBlock()->getNodeLocationAsString().c_str(), msg.str().c_str(), INFO);
      }

    u32 warns = m_state.m_err.getWarningCount();
    if(warns > 0)
      {
	std::ostringstream msg;
	msg << warns << " warning" << (warns > 1 ? "s " : " ") << "during type labeling";
	MSG(m_state.getClassBlock()->getNodeLocationAsString().c_str(), msg.str().c_str(), INFO);
      }

    errCnt = m_state.m_err.getErrorCount();
    if(errCnt > 0)
      {
	std::ostringstream msg;
	msg << errCnt << " TOO MANY TYPELABEL ERRORS";
	MSG(m_state.getClassBlock()->getNodeLocationAsString().c_str(), msg.str().c_str(), INFO);
      }

    // testing targetmap only
    //#define TESTTARGETMAP
#ifdef TESTTARGETMAP
    TargetMap tm = getMangledTargetsMap();
    std::cerr << "Size of target map is " << tm.size() << std::endl;
    for(TargetMap::const_iterator i = tm.begin(); i != tm.end(); ++i)
      {
	std::cerr
	  << "ULAM INFO: "  // Magic cookie text! ulam.tmpl recognizes it! emacs *compilation* doesn't!
	  << "TARGET "
	  << MFM::HexEscape(getFullPathLocationAsString(i->second.m_loc))
	  << " " << i->second.m_className
	  << " " << i->first
	  << " " << i->second.m_bitsize
	  << " " << (i->second.m_hasTest?"test":"notest")
	  << " " << (i->second.m_isQuark?"quark":"element")
	  << " " << MFM::HexEscape(i->second.m_structuredComment)
	  << std::endl;
      }
#endif

    // testing class member map only
    //#define TESTCLASSMEMBERMAP
#ifdef TESTCLASSMEMBERMAP
    ClassMemberMap cmm = getMangledClassMembersMap();
    std::cerr << "Size of class members map is " << cmm.size() << std::endl;
    for(ClassMemberMap::const_iterator i = cmm.begin(); i != cmm.end(); ++i)
      {
	u64 val;
	const MFM::ClassMemberDesc * cmd = i->second.getClassMemberDesc();
	std::cerr
	  << "ULAM INFO: "  // Magic cookie text! ulam.tmpl recognizes it! emacs *compilation* doesn't!
	  << cmd->getMemberKind() << " "
	  << MFM::HexEscape(getFullPathLocationAsString(cmd->m_loc))
	  << " " << cmd->m_mangledClassName
	  << " " << cmd->m_mangledType
	  << " " << cmd->m_memberName
	  << " " << cmd->m_mangledMemberName;

	if(cmd->getValue(val))
	  std::cerr << " 0x" << std::hex << val;

	std::cerr << " " << MFM::HexEscape(cmd->m_structuredComment)
		  << std::endl;
      }
#endif

    return m_state.m_err.getErrorCount();
  } //checkAndTypeLabelProgram

  bool Compiler::resolvingLoop()
  {
    bool sumbrtn = true;
    sumbrtn &= m_state.m_programDefST.setBitSizeOfTableOfClasses();
    sumbrtn &= m_state.m_programDefST.statusNonreadyClassArgumentsInTableOfClasses(); //without context
    sumbrtn &= m_state.m_programDefST.fullyInstantiateTableOfClasses(); //with ready args

    //checkAndLabelTypes: lineage updated incrementally
    sumbrtn &= m_state.m_programDefST.labelTableOfClasses(); //labelok, stubs not labeled, checks goagain flag!
    return sumbrtn;
  } //resolvingLoop

  bool Compiler::hasTheTestMethod()
  {
    return m_state.thisClassHasTheTestMethod();
  }

  bool Compiler::targetIsAQuark()
  {
    return m_state.thisClassIsAQuark();
  }

  // after checkAndTypeLabelProgram
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
	errorOutput->write("Error in making new file manager for test code generation...aborting");
	return;
      }
    m_state.m_programDefST.genCodeForTableOfClasses(fm);
    delete fm;
  } //generateCodedProgram (tests)

} //MFM
