#pragma once

#include "Scene.h"

#include <any>
#include <algorithm>
#include <glm/gtx/spline.hpp>

namespace std
{
    template <typename T>
    struct hash<std::span<const T>>
    {
        size_t operator()(std::span<const T> value) const noexcept
        {
            return std::hash<decltype(value)::const_pointer> {}(value.data()) ^
                std::hash<size_t> {}(value.size_bytes());
        }
    };

    template <typename T>
    struct equal_to<std::span<const T>>
    {
        bool operator()(std::span<const T> lhv, std::span<const T> rhv) const noexcept
        {
            return lhv.data() == rhv.data() && lhv.size_bytes() == rhv.size_bytes();
        }
    };
} // namespace std


namespace AT2::Animation
{

    struct Step {};
    struct Linear {};
    struct CubicSpline{};
    using InterpolationMode = std::variant<Step, Linear, CubicSpline>;

    inline float wrapValue(float x, float x_min, float x_max)
    {
        const auto biased_x = x - x_min;
        const auto range = x_max - x_min;

        return x - range * floor(biased_x / range);
    }


    class ChannelBase
    {
    public:
        virtual ~ChannelBase() = default;

        virtual void performUpdate(Scene::Node& node, float t) const = 0;
    };


    template <typename T, typename ConcreteImplementation, typename F = std::function<void(const T&, Scene::Node&)>>
    class Channel : public ChannelBase
    {
        F m_effector;

    protected:
        std::span<const float> m_time;
        std::span<const T> m_values;

    public:
        Channel(std::span<const float> timeSpan, std::span<const T> valuesSpan, F&& effector) :
            m_effector(std::forward<F>(effector)), m_time(timeSpan), m_values(valuesSpan)
        {
            if (m_time.empty() || m_values.size() < m_time.size() || m_time.front() >= m_time.back())
                throw std::range_error("Animation channel must be initialized with at least two different time values");

            assert(std::is_sorted(m_time.begin(), m_time.end()));
        }

        //TODO: remember current frame and time to get rid of searching frame index. Use dt instead of t.
        void performUpdate(Scene::Node& node, float t) const override
        {
            if (t <= m_time.front() || t > m_time.back())
                return;

            m_effector(static_cast<const ConcreteImplementation*>(this)->getValue(t), node);
        }

    protected:
        [[nodiscard]] size_t findFramePosition(float time) const
        {
            auto pos = std::upper_bound(m_time.begin(), m_time.end(), time);
            if (pos == m_time.end())
                return m_time.size() - 1;

            return std::distance(m_time.begin(), pos);
        }
    };

    template <typename T, typename F>
    class Channel<T, Step, F> : public Channel<T, Channel<T, Step, F>, F>
    {
    public:
        using Channel<T, Channel<T, Step, F>, F>::Channel;
        const T& getValue(float time) const noexcept
        {
            const auto nextFrame = this->findFramePosition(time);
            return this->m_values[(nextFrame > 0) ? nextFrame - 1 : 0];
        }
    };

    template <typename T, typename F>
    class Channel<T, Linear, F> : public Channel<T, Channel<T, Linear, F>, F>
    {
    public:
        using Channel<T, Channel<T, Linear, F>, F>::Channel;
        const T& getValue(float time) const noexcept
        {
            const auto nextFrame = this->findFramePosition(time);
            const auto frame = (nextFrame > 0) ? nextFrame - 1 : 0;
            const auto t = (nextFrame > frame)
                ? (time - this->m_time[frame]) / (this->m_time[nextFrame] - this->m_time[frame])
                : 0.0f;

            return glm::mix(this->m_values[frame], this->m_values[nextFrame], t);
        }
    };

    template <typename T, typename F>
    class Channel<T, CubicSpline, F> : public Channel<T, Channel<T, CubicSpline, F>, F>
    {
    public:
        using Channel<T, Channel<T, CubicSpline, F>, F>::Channel;
        const T& getValue(float time) const noexcept
        {
            const auto nextFrame = this->findFramePosition(time);
            const auto frame = (nextFrame > 0) ? nextFrame - 1 : 0;
            const auto frameLatency = this->m_time[nextFrame] - this->m_time[frame];
            const auto t = (nextFrame > frame) ? (time - this->m_time[frame]) / frameLatency : 0.0f;

            //m_values[k] are tuple of [in-tangent Ak(0), point Vk (+1), out-tangent Bk (+2)].
            const auto p0 = this->m_values[frame * 3 + 1];
            const auto m0 = frameLatency * this->m_values[frame * 3 + 2];
            const auto p1 = this->m_values[nextFrame * 3 + 1];
            const auto m1 = frameLatency * this->m_values[nextFrame * 3];

            return glm::hermite(p0, m0, p1, m1, t);
        }
    };


    enum class AnimationNodeId : uint32_t
    {
        None = -1
    };


    class Animation;
    class AnimationCollection
    {
        std::unordered_map<std::span<const std::byte>, std::pair<std::span<const std::byte>, std::any>> m_dataSources;
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

    class BoneComponent : public Scene::NodeComponent
    {
        size_t m_boneIndex;
        Scene::MeshComponent::SkeletonInstanceRef m_skeletonInstance;

    public:
        BoneComponent(size_t boneIndex, Scene::MeshComponent::SkeletonInstanceRef skeletonInstanceRef) :
            m_boneIndex(boneIndex), m_skeletonInstance(std::move(skeletonInstanceRef))
        {
        }

        void update(Scene::UpdateVisitor& updateVisitor) override
        {
            m_skeletonInstance->calculateBoneTransform(m_boneIndex, updateVisitor.getTransformsStack().getModelView());
        }
    };

} // namespace AT2::Animation