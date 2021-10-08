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
    desc.m_uses = 0;
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
    assert(okToParseABreak());

    u32 rtn = 0; //zero uses c++ break
    LoopSwitchDesc desc = m_stack.back();
    if(desc.m_token.m_type == TOK_KW_SWITCH)
      {
	rtn = desc.m_tmpvarexitnum;
	m_stack.back().m_uses++; //t41016,etc..
      }
    return rtn;
  } //getNearestBreakExitNumber

  u32 ParsingLoopsSwitchStack::getNearestBreakExitNumberIfUsed()
  {
    assert(okToParseABreak());

    u32 rtn = 0; //zero uses c++ break
    LoopSwitchDesc desc = m_stack.back();
    if(desc.m_token.m_type == TOK_KW_SWITCH)
      {
	if(desc.m_uses > 0)
	  rtn = desc.m_tmpvarexitnum; //for label
      }
    return rtn;
  } //getNearestBreakExitNumberIfUsed

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
	    rit->m_uses++;
	    break;
	  }
	//else skip switches
      }
    return rtn;
  } //getNearestContinueExitNumber

  u32 ParsingLoopsSwitchStack::getNearestContinueExitNumberIfUsed()
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
	    if(rit->m_uses > 0)
	      rtn = desc.m_tmpvarexitnum;
	    break;
	  }
	//else skip switches
      }
    return rtn;
  } //getNearestContinueExitNumberIfUsed

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
