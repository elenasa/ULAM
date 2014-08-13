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


  UlamType * NodeTerminal::checkAndLabelType()
  {
    UlamType * newType = m_state.getUlamTypeByIndex(Nav);  //Nav
    ULAMTYPE numType = Nav;

    switch(m_token.m_type)
      {
      case TOK_NUMBER:
	{
	  numType = Int;
	  std::string numstr = getName();
	  std::size_t found = numstr.find('.');
	  if ( found != std::string::npos)
	    {
	      if( numstr.find('.', found+1) == std::string::npos)   //needs work...
		{
		  numType = Float;
		}
	      else
		{
		  std::ostringstream msg;
		  msg << "Found 2 dots in number: <" << numstr << ">, type is Nav";
		  MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), DEBUG);
		  numType = Nav;
		}
	    } 
	}
	break;
	
      case TOK_KW_TRUE:
      case TOK_KW_FALSE:
	numType = Bool;
	break;
      default:
	{
	  std::ostringstream msg;
	  msg << "Token not a number or boolean: <" << m_token.getTokenString() << ">";
	  MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), DEBUG);
	}
      };
    
    newType = m_state.getUlamTypeByIndex(numType);
    setNodeType(newType);
    
    setStoreIntoAble(false);
    return newType;
  }


  EvalStatus NodeTerminal::eval()
  {
    EvalStatus evs = NORMAL; //init ok 
    UlamValue rtnUV(m_state.getUlamTypeByIndex(Nav), 0, IMMEDIATE); //init to error case

    evalNodeProlog(0); //new current frame pointer

    switch(m_token.m_type)
      {
      case TOK_NUMBER:
	{
	  std::string numstr = getName();
	  const char * numlist = numstr.c_str();
	  char * nEnd;
	  
	  UlamType * nut = getNodeType();
	  UTI typidx = m_state.getUlamTypeIndex(nut);
	  
	  if (typidx == Float)
	    {
	      float numFloat = strtof(numlist,&nEnd);
	      rtnUV.init(m_state.getUlamTypeByIndex(Float), numFloat);
	      break;
	    }
	  
	  if (typidx == Int)
	    {
	      s32 numval = strtol(numlist, &nEnd, 10);   //base 10
	      rtnUV.init(m_state.getUlamTypeByIndex(Int), numval); 
	      break;
	    }
 
	    std::ostringstream msg;
	    msg << "Token not a number: <" << m_token.getTokenString() << ">";
	    MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), DEBUG);
	}
	break;
      case TOK_KW_TRUE:
	rtnUV.init(m_state.getUlamTypeByIndex(Bool), true);
	break;
      case TOK_KW_FALSE:
	rtnUV.init(m_state.getUlamTypeByIndex(Bool), false);
	break;
      default:
	{
	    std::ostringstream msg;
	    msg << "Token not a number, or a boolean: <" << m_token.getTokenString() << ">";
	    MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), DEBUG);
	    evs = ERROR;
	}
      };

    //copy result UV to stack, -1 relative to current frame pointer
    assignReturnValueToStack(rtnUV);

    evalNodeEpilog();
    return evs;
  }
  

} //end MFM
