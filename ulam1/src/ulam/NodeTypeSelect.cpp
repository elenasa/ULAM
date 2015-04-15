#include <stdlib.h>
#include "NodeTypeSelect.h"
#include "CompilerState.h"


namespace MFM {

  NodeTypeSelect::NodeTypeSelect(Token typetoken, UTI auti, NodeType * node, CompilerState & state) : NodeType(typetoken, auti, state), m_nodeSelect(node)
  {
    if(m_nodeSelect)
      m_nodeSelect->updateLineage(getNodeNo()); //for unknown subtrees
  }

  NodeTypeSelect::NodeTypeSelect(const NodeTypeSelect& ref) : NodeType(ref), m_nodeSelect(NULL)
  {
    if(ref.m_nodeSelect)
      m_nodeSelect = (NodeType *) ref.m_nodeSelect->instantiate();
  }

  NodeTypeSelect::~NodeTypeSelect()
  {
    delete m_nodeSelect;
    m_nodeSelect = NULL;
  } //destructor

  Node * NodeTypeSelect::instantiate()
  {
    return new NodeTypeSelect(*this);
  }

  void NodeTypeSelect::updateLineage(NNO pno)
  {
    setYourParentNo(pno);
    if(m_nodeSelect)
      m_nodeSelect->updateLineage(getNodeNo());
  }//updateLineage

  bool NodeTypeSelect::findNodeNo(NNO n, Node *& foundNode)
  {
    if(Node::findNodeNo(n, foundNode))
      return true;
    if(m_nodeSelect && m_nodeSelect->findNodeNo(n, foundNode))
      return true;
    return false;
  } //findNodeNo

  void NodeTypeSelect::printPostfix(File * fp)
  {
    fp->write(getName());
  }

  const char * NodeTypeSelect::getName()
  {
    std::ostringstream nstr;
    if(m_nodeSelect)
      {
	nstr << m_nodeSelect->getName();
	nstr << ".";
      }
    nstr << m_state.getTokenDataAsString(&m_typeTok);
    return nstr.str().c_str();
  } //getName

  const std::string NodeTypeSelect::prettyNodeName()
  {
    return nodeName(__PRETTY_FUNCTION__);
  }

  UTI NodeTypeSelect::checkAndLabelType()
  {
    if(isReadyType())
      return getNodeType();

    UTI it = Nav;
    //    it = m_nodeSelect->checkAndLabelType();
    if(resolveType(it))
      {
	m_ready = true; // set here
      }

    setNodeType(it);
    return getNodeType();
  } //checkAndLabelType


  bool NodeTypeSelect::resolveType(UTI& rtnuti)
  {
    bool rtnb = false;
    if(isReadyType())
      {
	rtnuti = getNodeType();
	return true;
      }

    // we are in a "chain" of type selects..
    assert(m_nodeSelect);

    UTI seluti;
    if(m_nodeSelect->resolveType(seluti))
      {
	UlamType * selut = m_state.getUlamTypeByIndex(seluti);
	ULAMTYPE seletype = selut->getUlamTypeEnum();
	if(seletype == Class)
	  {
	    SymbolClass * csym = NULL;
	    assert(m_state.alreadyDefinedSymbolClass(seluti, csym));

	    m_state.pushClassContext(seluti, csym->getClassBlockNode(), csym->getClassBlockNode(), false, NULL);
	    // find our id in the "selected" class, must be a typedef at top level
	    Symbol * asymptr = NULL;
	    if(m_state.alreadyDefinedSymbol(m_typeTok.m_dataindex, asymptr))
	      {
		if(asymptr->isTypedef())
		  {
		    UTI auti = asymptr->getUlamTypeIdx();
		    if(m_state.isComplete(auti))
		      {
			rtnuti = auti; //should be mapped already, if necessary
			rtnb = true;
		      }
		  }
		else
		  {
		    //error id is not a typedef
		    std::ostringstream msg;
		    msg << "Not a typedef <" << m_state.getTokenDataAsString(&m_typeTok).c_str();
		    msg << "> in another class, " ;;
		    msg << m_state.getUlamTypeNameBriefByIndex(seluti).c_str();
		    msg <<" while compiling: ";
		    msg << m_state.getUlamTypeNameBriefByIndex(m_state.getCompileThisIdx()).c_str();
		    MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), WARN);
		  }
	      }
	    else
	      {
		//error! id not found
		std::ostringstream msg;
		msg << "Undefined Typedef <" << m_state.getTokenDataAsString(&m_typeTok).c_str();
		msg << "> in another class, " ;;
		msg << m_state.getUlamTypeNameBriefByIndex(seluti).c_str();
		msg <<" while compiling: ";
		msg << m_state.getUlamTypeNameBriefByIndex(m_state.getCompileThisIdx()).c_str();
		MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), DEBUG);
	      }

	    m_state.popClassContext();
	  }
	else
	  {
	    //error has to be a class
	    std::ostringstream msg;
	    msg << "Type selected by <" << m_state.getTokenDataAsString(&m_typeTok).c_str();
	    msg << "> is NOT another class, " ;
	    msg << m_state.getUlamTypeNameBriefByIndex(seluti).c_str();
	    msg << ", rather a " << UlamType::getUlamTypeEnumAsString(seletype) << " type,";
	    msg <<" while compiling: ";
	    msg << m_state.getUlamTypeNameBriefByIndex(m_state.getCompileThisIdx()).c_str();
	    MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	  }
      } //else select not ready, so neither are we!!
    return rtnb;
  } //resolveType

  void NodeTypeSelect::countNavNodes(u32& cnt)
  {
    Node::countNavNodes(cnt);
    m_nodeSelect->countNavNodes(cnt);
  }

} //end MFM
