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
      sprintf(id,"%s<%s>\n", prettyNodeName().c_str(), myut->getUlamTypeName().c_str());
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


  void NodeProgram::genCode(File * fp)
    {
      assert(m_root);
      m_state.m_err.clearCounts();

      m_state.m_currentIndentLevel = 0;

      //output the UlamTest class first
      m_root->genCode(fp);

      m_state.m_currentIndentLevel = 0;

      fp->write("\n");
      //m_state.indent(fp);
      //fp->write("#include \"UlamTest.h\"\n");
      m_state.indent(fp);
      fp->write("int main()\n");

      m_state.indent(fp);
      fp->write("{\n");

      m_state.m_currentIndentLevel++;

      m_state.indent(fp);
      fp->write("MFM::UlamTest utest;\n");

      m_state.indent(fp);
      fp->write("return utest.test();\n");

      m_state.m_currentIndentLevel--;

      m_state.indent(fp);
      fp->write("}\n");
    }

} //end MFM
