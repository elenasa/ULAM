#include <stdio.h>
#include <map>
#include "NodeProgram.h"
#include "CompilerState.h"

namespace MFM {

  NodeProgram::NodeProgram(u32 id, CompilerState & state) : Node(state), m_root(NULL), m_compileThisId(id) {}

  NodeProgram::~NodeProgram()
  {
    //m_root deletion handled by m_programDefST;
  }


  void NodeProgram::updateLineage(Node * p)
  {
    setYourParent(p);             //god is null
    m_root->updateLineage(this);  //walk the tree..
  }


  void NodeProgram::setRootNode(NodeBlockClass * root)
  {
    m_root = root;
  }


  void NodeProgram::print(File * fp)
  {
    printNodeLocation(fp);
    UTI myut = getNodeType();
    char id[255];
    if(myut == Nav)
      sprintf(id,"%s<NOTYPE>\n", prettyNodeName().c_str());
    else
      sprintf(id,"%s<%s>\n", prettyNodeName().c_str(), m_state.getUlamTypeNameByIndex(myut).c_str());
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
    m_state.m_classBlock = m_root;  //reset to compileThis' class block
    m_state.m_currentBlock = m_state.m_classBlock;

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


  UTI NodeProgram::checkAndLabelType()
  {
    assert(m_root);
    m_state.m_err.clearCounts();

    m_root->updateLineage(this);

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
		MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
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
		MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
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
		MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
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

    UTI rtnType =  m_root->getNodeType();
    setNodeType(rtnType);   //void type. missing?

    // reset m_current class block, for next stage
    m_state.m_classBlock = m_root;  //reset to compileThis' class block
    m_state.m_currentBlock = m_state.m_classBlock;

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
  } //checkAndLabelType


  //performed across classes starting at NodeBlockClass
  void NodeProgram::countNavNodes(u32& cnt)
  {
    assert(0);
    return;
  }


   EvalStatus NodeProgram::eval()
  {
    assert(m_root);
    m_state.m_err.clearCounts();

    m_state.m_classBlock = m_root;  //reset to compileThis' class block
    m_state.m_currentBlock = m_state.m_classBlock;

    setNodeType(Int);     //for testing

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
  } //eval


  // guts moved to Symbol Class!!!!!!!!!!!
  void NodeProgram::generateCode(FileManager * fm)
  {
    assert(m_root);
    m_state.m_err.clearCounts();
    m_state.m_programDefST.genCodeForTableOfClasses(fm);
  } //generateCode


} //end MFM
