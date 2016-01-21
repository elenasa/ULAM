#include <stdlib.h>
#include "NodeTypeDescriptorSelect.h"
#include "CompilerState.h"


namespace MFM {

  NodeTypeDescriptorSelect::NodeTypeDescriptorSelect(Token tokarg, UTI auti, NodeTypeDescriptor * node, CompilerState & state) : NodeTypeDescriptor(tokarg, auti, state), m_nodeSelect(node)
  {
    if(m_nodeSelect)
      m_nodeSelect->updateLineage(getNodeNo()); //for unknown subtrees
  }

  NodeTypeDescriptorSelect::NodeTypeDescriptorSelect(const NodeTypeDescriptorSelect& ref) : NodeTypeDescriptor(ref), m_nodeSelect(NULL)
  {
    if(ref.m_nodeSelect)
      m_nodeSelect = (NodeTypeDescriptor *) ref.m_nodeSelect->instantiate();
  }

  NodeTypeDescriptorSelect::~NodeTypeDescriptorSelect()
  {
    delete m_nodeSelect;
    m_nodeSelect = NULL;
  } //destructor

  Node * NodeTypeDescriptorSelect::instantiate()
  {
    return new NodeTypeDescriptorSelect(*this);
  }

  void NodeTypeDescriptorSelect::updateLineage(NNO pno)
  {
    setYourParentNo(pno);
    if(m_nodeSelect)
      m_nodeSelect->updateLineage(getNodeNo());
  } //updateLineage

  bool NodeTypeDescriptorSelect::findNodeNo(NNO n, Node *& foundNode)
  {
    if(Node::findNodeNo(n, foundNode))
      return true;
    if(m_nodeSelect && m_nodeSelect->findNodeNo(n, foundNode))
      return true;
    return false;
  } //findNodeNo

  void NodeTypeDescriptorSelect::printPostfix(File * fp)
  {
    fp->write(getName());
  }

  const char * NodeTypeDescriptorSelect::getName()
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

  const std::string NodeTypeDescriptorSelect::prettyNodeName()
  {
    return nodeName(__PRETTY_FUNCTION__);
  }

  UTI NodeTypeDescriptorSelect::checkAndLabelType()
  {
    UTI it = getNodeType();
    if(isReadyType())
      return it;

    if(resolveType(it))
      {
	m_ready = true; // set here
      }
    else
	m_state.setGoAgain();

    setNodeType(it);
    return getNodeType();
  } //checkAndLabelType

  bool NodeTypeDescriptorSelect::resolveType(UTI& rtnuti)
  {
    bool rtnb = false;
    if(isReadyType())
      {
	rtnuti = getNodeType();
	return true;
      }

    // we are in a "chain" of type selects..
    assert(m_nodeSelect);

    UTI seluti = m_nodeSelect->checkAndLabelType();
    if(m_nodeSelect->isReadyType())
      {
	UlamType * selut = m_state.getUlamTypeByIndex(seluti);
	ULAMTYPE seletype = selut->getUlamTypeEnum();
	if(seletype == Class)
	  {
	    SymbolClass * csym = NULL;
	    AssertBool isDefined = m_state.alreadyDefinedSymbolClass(seluti, csym);
	    assert(isDefined);

	    m_state.pushClassContext(seluti, csym->getClassBlockNode(), csym->getClassBlockNode(), false, NULL);
	    // find our id in the "selected" class, must be a typedef at top level
	    Symbol * asymptr = NULL;
	    bool hazyKin = false;
	    if(m_state.alreadyDefinedSymbol(m_typeTok.m_dataindex, asymptr, hazyKin) && !hazyKin)
	      {
		if(asymptr->isTypedef())
		  {
		    UTI auti = asymptr->getUlamTypeIdx();
		    if(m_state.isComplete(auti))
		      {
			rtnuti = auti; //should be mapped already, if necessary
			rtnb = true;
		      }
		    else if(m_state.isHolder(auti))
		      {
			UTI mappedUTI;
			if(m_state.mappedIncompleteUTI(seluti, auti, mappedUTI))
			  {
			    std::ostringstream msg;
			    msg << "Substituting Mapped UTI" << mappedUTI << ", ";
			    msg << m_state.getUlamTypeNameBriefByIndex(mappedUTI).c_str();
			    msg << " for incomplete descriptor type: ";
			    msg << m_state.getUlamTypeNameBriefByIndex(auti).c_str();
			    msg << "' UTI" << auti << " while labeling class: ";
			    msg << m_state.getUlamTypeNameBriefByIndex(seluti).c_str();
			    MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), DEBUG);
			    auti = mappedUTI;
			  }
			else
			  rtnuti = Hzy;
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
		    rtnuti = Nav; //?
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
		if(!hazyKin)
		  {
		    MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR); //new
		    rtnuti = Nav;
		  }
		else
		  {
		    MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), DEBUG);
		    rtnuti = Hzy;
		  }
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
	    rtnuti = Nav;
	  }
      }
    else
      rtnuti = Hzy; //else select not ready, so neither are we!!
    return rtnb;
  } //resolveType

  void NodeTypeDescriptorSelect::countNavHzyNoutiNodes(u32& ncnt, u32& hcnt, u32& nocnt)
  {
    NodeTypeDescriptor::countNavHzyNoutiNodes(ncnt, hcnt, nocnt);
    m_nodeSelect->countNavHzyNoutiNodes(ncnt, hcnt, nocnt);
  }

} //end MFM
