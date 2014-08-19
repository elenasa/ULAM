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
	programme = P->parseProgram(output); //will be compared to answer
	//programme =  P->parseProgram();
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

    m_state.m_nodeEvalStack.addFrameSlots(1);     //prolog
    EvalStatus evs = root->eval();
    if(evs != NORMAL)
      {
	rtnValue =  -1;   //error!
      }
    else
      {
	 UlamValue rtnUV = m_state.m_nodeEvalStack.popArg();
	 rtnValue = rtnUV.m_valInt;
      }

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

    ((NodeProgram *) root)->generateCode(fm);

    delete fm;
  }

} //MFM
