#include "Animation.h"
#include <ranges>

//using namespace AT2;
using namespace AT2::Scene;
using namespace AT2::Animation;

bool AnimationCollection::setCurrentAnimation(size_t animationIndex)
{
    if (animationIndex >= m_animations.size())
        return false;

    m_activeAnimation = &m_animations[animationIndex];
    return true;
}

Animation& AnimationCollection::addAnimation(std::string name)
{
    return m_animations.emplace_back(*this, std::move(name));
}

void AnimationCollection::updateNode(AnimationNodeId nodeId, Node& nodeInstance, const ITime& time)
{
    if (!m_activeAnimation)
        return;
    
    m_activeAnimation->updateNode(nodeId, nodeInstance, time.getTime().count());
}


void Animation::updateNode(AnimationNodeId nodeId, Node& nodeInstance, double time)
{
    auto [rangeBegin, rangeEnd] = m_channelsByNode.equal_range(nodeId);
    for (auto it = rangeBegin; it != rangeEnd; ++it)
        it->second->performUpdate(nodeInstance, wrapValue(time, m_timeRange.first, m_timeRange.second));
}

const ChannelBase& Animation::getTrack(size_t trackIndex) const
{
    return *m_channels.at(trackIndex);
}

void AnimationComponent::update(UpdateVisitor& updateVisitor)
{
    if (!m_animation || !getParent())
        return;

    m_animation->updateNode(getNodeId(), *getParent(), updateVisitor.getTime());
}
