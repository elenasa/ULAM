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
#include "NodeProgram.h"
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

  u32 Compiler::compileProgram(FileManager * infm, std::string startstr, FileManager * outfm, File * errput)
  {
    SourceStream ss(infm, m_state);

    Lexer * Lex = new Lexer(ss, m_state);
    Preparser * PP =  new Preparser(Lex, m_state);
    Parser * P = new Parser(PP, m_state);
    Node * programme = NULL;

    if (ss.push(startstr))
      {
	programme = P->parseProgram(startstr, errput); //will be compared to answer
        if (programme == 0)
          {
            errput->write("Unrecoverable Program Parse FAILURE.\n");
          }
        else if(checkAndTypeLabelProgram(programme, errput) == 0)
          {
	    //generateCodedProgram(programme, output);
	    // setup for codeGen
	    m_state.m_currentSelfSymbolForCodeGen = m_state.m_programDefST.getSymbolPtr(m_state.m_compileThisId);
	    m_state.m_currentObjSymbolsForCodeGen.clear();

	    m_state.setupCenterSiteForTesting();  //temporary!!!

	    ((NodeProgram *) programme)->generateCode(outfm);

	  }
	else
	  errput->write("Unrecoverable Program Type Label FAILURE.\n");
      }
    else
      {
	errput->write("Compilation initialization FAILURE.\n");
      }

    delete P;
    delete PP;
    delete Lex;
    delete programme;
    return m_state.m_err.getErrorCount();
  } //compileProgram


  //returns parse tree..
  u32 Compiler::parseProgram(FileManager * fm, std::string startstr, File * output, Node * & rtnNode)
  {
    SourceStream ss(fm, m_state);

    Lexer * Lex = new Lexer(ss, m_state);
    Preparser * PP =  new Preparser(Lex, m_state);
    Parser * P = new Parser(PP, m_state);
    Node * programme = NULL;

    if (ss.push(startstr))
      {
	programme = P->parseProgram(startstr, output); //will be compared to answer
      }
    else
      {
	output->write("parseProgram failed to start SourceStream.");
      }

    delete P;
    delete PP;
    delete Lex;
    rtnNode = programme;  //ownership transferred to caller
    return m_state.m_err.getErrorCount();
  }


  // call before eval parse tree; return zero when no errors
  u32 Compiler::checkAndTypeLabelProgram(Node * root, File * output)
  {
    assert(root);

    m_state.m_err.setFileOutput(output);

    root->checkAndLabelType();        //side-effects

    return m_state.m_err.getErrorCount();
  }


  // after checkAndTypeLabelProgram
  u32 Compiler::testProgram(Node * root, File * output, s32& rtnValue)
  {
    assert(root);

    m_state.m_err.setFileOutput(output);

    // set up an atom in eventWindow; init m_currentObjPtr to point to it
    // set up STACK since func call not called
    m_state.setupCenterSiteForTesting();

    m_state.m_nodeEvalStack.addFrameSlots(1);     //prolog, 1 for return
    EvalStatus evs = root->eval();
    if(evs != NORMAL)
      {
	rtnValue =  -1;   //error!
      }
    else
      {
	 UlamValue rtnUV = m_state.m_nodeEvalStack.popArg();
	 rtnValue = rtnUV.getImmediateData(32);
      }

    //#define CURIOUS_T3146
#ifdef CURIOUS_T3146
    //curious..
    {
      UlamValue objUV = m_state.m_eventWindow.loadAtomFromSite(c0.convertCoordToIndex());
      u32 data = objUV.getData(25,32);  //Int f.m_i (t3146)
      std::ostringstream msg;
      msg << "Output for m_i = <" << data << "> (expecting 4 for t3146)";
      MSG("",msg.str().c_str() , INFO);
    }
#endif

    m_state.m_nodeEvalStack.returnFrame();       //epilog
    return m_state.m_err.getErrorCount();
  }


  void Compiler::printPostFix(Node * root, File * output)
  {
    assert(root);

    m_state.m_err.setFileOutput(output);

    root->printPostfix(output);

    output->write("\n");
  }


  void Compiler::printProgramForDebug(Node * root, File * output)
  {
    assert(root);

    m_state.m_err.setFileOutput(output);

    root->print(output);

    output->write("\n");
  }


  void Compiler::generateCodedProgram(Node * root, File * errorOutput)
  {
    assert(root);
    m_state.m_err.setFileOutput(errorOutput);

    FileManagerStdio * fm = new FileManagerStdio("./src/test/bin"); //temporary!!!
    if(!fm)
      {
	errorOutput->write("Error in making new file manager for code generation...aborting");
	return;
      }


    // setup for codeGen
    m_state.m_currentSelfSymbolForCodeGen = m_state.m_programDefST.getSymbolPtr(m_state.m_compileThisId);
    m_state.m_currentObjSymbolsForCodeGen.clear();

    m_state.setupCenterSiteForTesting();  //temporary!!!

    ((NodeProgram *) root)->generateCode(fm);

    delete fm;
  }

} //MFM
