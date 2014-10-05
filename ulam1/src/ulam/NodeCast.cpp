#include <stdio.h>
#include "NodeCast.h"
#include "CompilerState.h"

namespace MFM {

  NodeCast::NodeCast(Node * n, UTI typeToBe, CompilerState & state): NodeUnaryOp(n, state) 
  {
    setNodeType(typeToBe);
  }

  NodeCast::~NodeCast()
  { 
    delete m_node;
    m_node = NULL;
  }


  const char * NodeCast::getName()
  {
    return "cast";
  }


  const std::string NodeCast::prettyNodeName()
  {
    return nodeName(__PRETTY_FUNCTION__);
  }


  UTI NodeCast::checkAndLabelType()
  { 
    // unlike the other nodes, nodecast knows its type at construction time;
    // this is for checking for errors, before eval happens.
    u32 errorsFound = 0;
    UTI tobeType = getNodeType();
    UTI nodeType = m_node->checkAndLabelType(); //user cast 
    ULAMCLASSTYPE tobeClass = m_state.getUlamTypeByIndex(tobeType)->getUlamClass();

    if(tobeType == Nav || tobeClass != UC_NOTACLASS)
      {
	std::ostringstream msg;
	msg << "Cannot cast to type <" << m_state.getUlamTypeNameByIndex(tobeType).c_str() << ">";
	MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	errorsFound++;
      }
    else 
      {
	if(!m_state.isScalar(tobeType))
	  {
	    MSG(getNodeLocationAsString().c_str(), "Consider implementing array casts, elena", DEBUG);

	    if(!m_state.isScalar(nodeType))
	      MSG(getNodeLocationAsString().c_str(), "Consider implementing array casts: Cannot cast scalar into array", ERR);

	    if(m_state.getArraySize(tobeType) != m_state.getArraySize(nodeType))
	      MSG(getNodeLocationAsString().c_str(), "Consider implementing array casts: Array sizes differ", ERR);
	    errorsFound++;
	  }
	else
	  {
	    if(!m_state.isScalar(nodeType))
	      {
		MSG(getNodeLocationAsString().c_str(), "Consider implementing array casts: Cannot cast array into scalar", ERR);
		errorsFound++;
	      }
	  }
      }

    // special case: user casting a quark to an Int; 
    if(errorsFound == 0)  
      {
	ULAMCLASSTYPE nodeClass = m_state.getUlamTypeByIndex(nodeType)->getUlamClass();
	if(nodeClass == UC_QUARK)
	  {
	    ULAMTYPE tobeTypEnum = m_state.getUlamTypeByIndex(tobeType)->getUlamTypeEnum();
	    if(tobeTypEnum != Int)
	      {
		std::ostringstream msg;
		msg << "Cannot cast quark type <" << m_state.getUlamTypeNameByIndex(nodeType).c_str() << "> to non-Int <" << m_state.getUlamTypeNameByIndex(tobeType).c_str() << ">";
		MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
		errorsFound++;		//error!
	      }
	    else
	      {
		// update to NodeMemberSelect + NodeFunctionCall
		m_node = makeCastingNode(m_node, tobeType);
	      }
	  }
	else
	  {
	    if(nodeClass == UC_ELEMENT || nodeClass == UC_INCOMPLETE)
	      {
		std::ostringstream msg;
		msg << "Cannot cast type <" << m_state.getUlamTypeNameByIndex(nodeType).c_str() << "> to <" << m_state.getUlamTypeNameByIndex(tobeType).c_str() << ">";
		MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
		errorsFound++;	    //error!
	      }
	  }
      }
    
    return getNodeType(); 
  }


