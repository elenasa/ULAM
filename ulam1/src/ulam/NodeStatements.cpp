#include <stdio.h>
#include "NodeStatements.h"
#include "CompilerState.h"

namespace MFM {

  NodeStatements::NodeStatements(Node * s, CompilerState & state) : Node(state), m_node(s), m_nextNode(NULL) {}

  NodeStatements::~NodeStatements()
  {
    delete m_nextNode;
    m_nextNode = NULL;

    delete m_node;
    m_node = NULL;
  }

  void NodeStatements::updateLineage(Node * p)
  {
    setYourParent(p);
    if(m_node)
      m_node->updateLineage(this);
    if(m_nextNode)
      m_nextNode->updateLineage(this);
  }


  void NodeStatements::print(File * fp)
  {
    printNodeLocation(fp);  //has same location as it's node
    UTI myut = getNodeType();
    char id[255];
    if(myut == Nav)
      sprintf(id,"%s<NOTYPE>\n", prettyNodeName().c_str());
    else
      sprintf(id,"%s<%s>\n", prettyNodeName().c_str(), m_state.getUlamTypeNameByIndex(myut).c_str());
    fp->write(id);

    if(m_node)
      m_node->print(fp);
    else
      fp->write(" <EMPTYSTMT>\n");

    if(m_nextNode)
      m_nextNode->print(fp);
    else
      fp->write(" <NONEXTSTMT>\n");
    sprintf(id,"-----------------%s\n", prettyNodeName().c_str());
    fp->write(id);
  }


  void NodeStatements::printPostfix(File * fp)
  {
    assert(m_node);    //e.g. bad decl

    if(m_node)
      m_node->printPostfix(fp);
    else
      fp->write(" <EMPTYSTMT>");

    if(m_nextNode)
      m_nextNode->printPostfix(fp);
  }


  const char * NodeStatements::getName()
  {
    return "Stmts";  //?
  }


  const std::string NodeStatements::prettyNodeName()
  {
    return nodeName(__PRETTY_FUNCTION__);
  }


  UTI NodeStatements::checkAndLabelType()
  {
    assert(m_node);

    //unlike statements, blocks don't have an m_node
    m_node->checkAndLabelType();       //side-effect

    if(m_nextNode)
      m_nextNode->checkAndLabelType(); //side-effect

    //statements don't have types
    setNodeType(Void);
    return getNodeType();
  } //checkAndLabelType


  void NodeStatements::countNavNodes(u32& cnt)
  {
    if(m_node)
      m_node->countNavNodes(cnt);
    if(m_nextNode)
      m_nextNode->countNavNodes(cnt);
  }


  EvalStatus NodeStatements::eval()
  {
    assert(m_node);

    evalNodeProlog(0);
    makeRoomForNodeType(m_node->getNodeType());
    EvalStatus evs = m_node->eval();
    if(evs != NORMAL)
      {
	evalNodeEpilog();
	return evs;
      }

    //not the last one, so thrown out results and continue
    if(m_nextNode)
      {
	evalNodeEpilog();  //Tue Aug 26 16:18:43 2014
	evs = m_nextNode->eval();
	if(evs != NORMAL)
	  {
	    ////evalNodeEpilog();
	    //evalNodeEpilog(); //Tue Aug 26 16:18:56 2014
	    return evs;
	  }
      }
    else
      {
	//end, so copy to -1
	//	UlamValue rtnPtr(getNodeType(), 1, true, EVALRETURN);  //positive to current frame pointer
	//assignReturnValueToStack(rtnPtr);
	evalNodeEpilog();
      }

    return NORMAL;
  }



  void NodeStatements::setNextNode(NodeStatements * s)
  {
    m_nextNode = s;
  }


  void NodeStatements::packBitsInOrderOfDeclaration(u32& offset)
  {
    m_node->packBitsInOrderOfDeclaration(offset); //updates offset

    if(m_nextNode)
      m_nextNode->packBitsInOrderOfDeclaration(offset);
  }


  void NodeStatements::genCode(File * fp, UlamValue& uvpass)
  {
    Locator nodeloc = getNodeLocation();
    m_state.outputTextAsComment(fp, nodeloc);
    m_state.m_locOfNextLineText = nodeloc;  //during gen code here

#ifdef TMPVARBRACES
    m_state.indent(fp);
    fp->write("{\n");    //open for tmpvar arg's
    m_state.m_currentIndentLevel++;
#endif

    m_node->genCode(fp, uvpass);

#ifdef TMPVARBRACES
    m_state.m_currentIndentLevel--;
    m_state.indent(fp);
    fp->write("}\n");  //close for tmpVar
#endif

    if(m_nextNode)
      m_nextNode->genCode(fp, uvpass);
  } //genCode


  void NodeStatements::genCodeToStoreInto(File * fp, UlamValue& uvpass)
  {
    m_node->genCodeToStoreInto(fp, uvpass);

    if(m_nextNode)
      m_nextNode->genCodeToStoreInto(fp, uvpass);
  } //genCodeToStoreInto

} //end MFM
