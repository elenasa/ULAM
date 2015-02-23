#include <assert.h>
#include "ClassContextStack.h"

namespace MFM {

  ClassContextStack::ClassContextStack(){ }

  ClassContextStack::~ClassContextStack()
  {
    m_contexts.clear();
  }

  void ClassContextStack::getCurrentClassContext(ClassContext & contextref) //no change to stack
  {
    if(m_contexts.empty())
      return;
    contextref = m_contexts.back();
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

} //MFM
