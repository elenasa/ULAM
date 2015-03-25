#include <stdlib.h>
#include "NodeTypedef.h"
#include "CompilerState.h"


namespace MFM {

  NodeTypedef::NodeTypedef(SymbolTypedef * sym, CompilerState & state) : Node(state), m_typedefSymbol(sym), m_tdid(0), m_currBlockNo(0)
  {
    if(sym)
      {
	m_tdid = sym->getId();
	m_currBlockNo = sym->getBlockNoOfST();
      }
  }

  NodeTypedef::NodeTypedef(const NodeTypedef& ref) : Node(ref), m_typedefSymbol(NULL), m_tdid(ref.m_tdid), m_currBlockNo(ref.m_currBlockNo) {}

  NodeTypedef::~NodeTypedef() {}

  Node * NodeTypedef::instantiate()
  {
    return new NodeTypedef(*this);
  }

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
	//check for incomplete Classes
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

	if(!tdut->isComplete())
	  {
	    UTI cuti = m_state.getCompileThisIdx();
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
		m_typedefSymbol->resetUlamType(mappedUTI); //consistent!
	      }
	  }

	if(!m_state.isComplete(it)) //reloads
	  {
	    std::ostringstream msg;
	    msg << "Incomplete Typedef for type: ";
	    msg << m_state.getUlamTypeNameByIndex(it).c_str();
	    msg << " used with typedef symbol name '" << getName();
	    msg << "' (UTI" << it << ")";
	    MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), WARN);
	  }
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

  EvalStatus NodeTypedef::eval()
  {
    assert(m_typedefSymbol);
    return NORMAL;
  } //eval

  bool NodeTypedef::getSymbolPtr(Symbol *& symptrref)
  {
    symptrref = m_typedefSymbol;
    return true;
  } //getSymbolPtr

  void NodeTypedef::packBitsInOrderOfDeclaration(u32& offset)
  {
    //do nothing, but override
  }

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
    fp->write(";\n");
#endif
  } //genCode

} //end MFM
