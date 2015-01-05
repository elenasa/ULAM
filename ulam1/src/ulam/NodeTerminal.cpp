#include <stdlib.h>
#include <errno.h>
#include "NodeTerminal.h"
#include "CompilerState.h"

namespace MFM {

  NodeTerminal::NodeTerminal(Token tok, CompilerState & state) : Node(state), m_token(tok) {}

  NodeTerminal::~NodeTerminal(){}


  void NodeTerminal::printPostfix(File * fp)
  {
    fp->write(" ");
    fp->write(getName());
  }


  const char * NodeTerminal::getName()
  {
    if(Token::getSpecialTokenWork(m_token.m_type) == TOKSP_KEYWORD)
      {
	return Token::getTokenAsString(m_token.m_type);  //true or false
      }

    return m_state.getTokenDataAsString(&m_token).c_str();
  }


  const std::string NodeTerminal::prettyNodeName()
  {
    return nodeName(__PRETTY_FUNCTION__);
  }


  UTI NodeTerminal::checkAndLabelType()
  {
    UTI newType = Nav;  //init
    switch(m_token.m_type)
      {
      case TOK_NUMBER_SIGNED:
	newType = m_state.getUlamTypeOfConstant(Int);  	  // a constant Int
	break;
      case TOK_NUMBER_UNSIGNED:
	newType = m_state.getUlamTypeOfConstant(Unsigned); // an Unsigned constant
	break;
      case TOK_KW_TRUE:
      case TOK_KW_FALSE:
	newType = m_state.getUlamTypeOfConstant(Bool);  	  // a constant Bool
	break;
      default:
	{
	  std::ostringstream msg;
	  msg << "Token not a number or boolean: <" << m_token.getTokenString() << ">";
	  MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	}
      };

    if(newType != Nav)
      {
	if(!setConstantValue())
	  newType = Nav;  //error
      }

    setNodeType(newType);
    setStoreIntoAble(false);
    return newType;
  } //checkAndLabelType


  EvalStatus NodeTerminal::eval()
  {
    EvalStatus evs = NORMAL; //init ok
    evalNodeProlog(0); //new current frame pointer

    UlamValue rtnUV;
    evs = makeTerminalValue(rtnUV);

    //copy result UV to stack, -1 relative to current frame pointer
    if(evs == NORMAL)
      assignReturnValueToStack(rtnUV);

    evalNodeEpilog();
    return evs;
  }


