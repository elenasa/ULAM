#include <stdio.h>
#include <map>

#include "NodeProgram.h"
#include "CompilerState.h"

namespace MFM {

  NodeProgram::NodeProgram(CompilerState & state) : Node(state), m_root(NULL) {}

  NodeProgram::~NodeProgram()
  {
    delete m_root;
    m_root = NULL;
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
    return "ulam";
  }


  const std::string NodeProgram::prettyNodeName()
  {
    return nodeName(__PRETTY_FUNCTION__);
  }


  UlamType * NodeProgram::checkAndLabelType()
  { 
    assert(m_root);
    m_state.m_err.clearCounts();

    UlamType * rtnType =  m_root->checkAndLabelType();
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

      //output the mangled types
      File * fpt = fm->open("UlamTest_Types.h", WRITE);
      if(!fpt)
	{
	  assert(0);
	  return;      	 //error!
	}
      genMangledTypeHeaderFile(fpt);
      delete fpt;


      //output the UlamTest class
      File * fpc = fm->open("UlamTest_Class.h", WRITE);
      if(!fpc)
	{
	  assert(0);
	  return;      	  //error!
	}
      m_root->genCode(fpc);
      delete fpc;


      //create main.cpp
      File * fpm = fm->open("UlamTest_main.cpp", WRITE);
      if(!fpm)
	{
	  assert(0);
	  return; 	  //error!
	}
      m_state.m_currentIndentLevel = 0;
      m_state.indent(fpm);
      fpm->write("#include \"UlamTest_Types.h\"\n");
      m_state.indent(fpm);
      fpm->write("#include \"UlamTest_Class.h\"\n");
      fpm->write("\n");

      m_state.indent(fpm);
      fpm->write("int main()\n");

      m_state.indent(fpm);
      fpm->write("{\n");

      m_state.m_currentIndentLevel++;

      m_state.indent(fpm);
      fpm->write("MFM::UlamTest_Class utest;\n");

      m_state.indent(fpm);
      fpm->write("return utest.Uf_14test();\n");  //hardcoded mangled test name

      m_state.m_currentIndentLevel--;

      m_state.indent(fpm);
      fpm->write("}\n");
      delete fpm;
      //generateCode
    }


  void NodeProgram::genMangledTypeHeaderFile(File * fp)
  {
    m_state.m_currentIndentLevel = 0;
    fp->write("/**                                        -*- mode:C++ -*/\n\n");

    m_state.indent(fp);
    //fp->write("#include \"../../../include/itype.h\"\n"); //XXX
    fp->write("#include \"itype.h\"\n"); 
    fp->write("\n");

    //skip Nav type (0), and Void (1)
    u32 numTypes = m_state.m_indexToUlamType.size();
    for(u32 i = 1; i < numTypes; i++)
      {
	UlamType * ut = m_state.getUlamTypeByIndex(i);
	ut->genUlamTypeMangledDefinitionForC(fp, &m_state);
      }
  }

} //end MFM
