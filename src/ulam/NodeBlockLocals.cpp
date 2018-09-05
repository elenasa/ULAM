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

  UTI NodeBlockLocals::checkAndLabelType()
  {
    UTI savnuti = getNodeType();
    assert(savnuti != Nouti);

    //possibly empty (e.g. error/t3875)
    if(m_nodeNext)
      NodeBlock::checkAndLabelType();
    setNodeType(savnuti);
    return savnuti;
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
  } //assignRegistryNumberToLocalsBlock

  u32 NodeBlockLocals::getRegistrationNumberForLocalsBlock() const
  {
    assert(m_registryNumberLocalsSafe != UNINITTED_REGISTRY_NUMBER);
    return m_registryNumberLocalsSafe;
  }

} //end MFM
