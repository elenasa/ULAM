#include <assert.h>
#include "ClassContextStack.h"

namespace MFM {

  ClassContextStack::ClassContextStack(){ }

  ClassContextStack::~ClassContextStack()
  {
    m_contexts.clear();
  }

  bool ClassContextStack::getCurrentClassContext(ClassContext & contextref) //no change to stack
  {
    if(m_contexts.empty())
      return false;
    contextref = m_contexts.back();
    return true;
  }

  void ClassContextStack::pushClassContext(ClassContext context)
  {
    m_contexts.push_back(context);
  }

  void ClassContextStack::popClassContext()
  {
    assert(!m_contexts.empty());
    m_contexts.pop_back();
  }

  bool ClassContextStack::popClassContext(ClassContext & contextref)
  {
    if(m_contexts.empty()) return false;
    getCurrentClassContext(contextref);
    m_contexts.pop_back();
    return true;
  }

  u32 ClassContextStack::getClassContextStackSize()
  {
    return m_contexts.size();
  }

  u32 ClassContextStack::countClassContextOnStack(const NodeBlockClass * cblock)
  {
    u32 count = 0;
    std::vector<ClassContext>::iterator it;

    for(it = m_contexts.end()--; it >= m_contexts.begin(); it--)
      {
	ClassContext cc = *it;
	if(cc.getContextBlock() == cblock)
	  count++;
      }
    return count;
  }
} //MFM
