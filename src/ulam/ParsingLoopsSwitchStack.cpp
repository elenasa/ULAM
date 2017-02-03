#include <assert.h>
#include "ParsingLoopsSwitchStack.h"

namespace MFM {

  ParsingLoopsSwitchStack::ParsingLoopsSwitchStack(){}

  ParsingLoopsSwitchStack::~ParsingLoopsSwitchStack()
  {
    m_stack.clear();
  }

  void ParsingLoopsSwitchStack::push(Token & tok, u32 exitnum)
  {
    assert(exitnum > 0);
    assert((tok.m_type == TOK_KW_WHILE) || (tok.m_type == TOK_KW_FOR) || (tok.m_type == TOK_KW_SWITCH));

    struct LoopSwitchDesc desc;
    desc.m_token = tok;
    desc.m_tmpvarexitnum = exitnum;
    m_stack.push_back(desc);
  }

  u32 ParsingLoopsSwitchStack::getLastExitNumber()
  {
    if(m_stack.size() == 0)
      return 0; //not in a loop or switch

    return m_stack.back().m_tmpvarexitnum; //last is nearest!
  }

  u32 ParsingLoopsSwitchStack::getNearestBreakExitNumber()
  {
    return getLastExitNumber();
  }

  u32 ParsingLoopsSwitchStack::getNearestContinueExitNumber()
  {
    if(m_stack.size() == 0)
      return 0; //not in a loop or switch

    u32 rtn = 0;
    std::vector<LoopSwitchDesc>::reverse_iterator rit;
    for(rit = m_stack.rbegin(); rit != m_stack.rend(); rit++)
      {
	LoopSwitchDesc desc = *rit;
	if((desc.m_token.m_type == TOK_KW_WHILE) || (desc.m_token.m_type == TOK_KW_FOR))
	  {
	    rtn = desc.m_tmpvarexitnum;
	    break;
	  }
	//else skip switches
      }
    return rtn;
  } //getNearestContinueExitNumber

  bool ParsingLoopsSwitchStack::okToParseAContinue()
  {
    return (getNearestContinueExitNumber() != 0);
  }

  bool ParsingLoopsSwitchStack::okToParseABreak()
  {
    if(m_stack.size() == 0)
      return false; //not in a loop or switch
    return true; //loop or switch ok
  }

  void ParsingLoopsSwitchStack::pop()
  {
    assert(m_stack.size() > 0);
    m_stack.pop_back();
  }

} //MFM
