#include <stdio.h>
#include "NodeBlockLocals.h"
#include "CompilerState.h"

namespace MFM {

  NodeBlockLocals::NodeBlockLocals(NodeBlock * prevBlockNode, CompilerState & state): NodeBlockContext(prevBlockNode, state), m_registryNumberLocalsSafe(UNINITTED_REGISTRY_NUMBER) {}

  NodeBlockLocals::NodeBlockLocals(const NodeBlockLocals& ref) : NodeBlockContext(ref), m_registryNumberLocalsSafe(UNINITTED_REGISTRY_NUMBER) {}

  NodeBlockLocals::~NodeBlockLocals() {}

  Node * NodeBlockLocals::instantiate()
  {
    return new NodeBlockLocals(*this);
  }

  void NodeBlockLocals::updateLineage(NNO pno)
  {
    assert(getPreviousBlockPointer() == NULL);

    setYourParentNo(pno);
    //has no m_node
    if(m_nodeNext)
      m_nodeNext->updateLineage(getNodeNo());
  } //updateLineage

  void NodeBlockLocals::printPostfix(File * fp)
  {
    fp->write(" locals: ");
    if(m_nodeNext)
      NodeBlock::printPostfix(fp);
  }

  const char * NodeBlockLocals::getName()
  {
    return "localdefs";
  }

  const std::string NodeBlockLocals::prettyNodeName()
  {
    return nodeName(__PRETTY_FUNCTION__);
  }

  bool NodeBlockLocals::isAClassBlock()
  {
    return false;
  }

  UTI NodeBlockLocals::checkAndLabelType(Node * thisparentnode)
  {
    //possibly empty (e.g. error/t3875)
    return NodeBlockContext::checkAndLabelType(thisparentnode);
  }

  void NodeBlockLocals::calcMaxDepth(u32& depth, u32& maxdepth, s32 base)
  {
    return;  //overrides NodeBlock
  }

  void NodeBlockLocals::genCode(File * fp, UVPass& uvpass)
  {
    //noop, neither named constants nor typedefs require generated C++ code.
    m_state.abortShouldntGetHere();
  }

  void NodeBlockLocals::cloneAndAppendNode(std::vector<Node *> & cloneVec)
  {
    if(m_nodeNext)
      m_nodeNext->cloneAndAppendNode(cloneVec); //work done by NodeConstantDef, NodeTypedef
  }

  void NodeBlockLocals::addTargetDescriptionToInfoMap(TargetMap& classtargets, u32 scid)
  {
    UTI cuti = getNodeType();
    u32 classNameId = m_state.getClassNameIdForUlamLocalsFilescope(cuti); //cut->getUlamTypeNameOnly();
    std::string className = m_state.m_pool.getDataAsString(classNameId);
    u32 mangledNameId = m_state.getMangledClassNameIdForUlamLocalsFilescope(cuti); //cut->getUlamTypeMangledName();
    std::string mangledName = m_state.m_pool.getDataAsString(mangledNameId);

    struct TargetDesc desc;

    desc.m_hasTest = false;
    desc.m_classType = UC_LOCALSFILESCOPE;

    desc.m_bitsize = 0;
    desc.m_loc = getNodeLocation();
    desc.m_className = className;
    desc.m_structuredComment = "NONE";

    classtargets.insert(std::pair<std::string, struct TargetDesc>(mangledName, desc));
  } //addTargetDescriptionToInfoMap

  void NodeBlockLocals::addMemberDescriptionsToInfoMap(ClassMemberMap& classmembers)
  {
    NodeBlock::addMemberDescriptionsToInfoMap(classmembers); //Table of Variables request
  }

  void NodeBlockLocals::generateTestInstance(File * fp, bool runtest)
  {
    UTI suti = getNodeType();

    fp->write("\n");
    m_state.indent(fp);
    fp->write("{\n");

    m_state.m_currentIndentLevel++;

    m_state.indent(fp);
    fp->write("UlamClass<EC> & clt = ");
    fp->write(m_state.getLocalsFilescopeTheInstanceMangledNameByIndex(suti).c_str());
    fp->write(";"); GCNL;

    m_state.indent(fp);
    fp->write("tile.GetUlamClassRegistry().RegisterUlamClass(clt);"); GCNL;

    m_state.m_currentIndentLevel--;

    m_state.indent(fp);
    fp->write("}\n");
    return;
  } //generateTestInstance

  void NodeBlockLocals::generateIncludeTestMain(File * fp)
  {
    UTI locuti = getNodeType();
    u32 mangledclassid = m_state.getMangledClassNameIdForUlamLocalsFilescope(locuti);

    m_state.indent(fp);
    fp->write("#include \"");
    fp->write(m_state.m_pool.getDataAsString(mangledclassid).c_str());
    fp->write(".h\""); GCNL;
    return;
  } //generateIncludeTestMain

  bool NodeBlockLocals::assignRegistrationNumberToLocalsBlock(u32 n)
  {
    if (n == UNINITTED_REGISTRY_NUMBER)
      {
	std::ostringstream msg;
	msg << "Attempting to assign invalid Registry Number to Locals FileScope";
	MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	return false;
      }

    if (m_registryNumberLocalsSafe != UNINITTED_REGISTRY_NUMBER)
      {
	std::ostringstream msg;
	msg << "Attempting to assign duplicate Registry Number " << n;
	msg << " to Locals FileScope " << m_registryNumberLocalsSafe;
	MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	return false;
      }

    m_registryNumberLocalsSafe = n;
    return true;
  } //assignRegistrationNumberToLocalsBlock

  u32 NodeBlockLocals::getRegistrationNumberForLocalsBlock()
  {
    if(m_registryNumberLocalsSafe == UNINITTED_REGISTRY_NUMBER)
      {
	UTI luti = getNodeType();
	if(m_state.okUTItoContinue(luti))// && m_state.isComplete(luti))
	  {
	    u32 n = m_state.assignClassId(luti);
	    AssertBool rnset = assignRegistrationNumberToLocalsBlock(n);
	    assert(rnset);
	  }
      }
    return m_registryNumberLocalsSafe;
  }

} //end MFM
