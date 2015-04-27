#include <stdlib.h>
#include "NodeTypedef.h"
#include "CompilerState.h"


namespace MFM {

  NodeTypedef::NodeTypedef(SymbolTypedef * sym, NodeTypeDescriptor * nodetype, CompilerState & state) : Node(state), m_typedefSymbol(sym), m_tdid(0), m_currBlockNo(0), m_nodeTypeDesc(nodetype)
  {
    if(sym)
      {
	m_tdid = sym->getId();
	m_currBlockNo = sym->getBlockNoOfST();
      }
  }

  NodeTypedef::NodeTypedef(const NodeTypedef& ref) : Node(ref), m_typedefSymbol(NULL), m_tdid(ref.m_tdid), m_currBlockNo(ref.m_currBlockNo), m_nodeTypeDesc(NULL)
  {
    if(ref.m_nodeTypeDesc)
      m_nodeTypeDesc = (NodeTypeDescriptor *) ref.m_nodeTypeDesc->instantiate();
  }

  NodeTypedef::~NodeTypedef()
  {
    delete m_nodeTypeDesc;
    m_nodeTypeDesc = NULL;
  }

  Node * NodeTypedef::instantiate()
  {
    return new NodeTypedef(*this);
  }

  void NodeTypedef::updateLineage(NNO pno)
  {
    Node::updateLineage(pno);
    if(m_nodeTypeDesc)
      m_nodeTypeDesc->updateLineage(getNodeNo());
  }//updateLineage

  bool NodeTypedef::findNodeNo(NNO n, Node *& foundNode)
  {
    if(Node::findNodeNo(n, foundNode))
      return true;
    if(m_nodeTypeDesc && m_nodeTypeDesc->findNodeNo(n, foundNode))
      return true;
    return false;
  } //findNodeNo

  void NodeTypedef::printPostfix(File * fp)
  {
    fp->write(" typedef");

    fp->write(" ");
    fp->write(m_state.getUlamTypeNameBriefByIndex(m_typedefSymbol->getUlamTypeIdx()).c_str());
    fp->write(" ");
    fp->write(getName());

    s32 arraysize = m_state.getArraySize(m_typedefSymbol->getUlamTypeIdx());
    if(arraysize > NONARRAYSIZE)
      {
	fp->write("[");
	fp->write_decimal(arraysize);
	fp->write("]");
      }
    else if(arraysize == UNKNOWNSIZE)
      {
	fp->write("[UNKNOWN]");
      }

    fp->write("; ");
  } //printPostfix

  const char * NodeTypedef::getName()
  {
    //same as m_typedefSymbol->getUlamType()->getUlamTypeNameBrief()); //short type name
    return m_state.m_pool.getDataAsString(m_typedefSymbol->getId()).c_str();
  }

  const std::string NodeTypedef::prettyNodeName()
  {
    return nodeName(__PRETTY_FUNCTION__);
  }

  bool NodeTypedef::getSymbolPtr(Symbol *& symptrref)
  {
    symptrref = m_typedefSymbol;
    return true;
  } //getSymbolPtr

  UTI NodeTypedef::checkAndLabelType()
  {
    UTI it = Nav;

    // instantiate, look up in current block
    if(m_typedefSymbol == NULL)
      {
	//in case of a cloned unknown
	NodeBlock * currBlock = getBlock();
	m_state.pushCurrentBlockAndDontUseMemberBlock(currBlock);

	Symbol * asymptr = NULL;
	if(m_state.alreadyDefinedSymbol(m_tdid, asymptr))
	  {
	    if(asymptr->isTypedef())
	      {
		m_typedefSymbol = (SymbolTypedef *) asymptr;
	      }
	    else
	      {
		std::ostringstream msg;
		msg << "(1) <" << m_state.m_pool.getDataAsString(m_tdid).c_str();
		msg << "> is not a typedef, and cannot be used as one";
		MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	      }
	  }
	else
	  {
	    std::ostringstream msg;
	    msg << "(2) Typedef <" << m_state.m_pool.getDataAsString(m_tdid).c_str();
	    msg << "> is not defined, and cannot be used";
	    MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	  }
	m_state.popClassContext(); //restore
      } //toinstantiate

    if(m_typedefSymbol)
      {
	it = m_typedefSymbol->getUlamTypeIdx();

	//check for incomplete Classes (e.g. array of UC_QUARK)
	UlamType * tdut = m_state.getUlamTypeByIndex(it);
	ULAMCLASSTYPE tdclasstype = tdut->getUlamClass();
	if(tdclasstype == UC_UNSEEN)
	  {
	    if(!m_state.completeIncompleteClassSymbol(it))
	      {
		std::ostringstream msg;
		msg << "Incomplete Typedef for class type: ";
		msg << m_state.getUlamTypeNameByIndex(it).c_str();
		msg << " used with variable symbol name <" << getName();
		msg << "> (UTI" << it << ")";
		MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), DEBUG);
	      }
	  }

