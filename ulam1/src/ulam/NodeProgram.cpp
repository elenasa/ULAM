#include <stdio.h>
#include <map>

#include "NodeProgram.h"
#include "CompilerState.h"

namespace MFM {

  static const char * CModeForHeaderFiles = "/**                                        -*- mode:C++ -*/\n\n";

  NodeProgram::NodeProgram(u32 id, CompilerState & state) : Node(state), m_root(NULL), m_compileThisId(id) {}

  NodeProgram::~NodeProgram()
  {
    //m_root deletion handled by m_programDefST;
  }


  void NodeProgram::setRootNode(NodeBlockClass * root)
  {
    m_root = root;
  }


  void NodeProgram::print(File * fp)
  {
    printNodeLocation(fp);
    UlamType * myut = getNodeType();
    char id[255];
    if(!myut)
      sprintf(id,"%s<NOTYPE>\n", prettyNodeName().c_str());
    else
      sprintf(id,"%s<%s>\n", prettyNodeName().c_str(), myut->getUlamTypeName(&m_state).c_str());
    fp->write(id);

    //overrides NodeBlock print, has m_root, no m_node or m_nextNode.
    if(m_root)
      m_root->print(fp);
    else 
      fp->write("<NULL>\n");

    sprintf(id,"-----------------%s\n", prettyNodeName().c_str());
    fp->write(id);
  }


  void NodeProgram::printPostfix(File * fp)
  {
    if(m_root)
	m_root->printPostfix(fp);
    else 
      fp->write("<NULL>\n");
  }


  const char * NodeProgram::getName()
  {
    return  m_state.m_pool.getDataAsString(m_compileThisId).c_str();
  }


  const std::string NodeProgram::prettyNodeName()
  {
    return nodeName(__PRETTY_FUNCTION__);
  }


  UlamType * NodeProgram::checkAndLabelType()
  { 
    assert(m_root);
    m_state.m_err.clearCounts();

    // label all the class; sets "current" m_currentClassSymbol in CS
    m_state.m_programDefST.labelTableOfClasses(m_state);

    //UlamType * rtnType =  m_root->checkAndLabelType();
    UlamType * rtnType =  m_root->getNodeType();
    setNodeType(rtnType);   //void type. missing?

    u32 warns = m_state.m_err.getWarningCount();
    if(warns > 0)
      {
	std::ostringstream msg;
	msg << warns << " warnings during type labeling";
	MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), INFO);
      }

    u32 errs = m_state.m_err.getErrorCount();
    if(errs > 0)
      {
	std::ostringstream msg;
	msg << errs << " TOO MANY TYPELABEL ERRORS";
	MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), INFO);
      }

    return rtnType;
  }


   EvalStatus NodeProgram::eval()
  {
    assert(m_root);
    m_state.m_err.clearCounts();

    setNodeType(m_state.getUlamTypeByIndex(Int)); //for testing

    evalNodeProlog(1);    //new current frame pointer for nodeeval stack
    EvalStatus evs = m_root->eval();

    // output informational warning and error counts
    u32 warns = m_state.m_err.getWarningCount();
    if(warns > 0)
      {
	std::ostringstream msg;
	msg << warns << " warnings during eval";
	MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), INFO);
      }

    u32 errs = m_state.m_err.getErrorCount();
    if(errs > 0)
      {
	std::ostringstream msg;
	msg << errs << " TOO MANY EVAL ERRORS";
	MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), INFO);
      }

    if(evs == NORMAL)
      {
	UlamValue testUV = m_state.m_nodeEvalStack.popArg();
	assignReturnValueToStack(testUV);     //for testProgram in Compiler
      }

    evalNodeEpilog();
    return evs;
  }


  void NodeProgram::generateCode(FileManager * fm)
    {
      assert(m_root);
      m_state.m_err.clearCounts();

      m_state.m_classBlock = m_root;  //reset to cmopileThis' class block

      // mangled types and forward class declarations
      genMangledTypeHeaderFile(fm);    

      // this class header
      {
	File * fp = fm->open(m_state.getFileNameForThisClassHeader().c_str(), WRITE);
	assert(fp);

	m_root->genCode(fp);      //compileThisId only, class block
	delete fp;
      }

      // this class body
      {
	File * fp = fm->open(m_state.getFileNameForThisClassBody().c_str(), WRITE);
	assert(fp);

	m_root->genCodeBody(fp);  //compileThisId only, MFM namespace

	generateMain(fp);  	  //append main to .cpp
	delete fp;
      }
    }     //generateCode


  // append main to .cpp for debug useage
  // outside the MFM namespace !!!
  void NodeProgram::generateMain(File * fp)
  {
    /*
    File * fp = fm->open(m_state.getFileNameForThisClassMain().c_str(), WRITE);
    assert(fp);
    
    m_state.m_currentIndentLevel = 0;
    m_state.indent(fp);
    fp->write("#include \"");
    fp->write(m_state.getFileNameforThisTypesHeader().c_str());
    fp->write("\"\n");

    m_state.indent(fp);
    fp->write("#include \"");
    fp->write(m_state.getFileNameforThisClassHeader().c_str());
    fp->write("\"\n");
    */

    fp->write("\n");
    m_state.indent(fp);
    fp->write("int main()\n");
    
    m_state.indent(fp);
    fp->write("{\n");

    m_state.m_currentIndentLevel++;

    //declare an instance of This class
    Symbol * csym = m_state.m_programDefST.getSymbolPtr(m_compileThisId);

    m_state.indent(fp);
    fp->write("MFM::");
    fp->write(csym->getUlamType()->getUlamTypeMangledName(&m_state).c_str());
    fp->write(" utest;\n");
    
    m_state.indent(fp);
    fp->write("return utest.Uf_14test();\n");  //hardcoded mangled test name
	
    m_state.m_currentIndentLevel--;

    m_state.indent(fp);
    fp->write("}\n");
    //delete fp;
  }


  void NodeProgram::genMangledTypeHeaderFile(FileManager * fm)
  {
    File * fp = fm->open(m_state.getFileNameForThisTypesHeader().c_str(), WRITE);
    assert(fp);
    
    m_state.m_currentIndentLevel = 0;
    fp->write(CModeForHeaderFiles);

    m_state.indent(fp);
    //use -I ../../../include in g++ command
    fp->write("#include \"itype.h\"\n"); 
    fp->write("\n");

    //skip Nav type (0)
    u32 numTypes = m_state.m_indexToUlamType.size();
    for(u32 i = 1; i < numTypes; i++)
      {
	UlamType * ut = m_state.getUlamTypeByIndex(i);
	ut->genUlamTypeMangledDefinitionForC(fp, &m_state);
      }
    delete fp;
  }

} //end MFM
