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


namespace MFM {

  Compiler::Compiler()
  {  }

  Compiler::~Compiler()
  {  }

  //returns parse tree..
  u32 Compiler::start(FileManager * fm, std::string startstr, File * output, Node * & rtnNode)
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
  u32 Compiler::labelParseTree(Node * root, File * output)
  {
    assert(root);

    m_state.m_err.setFileOutput(output);

    root->checkAndLabelType(); //side-effects

    return m_state.m_err.getErrorCount();
  }


  // after labelParseTree
  u32 Compiler::evalParseTree(Node * root, File * output)
  {
    assert(root);

    m_state.m_err.setFileOutput(output);

    root->eval();

    return m_state.m_err.getErrorCount();
  }


  void Compiler::printPostFix(Node * root, File * output)
  {
    assert(root);

    m_state.m_err.setFileOutput(output);

    root->printPostfix(output);

    output->write("\n");
  }


  void Compiler::printParseTree(Node * root, File * output)
  {
    assert(root);

    m_state.m_err.setFileOutput(output);

    root->print(output);

    output->write("\n");
  }


} //MFM
