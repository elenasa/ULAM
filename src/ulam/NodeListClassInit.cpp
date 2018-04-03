#include <stdio.h>
#include <assert.h>
#include "NodeInitDM.h"
#include "NodeListClassInit.h"
#include "CompilerState.h"

namespace MFM{

  NodeListClassInit::NodeListClassInit(UTI cuti, u32 classvarid, CompilerState & state) : NodeList(state), m_classUTI(cuti), m_classvarId(classvarid) { }

  NodeListClassInit::NodeListClassInit(const NodeListClassInit & ref) : NodeList(ref), m_classUTI(m_state.mapIncompleteUTIForCurrentClassInstance(ref.m_classUTI,ref.getNodeLocation())), m_classvarId(ref.m_classvarId) { }

  NodeListClassInit::~NodeListClassInit() { }

  Node * NodeListClassInit::instantiate()
  {
    return new NodeListClassInit(*this);
  }

  void NodeListClassInit::printPostfix(File * fp)
  {
    fp->write(" { ");
    for(u32 i = 0; i < m_nodes.size(); i++)
      {
	if(i > 0)
	  fp->write(", ");
	m_nodes[i]->printPostfix(fp);
      }
    fp->write(" }");
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

  void NodeListClassInit::resetOfClassType(UTI cuti)
  {
    m_state.abortShouldntGetHere(); //override Node's noop
  }

  void NodeListClassInit::setClassType(UTI cuti) //from parent
  {
    assert(m_state.okUTItoContinue(cuti));
    if(!m_state.okUTItoContinue(m_classUTI) || !m_state.isComplete(m_classUTI))
      {
	m_classUTI = cuti; //reset
      }
    else if(!m_state.isScalar(m_classUTI))
      {
	UTI scalaruti = m_state.getUlamTypeAsScalar(m_classUTI);
	assert(UlamType::compare(scalaruti, cuti, m_state) == UTIC_SAME);
	m_classUTI = cuti; //t41185
      }
    for(u32 i = 0; i < m_nodes.size(); i++)
      {
	m_nodes[i]->resetOfClassType(cuti); //each a NodeInitDM
      }
  }

  bool NodeListClassInit::isClassInit()
  {
    return true;
  }

  UTI NodeListClassInit::checkAndLabelType()
  {
    UTI rtnuti = Node::getNodeType();

    if(m_state.okUTItoContinue(rtnuti) && m_state.isComplete(rtnuti))
      return rtnuti; //t41167,t41169

    UTI nuti = m_classUTI; //initially of-class-uti
    if(!m_state.isScalar(nuti))
      nuti = m_state.getUlamTypeAsScalar(nuti); //t41185

    if(!m_state.isComplete(nuti) || m_state.isAnonymousClass(nuti))
      {
	//incomplete, see if mapped; if so, update list
	UTI mappedUTI = nuti;
	UTI cuti = m_state.getCompileThisIdx();
	if(m_state.mappedIncompleteUTI(cuti, nuti, mappedUTI))
	  {
	    std::ostringstream msg;
	    msg << "Substituting Mapped UTI" << mappedUTI;
	    msg << ", " << m_state.getUlamTypeNameBriefByIndex(mappedUTI).c_str();
	    msg << " for incomplete initialized class type: '";
	    msg << m_state.getUlamTypeNameBriefByIndex(nuti).c_str();
	    msg << "' UTI" << nuti << " while labeling class: ";
	    msg << m_state.getUlamTypeNameBriefByIndex(cuti).c_str();
	    MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), DEBUG);
	    m_classUTI = nuti = mappedUTI;
	  }

	if(!m_state.isComplete(nuti)) //reloads to recheck for debug message
	  {
	    std::ostringstream msg;
	    msg << "Incomplete descriptor for initialized class type: ";
	    msg << m_state.getUlamTypeNameBriefByIndex(nuti).c_str();
	    MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), WAIT); //t41170
	    Node::setNodeType(Hzy);
	    m_state.setGoAgain(); //missing
	    return Hzy;
	  }
      }

    assert(m_state.okUTItoContinue(nuti) && m_state.isComplete(nuti));
    setClassType(nuti);

    rtnuti = nuti; //if all goes well..

    for(u32 i = 0; i < m_nodes.size(); i++)
      {
	assert(m_nodes[i]);
	UTI puti = m_nodes[i]->checkAndLabelType();
	if(puti == Nav)
	  {
	    std::ostringstream msg;
	    msg << "Class Init Argument " << i + 1 << " has a problem";
	    msg << " for variable " << m_state.m_pool.getDataAsString(m_classvarId).c_str();
	    msg << ", type " << m_state.getUlamTypeNameBriefByIndex(m_classUTI).c_str();
	    MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	    rtnuti = Nav;
	  }
	else if((rtnuti != Nav) && !m_state.isComplete(puti))
	  {
	    rtnuti = Hzy; // all or none
	  }
	//else rtnuti remains == m_classUTI
      }
    setNodeType(rtnuti);
    if(rtnuti == Hzy)
      m_state.setGoAgain(); //since no error msg
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
    return; //override NodeList
  } //calcMaxDepth

  bool NodeListClassInit::isAConstant()
  {
    bool isconstant = true;
    for(u32 i = 0; i < m_nodes.size(); i++)
      isconstant &= m_nodes[i]->isAConstant();
    return isconstant;
  }

  UTI NodeListClassInit::foldConstantExpression()
  {
    for(u32 i = 0; i < m_nodes.size(); i++)
      ((NodeInitDM *) m_nodes[i])->foldConstantExpression();

    m_state.tryToPackAClass(m_classUTI); //t41198 here?

    return Node::getNodeType();
  }

  UTI NodeListClassInit::constantFold()
  {
    return foldConstantExpression(); //t41170
  }

  void NodeListClassInit::genCode(File * fp, UVPass& uvpass)
  {
    //save before wipe out with each init dm; for local vars (o.w. empty)
    std::vector<Symbol *> saveCOSVector = m_state.m_currentObjSymbolsForCodeGen;

    m_state.indentUlamCode(fp);
    fp->write("{\n");

    m_state.m_currentIndentLevel++;

    for(u32 i = 0; i < m_nodes.size(); i++)
      {
	m_nodes[i]->genCode(fp, uvpass); //NodeInitDMs..
	m_state.m_currentObjSymbolsForCodeGen = saveCOSVector; //cleared by NodeVarDecl
      }

    m_state.m_currentIndentLevel--;

    m_state.indentUlamCode(fp);
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

  void NodeListClassInit::generateBuiltinConstantClassOrArrayInitializationFunction(File * fp, bool declOnly)
  {
    m_state.abortShouldntGetHere();
  }

  bool NodeListClassInit::initDataMembersConstantValue(BV8K& bvref, BV8K& bvmask)
  {
    //bvref contains default value at pos 0 (adjusted for elements!) of our m_forClassUTI.
    bool rtnok = true;
    for(u32 i = 0; i < m_nodes.size(); i++)
      {
	rtnok &= ((NodeInitDM *) m_nodes[i])->initDataMemberConstantValue(bvref, bvmask);
	if(!rtnok)
	  break;
      }
    return rtnok;
  }

} //MFM
