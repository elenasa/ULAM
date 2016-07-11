#include "ClassContext.h"

namespace MFM {

  ClassContext::ClassContext() : m_compileThisId(0), m_compileThisIdx(Nouti), m_currentBlock(NULL), m_classBlock(NULL), m_useMemberBlock(false), m_currentMemberClassBlock(NULL) {}

  ClassContext::ClassContext(u32 id, UTI idx, NodeBlock * nb, NodeBlockClass * nbc, bool usememberblock, NodeBlockClass * mbc): m_compileThisId(id), m_compileThisIdx(idx), m_currentBlock(nb), m_classBlock(nbc), m_useMemberBlock(usememberblock), m_currentMemberClassBlock(mbc) {}

  ClassContext::~ClassContext() {}

  u32 ClassContext::getCompileThisId()
  {
    return m_compileThisId;
  }

  UTI ClassContext::getCompileThisIdx()
  {
    return m_compileThisIdx;
  }

  void ClassContext::setCompileThisIdx(u32 id, UTI idx)
  {
    // keep in sync
    m_compileThisId = id;
    m_compileThisIdx = idx;
  }

  NodeBlock * ClassContext::getCurrentBlock()
  {
    return m_currentBlock;
  }

  void ClassContext::setCurrentBlock(NodeBlock * nb)
  {
    assert(nb);
    m_currentBlock = nb;
  }

  NodeBlockClass * ClassContext::getClassBlock()
  {
    return m_classBlock;
  }

  void ClassContext::setClassBlock(NodeBlockClass * nbc)
  {
    assert(nbc);
    m_classBlock = nbc;
  }

  bool ClassContext::useMemberBlock()
  {
    return m_useMemberBlock;
  }

  void ClassContext::useMemberBlock(bool use)
  {
    m_useMemberBlock = use;
  }

  NodeBlockClass * ClassContext::getCurrentMemberClassBlock()
  {
    return m_currentMemberClassBlock;
  }

  void ClassContext::setCurrentMemberClassBlock(NodeBlockClass * nbc)
  {
    m_currentMemberClassBlock = nbc; //could be null
  }

  std::string ClassContext::getClassContextAsString()
  {
    std::ostringstream str;
    str << m_compileThisId << ", " << m_compileThisIdx << ", <" << m_currentBlock << "> , <" << m_classBlock << ">, " << (m_useMemberBlock ? "true" : "false") << ", <" << m_currentMemberClassBlock << ">";
    return str.str();
  }

} //end MFM
