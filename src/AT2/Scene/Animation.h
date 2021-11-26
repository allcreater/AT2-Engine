#pragma once

#include "Scene.h"
#include "Channel.h"

#include <any>
#include <algorithm>
#include <glm/gtx/spline.hpp>
#include <unordered_map>

namespace AT2::Animation
{
    enum class AnimationNodeId : uint32_t
    {
        None = -1
    };


    class Animation;

    class AnimationCollection
    {
    private:
        using span_hash = decltype([]<typename T>(std::span<const T> value) {
            return std::hash<decltype(value)::const_pointer> {}(value.data()) ^ std::hash<size_t> {}(value.size_bytes());
        });

        using span_equal = decltype([]<typename T>(std::span<const T> lhv, std::span<const T> rhv) {
            return lhv.data() == rhv.data() && lhv.size_bytes() == rhv.size_bytes();
        });

    private:
        std::unordered_map<std::span<const std::byte>, std::pair<std::span<const std::byte>, std::any>, span_hash, span_equal> m_dataSources;
        std::vector<Animation> m_animations;

        Animation* m_activeAnimation = nullptr;

        friend class Animation;

    public:
        bool setCurrentAnimation(size_t animationIndex);
        const Animation* getCurrentAnimation() const noexcept { return m_activeAnimation; }

        Animation& addAnimation(std::string name);
        const std::vector<Animation>& getAnimationsList() const noexcept { return m_animations; }

        void updateNode(AnimationNodeId nodeId, Scene::Node& nodeInstance, const ITime& time);
    };

    // Инкапсулирует набор действий, который нужно совершить со сценой, чтобы она анимировалась
    class Animation
    {
        AnimationCollection& m_sourceCollection;

        std::string m_name;
        std::vector<std::unique_ptr<ChannelBase>> m_channels;
        std::unordered_multimap<AnimationNodeId, ChannelBase*> m_channelsByNode;

        std::pair<float, float> m_timeRange {0.0f, 0.0f};

    public:
        Animation(AnimationCollection& sourceCollection, std::string name) //make private?
            :
            m_sourceCollection(sourceCollection),
            m_name(std::move(name))
        {
        }

        Animation(Animation&&) noexcept = default;


        template <typename T, typename F>
        size_t addTrack(AnimationNodeId animationNodeId, std::span<const float> keySpan, std::span<const T> valueSpan,
                        F&& affector, InterpolationMode interpolation)
        {
            auto getTrustedSpan = [&dataSources = m_sourceCollection.m_dataSources]<typename T>(
                                      std::span<const T> data) -> std::span<const T> {
                const auto key = std::as_bytes(data);
                if (auto it = dataSources.find(key); it != dataSources.end())
                    return Utils::reinterpret_span_cast<const T>(it->second.first);

                auto buffer = std::vector<T> {data.begin(), data.end()};
                auto span = std::span<const T> {buffer};
                dataSources.emplace(key, std::pair {std::as_bytes(span), std::move(buffer)});

                return span;
            };

            auto& newChannel = m_channels.emplace_back(std::visit(
                [&]<typename Impl>(Impl) -> std::unique_ptr<ChannelBase> {
                    return std::make_unique<Channel<T, Impl, F>>(getTrustedSpan(keySpan), getTrustedSpan(valueSpan),
                                                                 std::forward<F>(affector));
                },
                interpolation));

            m_timeRange = {std::min(m_timeRange.first, keySpan.front()), std::max(m_timeRange.second, keySpan.back())};
            m_channelsByNode.emplace(animationNodeId, newChannel.get());

            return m_channels.size() - 1;
        }

        void updateNode(AnimationNodeId nodeId, Scene::Node& nodeInstance, double time);
        [[nodiscard]] std::pair<float, float> getTimeRange() const noexcept { return m_timeRange; }
        [[nodiscard]] float getDuration() const noexcept { return m_timeRange.second - m_timeRange.first; }
        [[nodiscard]] const ChannelBase& getTrack(size_t trackIndex) const;
    };

    using AnimationRef = std::shared_ptr<AnimationCollection>;

    class NodeIdComponent : public Scene::NodeComponent
    {
        AnimationNodeId m_nodeId;

    public:
        NodeIdComponent(AnimationNodeId nodeId) : m_nodeId(nodeId) {}
        AnimationNodeId getNodeId() const noexcept { return m_nodeId; }

        void update(Scene::UpdateVisitor& updateVisitor) override {}
    };

    //TODO: should be similar to skinning infrastucture, but need to decide good way
    class AnimationComponent : public NodeIdComponent
    {
        AnimationRef m_animation;

    public:
        AnimationComponent(AnimationRef animation, AnimationNodeId nodeId) : m_animation(std::move(animation)), NodeIdComponent(nodeId) {}

        bool isSameAs(const AnimationRef& animation, AnimationNodeId nodeId) const noexcept
        {
            return m_animation == animation && getNodeId() == nodeId;
        }

        const AnimationRef& getAnimation() const noexcept { return m_animation; }

        void update(Scene::UpdateVisitor& updateVisitor) override;
    };

} // namespace AT2::Animation