  EvalStatus NodeCast::eval()
  {
    assert(m_node); //has to be

    evalNodeProlog(0); //new current frame pointer
    UTI tobeType = getNodeType();
    UTI nodeType = m_node->getNodeType(); //uv.getUlamValueType()

    makeRoomForNodeType(nodeType);
    EvalStatus evs = m_node->eval();
    if(evs != NORMAL)
      {
	evalNodeEpilog();
	return evs;
      }

    //do we believe these to be scalars, only?
    //possibly an array that needs to be casted, per elenemt
    if(m_state.isScalar(tobeType))
      assert(m_state.isScalar(nodeType));
    else
      {
	//both arrays the same dimensions
	//assert(!nodeType->isScalar());
	if(m_state.getArraySize(tobeType) != m_state.getArraySize(nodeType))
	  MSG(getNodeLocationAsString().c_str(), "Considering implementing array casts!!!", ERR);
	assert(0);
      }

    UlamValue uv = m_state.m_nodeEvalStack.loadUlamValueFromSlot(1);

    if(nodeType != tobeType)
      {
	if(!(m_state.getUlamTypeByIndex(tobeType)->cast(uv, m_state)))
	  {
	    std::ostringstream msg;
	    msg << "Cast problem! Value type <" << m_state.getUlamTypeNameByIndex(uv.getUlamValueTypeIdx()).c_str() << "> failed to be cast as type: <" << m_state.getUlamTypeNameByIndex(tobeType).c_str() << ">";
	    MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), DEBUG);
	  }
      }

    //also copy result UV to stack, -1 relative to current frame pointer
    assignReturnValueToStack(uv);

    evalNodeEpilog();
    return NORMAL;
  }


  UlamValue NodeCast::makeImmediateUnaryOp(UTI type, u32 data, u32 len)
  {
    assert(0); // n/a
    return UlamValue();
  }


  void NodeCast::genCode(File * fp)
  {
    if(needsACast())
      {
	//	fp->write("(");
	//fp->write(m_state.getUlamTypeByIndex(getNodeType())->getUlamTypeImmediateMangledName(&m_state).c_str());
	//fp->write(") ");
	//fp->write("(");
	//m_node->genCode(fp);
	//fp->write(")");
	
	//std::string tmpVar = m_node->genCodeReadIntoATmpVar(fp);
	genCodeReadIntoATmpVar(fp);
      }
    else
	m_node->genCode(fp);
  }


  std::string NodeCast::genCodeReadIntoATmpVar(File * fp)
  {
    std::string tmpVar = m_node->genCodeReadIntoATmpVar(fp);   //just the terminal

    if(!needsACast())
      return tmpVar;


    UTI nuti = getNodeType();
    UTI nodetype = m_node->getNodeType();
    UlamType * nut = m_state.getUlamTypeByIndex(nuti);

    m_state.indent(fp);
    
    std::ostringstream tmpVarCast;
    tmpVarCast << tmpVar << "_1" ;

    fp->write(nut->getImmediateTypeAsString(&m_state).c_str()); //e.g. u32, s32, u64, etc.
    fp->write(" ");
    fp->write(tmpVarCast.str().c_str());
    fp->write(" = ");

    // write the cast method (e.g. _SignExtend32, _S32ToUnary, etc..)
    fp->write(nut->castMethodForCodeGen(nodetype, m_state).c_str());
    fp->write("(");

    fp->write(tmpVar.c_str());
    fp->write(", ");
    //LENGTH of node being casted (UH_AP_mi::LENGTH ?)
    //fp->write(m_state.getBitVectorLengthAsStringForCodeGen(nodetype).c_str());    
    s32 nodeLength = m_state.getTotalBitSize(nodetype);
    fp->write_decimal(nodeLength);

    fp->write(")");
    fp->write(";\n");

    return tmpVarCast.str();
  } //genCodeReadIntoTmp


  void NodeCast::genCodeWriteFromATmpVar(File * fp, std::string tmpVar)
  {
    assert(0);
    m_node->genCodeWriteFromATmpVar(fp, tmpVar);
  }    


  bool NodeCast::needsACast()
  {
    return true;  //debug

    UTI tobeType = getNodeType();
    UTI nodeType = m_node->getNodeType();
    ULAMTYPE typEnum = m_state.getUlamTypeByIndex(tobeType)->getUlamTypeEnum();
    ULAMTYPE nodetypEnum = m_state.getUlamTypeByIndex(nodeType)->getUlamTypeEnum();
    s32 arraysize = m_state.getArraySize(tobeType);
    s32 nodearraysize = m_state.getArraySize(nodeType);

    //cast if the base types are different OR the arraysizes differ (i.e. one's scalar, not constant)
    return(typEnum != nodetypEnum || (arraysize != nodearraysize && !m_state.isConstant(nodeType)));
    //return(typEnum != nodetypEnum || (arraysize != nodearraysize) || m_state.isConstant(nodeType));
  }

} //end MFM
