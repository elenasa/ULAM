#include <stdlib.h>
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

    return m_state.getDataAsString(&m_token).c_str();
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
      case TOK_NUMBER:
	{
	  std::string numstr = getName();
	  std::size_t found = numstr.find('.');
	  if ( found != std::string::npos)
	    {
	      std::ostringstream msg;
	      msg << "Float not supported: <" << numstr << ">, type is Nav";
	      MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	      newType = Nav;
	    }
	  else
	    {
	      newType = m_state.getUlamTypeOfConstant(Int);  	  // a constant Int
	    }
	}
	break;

      case TOK_KW_TRUE:
      case TOK_KW_FALSE:
	{
	  newType = m_state.getUlamTypeOfConstant(Bool);  	  // a constant Bool
	}
	break;
      default:
	{
	  std::ostringstream msg;
	  msg << "Token not a number or boolean: <" << m_token.getTokenString() << ">";
	  MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	}
      };

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
    assignReturnValueToStack(rtnUV);

    evalNodeEpilog();
    return evs;
  }


  EvalStatus NodeTerminal::makeTerminalValue(UlamValue& uvarg)
  {
    UlamValue rtnUV;         //init to Nav error case
    EvalStatus evs = NORMAL; //init ok

    switch(m_token.m_type)
      {
      case TOK_NUMBER:
	{
	  std::string numstr = getName();
	  const char * numlist = numstr.c_str();
	  char * nEnd;

	  UTI typidx = getNodeType();
	  ULAMTYPE typEnum = m_state.getUlamTypeByIndex(typidx)->getUlamTypeEnum();
	  if (typEnum == Int)
	    {
	      s32 numval = strtol(numlist, &nEnd, 10);   //base 10
	      rtnUV = UlamValue::makeImmediate(typidx, numval, m_state);
	      break;
	    }

	    std::ostringstream msg;
	    msg << "Token not a number: <" << m_token.getTokenString() << ">";
	    MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), DEBUG);
	}
	break;
      case TOK_KW_TRUE:
	rtnUV = UlamValue::makeImmediate(getNodeType(), true, m_state);
	break;
      case TOK_KW_FALSE:
	rtnUV = UlamValue::makeImmediate(getNodeType(), false, m_state);
	break;
      default:
	{
	    std::ostringstream msg;
	    msg << "Token not a number, or a boolean: <" << m_token.getTokenString() << ">";
	    MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), DEBUG);
	    evs = ERROR;
	}
      };

    uvarg = rtnUV;
    return evs;
  } //makeTerminalValue


  bool NodeTerminal::fitsInBits(UTI fituti)
  {
    bool rtnb = false;
    UlamType * fit = m_state.getUlamTypeByIndex(fituti);
    ULAMTYPE fitEnum = fit->getUlamTypeEnum();
    u32 fitbitsize = m_state.getBitSize(fituti);

    assert(fit->getTotalWordSize() == 32);

    switch(m_token.m_type)
      {
      case TOK_NUMBER:
	{
	  std::string numstr = getName();
	  const char * numlist = numstr.c_str();
	  char * nEnd;

	  UTI typidx = getNodeType();
	  ULAMTYPE typEnum = m_state.getUlamTypeByIndex(typidx)->getUlamTypeEnum();
	  if (typEnum == Int)
	    {
	      s32 numval = strtol(numlist, &nEnd, 10);   //base 10
	      if(fitEnum == Int)
		{
		  if(fitbitsize == m_state.getDefaultBitSize(Int))
		    rtnb = (numval <= S32_MAX) && (numval >= S32_MIN);
		  else
		    rtnb = (numval <= ((1 << (fitbitsize - 1)) - 1)) && (numval >= (1 << fitbitsize));
		}
	      else
		{
		  // unsigned
		  if(fitbitsize == m_state.getDefaultBitSize(Unsigned))
		    rtnb = ((u32) numval <= U32_MAX) && ((u32) numval >= U32_MIN);
		  else
		    rtnb = (numval <= ((1 << fitbitsize) - 1)) && (numval >= 0);
		}
	      break;
	    }

	    std::ostringstream msg;
	    msg << "Token not a number: <" << m_token.getTokenString() << ">";
	    MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), DEBUG);
	}
	break;
      case TOK_KW_TRUE:
      case TOK_KW_FALSE:
	rtnb = true;
	break;
      default:
	{
	    std::ostringstream msg;
	    msg << "Token not a number, or a boolean: <" << m_token.getTokenString() << ">";
	    MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), DEBUG);
	}
      };

    return rtnb;
  } //fitsInBits


  bool NodeTerminal::isNegativeConstant()
  {
    bool rtnb = false;

    if(m_token.m_type == TOK_NUMBER)
      {
	std::string numstr = getName();
	const char * numlist = numstr.c_str();
	char * nEnd;

	UTI typidx = getNodeType();
	ULAMTYPE typEnum = m_state.getUlamTypeByIndex(typidx)->getUlamTypeEnum();
	if (typEnum == Int)
	  {
	    s32 numval = strtol(numlist, &nEnd, 10);   //base 10
	    rtnb = (numval < 0);
	  }
      }
    return rtnb;
  } //isNegativeConstant


  bool NodeTerminal::isWordSizeConstant()
  {
    bool rtnb = false;
    if(m_token.m_type == TOK_NUMBER)
      {
	std::string numstr = getName();
	const char * numlist = numstr.c_str();
	char * nEnd;

	UTI typidx = getNodeType();
	ULAMTYPE typEnum = m_state.getUlamTypeByIndex(typidx)->getUlamTypeEnum();
	if (typEnum == Int)
	  {
	    s32 numval = strtol(numlist, &nEnd, 10);   //base 10
	    rtnb = (numval >= (s32) m_state.getDefaultBitSize(Int));
	  }
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

} //end MFM