  EvalStatus NodeTerminal::makeTerminalValue(UlamValue& uvarg)
  {
    UlamValue rtnUV;         //init to Nav error case
    EvalStatus evs = NORMAL; //init ok

    assert(getNodeType() != Nav);

    switch(m_token.m_type)
      {
      case TOK_NUMBER_SIGNED:
	rtnUV = UlamValue::makeImmediate(getNodeType(), m_constant.sval, m_state);
	break;
      case TOK_NUMBER_UNSIGNED:
	rtnUV = UlamValue::makeImmediate(getNodeType(), m_constant.uval, m_state);
	break;
      case TOK_KW_TRUE:
      case TOK_KW_FALSE:
	rtnUV = UlamValue::makeImmediate(getNodeType(), m_constant.bval, m_state);
	break;
      default:
	{
	  std::ostringstream msg;
	  msg << "Token neither a number, nor a boolean: <" << m_token.getTokenString() << ">";
	  MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), DEBUG);
	  evs = ERROR;
	}
      };
    uvarg = rtnUV;
    return evs;
  } //makeTerminalValue


  //used during check and label for binary arith and compare ops that have a constant term
  bool NodeTerminal::fitsInBits(UTI fituti)
  {
    bool rtnb = false;
    UlamType * fit = m_state.getUlamTypeByIndex(fituti);

    if(fit->getTotalWordSize() != 32)
      {
	std::ostringstream msg;
	msg << "Not supported at this time: <" << m_token.getTokenString() << ">, to fit into type: <" << m_state.getUlamTypeNameByIndex(fituti).c_str() << ">";
	MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
      return false;
      }

    if(getNodeType() == Nav)
      {
	std::ostringstream msg;
	msg << "Token not a number, or a boolean: <" << m_token.getTokenString() << ">, type: <" << m_state.getUlamTypeNameByIndex(getNodeType()).c_str() << ">";
	MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	return false;
      }

    switch(m_token.m_type)
      {
      case TOK_NUMBER_SIGNED:
	{
	  s32 numval = m_constant.sval;
	  if(fit->getUlamTypeEnum() == Int)
	    {
	      rtnb = (numval <= (s32) fit->getMax()) && (numval >= fit->getMin());
	    }
	  else
	    {
	      rtnb = (UABS32(numval) <= fit->getMax()) && (numval >= 0);
	    }
	}
	break;
      case TOK_NUMBER_UNSIGNED:
	{
	  u32 numval = m_constant.uval;
	  rtnb = (numval <= fit->getMax()) && (numval >= 0);
	}
	break;
      case TOK_KW_TRUE:
      case TOK_KW_FALSE:
	rtnb = true;
	break;
      default:
	{
	  std::ostringstream msg;
	  msg << "Token not a number, or a boolean: <" << m_token.getTokenString() << "> , to fit into type: <" << m_state.getUlamTypeNameByIndex(fituti).c_str() << ">";
	  MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	}
      };
    return rtnb;
  } //fitsInBits


  //used during check and label for bitwise shift op that has a negative constant term
  bool NodeTerminal::isNegativeConstant()
  {
    bool rtnb = false;
    if(m_token.m_type == TOK_NUMBER_SIGNED)
      {
	rtnb = (m_constant.sval < 0);
      }
    return rtnb;
  } //isNegativeConstant


  // used during check and label for bitwise shift op that has a constant term gt/ge 32;
  // false is ok.
  bool NodeTerminal::isWordSizeConstant()
  {
    bool rtnb = false;
    if(m_token.m_type == TOK_NUMBER_SIGNED)
      {
	rtnb = (m_constant.sval >= (s32) m_state.getDefaultBitSize(Int));
      }
    else if(m_token.m_type == TOK_NUMBER_UNSIGNED)
      {
	rtnb = (m_constant.uval > m_state.getDefaultBitSize(Unsigned)); // may be ==
      }
    return rtnb;
  } //isWordSizeConstant


  void NodeTerminal::genCode(File * fp, UlamValue& uvpass)
  {
    UlamValue tv;
    assert(makeTerminalValue(tv) == NORMAL);

    // unclear to do this read or not; squarebracket not happy, or cast not happy ?
    genCodeReadIntoATmpVar(fp, tv);  //tv updated to Ptr with a tmpVar "slot"
    uvpass = tv;
    return;
  }


  void NodeTerminal::genCodeToStoreInto(File * fp, UlamValue& uvpass)
  {
    UlamValue tv;
    assert(makeTerminalValue(tv) == NORMAL);
    uvpass = tv;
    return; //uvpass is an immediate UV, not a PTR
  }


  // reads into a tmp var
  // (for BitVector use Node::genCodeConvertATmpVarIntoBitVector)
  void NodeTerminal::genCodeReadIntoATmpVar(File * fp, UlamValue & uvpass)
  {
    // into tmp storage first, in case of casts
    UTI nuti = getNodeType();
    UlamType * nut = m_state.getUlamTypeByIndex(nuti);
    s32	tmpVarNum = m_state.getNextTmpVarNumber();

    m_state.indent(fp);
    fp->write("const ");

    fp->write(nut->getTmpStorageTypeAsString(&m_state).c_str());
    fp->write(" ");

    fp->write(m_state.getTmpVarAsString(nuti, tmpVarNum).c_str());

    fp->write(" = ");

    fp->write(getName());
    fp->write(";\n");

    // substitute Ptr for uvpass to contain the tmpVar number; save id of constant string in Ptr;
    uvpass = UlamValue::makePtr(tmpVarNum, TMPREGISTER, nuti, m_state.determinePackable(nuti), m_state, 0);  //POS 0 rightjustified (atom-based);
    uvpass.setPtrPos(0); //entire register
  } //genCodeReadIntoATmpVar


  bool NodeTerminal::setConstantValue()
  {
    bool rtnok = false;
    switch(m_token.m_type)
      {
      case TOK_NUMBER_SIGNED:
	{
	  std::string numstr = getName();
	  const char * numlist = numstr.c_str();
	  char * nEnd;

	  m_constant.sval = strtol(numlist, &nEnd, 0);   //base 10, 8, or 16
	  if (*numlist == 0 || *nEnd != 0)
	    {
	      std::ostringstream msg;
	      msg << "Invalid signed constant: <" << numstr.c_str() << ">, errno=" << errno << " " << strerror(errno);
	      MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	    }
	  else
	    rtnok = true;
	}
	break;
      case TOK_NUMBER_UNSIGNED:
	{
	  std::string numstr = getName();
	  const char * numlist = numstr.c_str();
	  char * nEnd;

	  m_constant.uval = strtoul(numlist, &nEnd, 0);   //base 10, 8, or 16
	  if (*numlist == 0 || !(*nEnd == 'u' || *nEnd == 'U') || *(nEnd + 1) != 0)
	    {
	      std::ostringstream msg;
	      msg << "Invalid unsigned constant: <" << numstr.c_str() << ">, errno=" << errno << " " << strerror(errno);
	      MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	    }
	  else
	    rtnok = true;
	}
	break;
      case TOK_KW_TRUE:
	m_constant.bval = true;
	rtnok = true;
	break;
      case TOK_KW_FALSE:
	m_constant.bval = false;
	rtnok = true;
	break;
      default:
	{
	    std::ostringstream msg;
	    msg << "Token neither a number, nor a boolean: <" << m_token.getTokenString() << ">";
	    MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), DEBUG);
	}
      };
    return rtnok;
  } //setConstantValue

} //end MFM
