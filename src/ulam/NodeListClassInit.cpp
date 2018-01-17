#include <stdio.h>
#include <assert.h>
#include "NodeInitDM.h"
#include "NodeListClassInit.h"
#include "CompilerState.h"

namespace MFM{

  NodeListClassInit::NodeListClassInit(UTI cuti, u32 classvarid, CompilerState & state) : NodeList(state), m_classUTI(cuti), m_classvarId(classvarid) { }

  NodeListClassInit::NodeListClassInit(const NodeListClassInit & ref) : NodeList(ref), m_classUTI(m_state.mapIncompleteUTIForCurrentClassInstance(ref.m_classUTI)), m_classvarId(ref.m_classvarId) { }

  NodeListClassInit::~NodeListClassInit() { }

  Node * NodeListClassInit::instantiate()
  {
    return new NodeListClassInit(*this);
  }

  void NodeListClassInit::printPostfix(File * fp)
  {
    fp->write("{");
    for(u32 i = 0; i < m_nodes.size(); i++)
      {
	if(i > 0)
	  fp->write(", ");
	m_nodes[i]->printPostfix(fp);
      }
    fp->write("}");
  } //printPostfix

  void NodeListClassInit::print(File * fp)
  {
    fp->write("{");
    for(u32 i = 0; i < m_nodes.size(); i++)
      {
	if(i > 0)
	  fp->write(", ");

	m_nodes[i]->print(fp);
      }
    fp->write("}");
  } //print

  const char * NodeListClassInit::getName()
  {
    //return variable of class being "init";
    return m_state.m_pool.getDataAsString(m_classvarId).c_str();
  }

  const std::string NodeListClassInit::prettyNodeName()
  {
    return nodeName(__PRETTY_FUNCTION__);
  }

  FORECAST NodeListClassInit::safeToCastTo(UTI newType)
  {
    //ulamtype checks for complete, non array, and type specific rules
    return m_state.getUlamTypeByIndex(newType)->safeCast(Node::getNodeType());
  }

  UTI NodeListClassInit::checkAndLabelType()
  {
    UTI rtnuti = Node::getNodeType();

    if(m_state.okUTItoContinue(rtnuti))
      return rtnuti; //t41167

    UTI nuti = m_classUTI;

    if(!m_state.okUTItoContinue(rtnuti) || !m_state.isComplete(nuti))
      {
	//incomplete, see if mapped; if so, update list
	UTI mappedUTI = nuti;
	UTI cuti = m_state.getCompileThisIdx();
	if(m_state.mappedIncompleteUTI(cuti, nuti, mappedUTI))
	  {
	    std::ostringstream msg;
	    msg << "Substituting Mapped UTI" << mappedUTI;
	    msg << ", " << m_state.getUlamTypeNameBriefByIndex(mappedUTI).c_str();
	    msg << " for incomplete type: '";
	    msg << m_state.getUlamTypeNameBriefByIndex(nuti).c_str();
	    msg << "' UTI" << nuti << " while labeling class: ";
	    msg << m_state.getUlamTypeNameBriefByIndex(cuti).c_str();
	    MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), DEBUG);
	    nuti = mappedUTI;
	  }

	if(!m_state.isComplete(nuti)) //reloads to recheck for debug message
	  {
	    std::ostringstream msg;
	    msg << "Incomplete descriptor for type: ";
	    msg << m_state.getUlamTypeNameBriefByIndex(nuti).c_str();
	    MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), WAIT); //t3787
	    Node::setNodeType(Hzy);
	    return Hzy;
	  }

	m_classUTI = rtnuti = nuti; //reset here and list
	for(u32 i = 0; i < m_nodes.size(); i++)
	  ((NodeInitDM *) m_nodes[i])->resetOfClassType(nuti);
      }


    for(u32 i = 0; i < m_nodes.size(); i++)
      {
	UTI puti = m_nodes[i]->checkAndLabelType();
	if(puti == Nav)
	  {
	    std::ostringstream msg;
	    msg << "Class Init Argument " << i + 1 << " has a problem";
	    msg << " for variable " << m_state.m_pool.getDataAsString(m_classvarId).c_str();
	    MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	    rtnuti = Nav;
	  }
	else if((rtnuti != Nav) && !m_state.isComplete(puti))
	  {
	    rtnuti = Hzy; // all or none
	    m_state.setGoAgain(); //since no error msg
	  }
	//else rtnuti remains == m_classUTI
      }

    setNodeType(rtnuti);
    return rtnuti;
  } //checkAndLabelType

  EvalStatus NodeListClassInit::evalToStoreInto(u32 n)
  {
    m_state.abortShouldntGetHere();
    return ERROR;
  }

  void NodeListClassInit::printUnresolvedLocalVariables(u32 fid)
  {
    m_state.abortShouldntGetHere();
  } //printUnresolvedLocalVariables

  void NodeListClassInit::calcMaxDepth(u32& depth, u32& maxdepth, s32 base)
  {
    m_state.abortShouldntGetHere();
  } //calcMaxDepth

  bool NodeListClassInit::isAConstant()
  {
    bool isconstant = true;
    for(u32 i = 0; i < m_nodes.size(); i++)
      isconstant &= m_nodes[i]->isAConstant();
    return isconstant;
  }

  void NodeListClassInit::genCode(File * fp, UVPass& uvpass)
  {
    m_state.indent(fp);
    fp->write("{\n");

    m_state.m_currentIndentLevel++;

    for(u32 i = 0; i < m_nodes.size(); i++)
      {
	m_nodes[i]->genCode(fp, uvpass);
      }

    m_state.m_currentIndentLevel--;

    m_state.indent(fp);
    fp->write("}\n");
  }

  void NodeListClassInit::genCode(File * fp, UVPass& uvpass, u32 n)
  {
    assert(n < m_nodes.size());
    m_nodes[n]->genCode(fp, uvpass);
  }

  void NodeListClassInit::genCodeToStoreInto(File * fp, UVPass& uvpass, u32 n)
  {
    m_state.abortShouldntGetHere();
  }

  void NodeListClassInit::genCodeConstantArrayInitialization(File * fp)
  {
    m_state.abortShouldntGetHere();
  }

  void NodeListClassInit::generateBuiltinConstantArrayInitializationFunction(File * fp, bool declOnly)
  {
    m_state.abortShouldntGetHere();
  }

  bool NodeListClassInit::initDataMembersConstantValue(BV8K& bvref)
  {
    //bvref contains default value at pos 0 of our m_forClassUTI.
    bool rtnok = true;
    for(u32 i = 0; i < m_nodes.size(); i++)
      {
	rtnok &= ((NodeInitDM *) m_nodes[i])->buildDefaultValue(0, bvref); //first arg dummy
      }
    return rtnok;
  }

} //MFM