	UTI cuti = m_state.getCompileThisIdx();
	if(m_nodeTypeDesc)
	  {
	    UTI duti = m_nodeTypeDesc->checkAndLabelType();
	    //assert(duti == Nav || duti == it);
	    if(duti != Nav && duti != it)
	      {
		std::ostringstream msg;
		msg << "REPLACING Symbol UTI" << it;
		msg << ", " << m_state.getUlamTypeNameBriefByIndex(it).c_str();
		msg << " used with typedef symbol name '" << getName();
		msg << "' with node type descriptor type: ";
		msg << m_state.getUlamTypeNameBriefByIndex(duti).c_str();
		msg << " UTI" << duti << " while labeling class: ";
		msg << m_state.getUlamTypeNameBriefByIndex(cuti).c_str();
		MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), DEBUG);
		m_typedefSymbol->resetUlamType(duti); //consistent!
		m_state.mapTypesInCurrentClass(it, duti);
		it = duti;
	      }
	  }

#if 0
	// i belive this is done by node type descriptor
	if(!m_state.isComplete(it))
	  {
	    UTI mappedUTI = Nav;
	    if(m_state.mappedIncompleteUTI(cuti, it, mappedUTI))
	      {
		std::ostringstream msg;
		msg << "Substituting Mapped UTI" << mappedUTI;
		msg << ", " << m_state.getUlamTypeNameByIndex(mappedUTI).c_str();
		msg << " for incomplete Typedef type: ";
		msg << m_state.getUlamTypeNameBriefByIndex(it).c_str();
		msg << " used with typedef symbol name '" << getName();
		msg << "' UTI" << it << " while labeling class: ";
		msg << m_state.getUlamTypeNameBriefByIndex(cuti).c_str();
		MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), DEBUG);
		it = mappedUTI;
	      }
	  }
#endif

	if(!m_state.isComplete(it)) //reloads
	  {
	    std::ostringstream msg;
	    msg << "Incomplete Typedef for type: ";
	    msg << m_state.getUlamTypeNameByIndex(it).c_str();
	    msg << " used with typedef symbol name '" << getName();
	    msg << "' UTI" << it << " while labeling class: ";
	    msg << m_state.getUlamTypeNameBriefByIndex(cuti).c_str();
	    MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), WARN);
	    //it = Nav; unlike vardecl
	  }
	//else
	//  m_typedefSymbol->resetUlamType(it); //consistent!
	//  } //not complete
      } // got typedef symbol

    setNodeType(it);
    return getNodeType();
  } //checkAndLabelType

  NNO NodeTypedef::getBlockNo()
  {
    return m_currBlockNo;
  }

  NodeBlock * NodeTypedef::getBlock()
  {
    assert(m_currBlockNo);
    NodeBlock * currBlock = (NodeBlock *) m_state.findNodeNoInThisClass(m_currBlockNo);
    assert(currBlock);
    return currBlock;
  } //getBlock

  void NodeTypedef::packBitsInOrderOfDeclaration(u32& offset)
  {
    //do nothing, but override
  }

  void NodeTypedef::countNavNodes(u32& cnt)
  {
    Node::countNavNodes(cnt);
    if(m_nodeTypeDesc)
      m_nodeTypeDesc->countNavNodes(cnt);
  } //countNavNodes

  EvalStatus NodeTypedef::eval()
  {
    assert(m_typedefSymbol);
    return NORMAL;
  } //eval

  void NodeTypedef::genCode(File * fp, UlamValue& uvpass)
  {
#if 0
    m_state.indent(fp);
    fp->write("typedef ");

    fp->write(m_state.getUlamTypeByIndex(m_typedefSymbol->getUlamTypeIdx())->getUlamTypeMangledName().c_str()); //for C++
    fp->write(" ");
    fp->write(getName());

    s32 arraysize = m_state.getArraySize(m_typedefSymbol->getUlamTypeIdx());
    if(arraysize > NONARRAYSIZE)
      {
	fp->write("[");
	fp->write_decimal(arraysize);
	fp->write("]");
      }
    else if(arraysize == UNKNOWNSIZE)
      {
	fp->write("[UNKNOWN]");
      }

    fp->write(";\n");
#endif
  } //genCode

} //end MFM
