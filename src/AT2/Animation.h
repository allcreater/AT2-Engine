#pragma once

#include "Scene.h"

#include <any>
#include <algorithm>

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
    enum class InterpolationMode
    {
        Step,
        Linear,
        CubicSpline
    };

    inline float wrapValue(float x, float x_min, float x_max)
    {
        const auto biased_x = x - x_min;
        const auto range = x_max - x_min;

        return x - range * floor(biased_x / range);
    }


    class ChannelBase
    {
        InterpolationMode m_interpolationMode = InterpolationMode::Linear;

    public:
        virtual ~ChannelBase() = default;

        InterpolationMode getInterpolationMode() const noexcept { return m_interpolationMode; }
        void setInterpolationMode(InterpolationMode mode) noexcept { m_interpolationMode = mode; }

        virtual void performUpdate(Node& node, float t) const = 0;
    };

    template <typename T, typename F = std::function(void(const T&, Node&))>
    class Channel : public ChannelBase
    {
        std::span<const float> m_time;
        std::span<const T> m_values;

        F m_effector;

    public:
        Channel(std::span<const float> timeSpan, std::span<const T> valuesSpan, F&& effector) :
            m_time(timeSpan), m_values(valuesSpan), m_effector(std::forward<F>(effector))
        {
            if (m_time.empty() || m_values.size() < m_time.size() || m_time.front() >= m_time.back())
                throw std::range_error("Animation channel must be initialized with at least two different time values");

            assert(std::is_sorted(m_time.begin(), m_time.end()));
        }

        //TODO: remember current frame and time to get rid of searching frame index. Use dt instead of t.
        void performUpdate(Node& node, float t) const override
        {
            if (t <= m_time.front() || t > m_time.back())
                return;

            m_effector(getValue(t), node);
        }

    private:
        const T& getValue(float time) const noexcept
        {
            const auto nextFrame = findFramePosition(time);
            const auto frame = (nextFrame > 0) ? nextFrame - 1 : 0;
            const auto t = (nextFrame > frame ) ? (time - m_time[frame]) / (m_time[nextFrame] - m_time[frame]) : 0.0f;

            switch (getInterpolationMode())
            {
            case InterpolationMode::Step: return m_values[frame];
            case InterpolationMode::Linear: return glm::mix(m_values[frame], m_values[nextFrame], t);
            case InterpolationMode::CubicSpline:
                return glm::mix(m_values[frame], m_values[nextFrame], glm::smoothstep(0.0f, 1.0f, t));
            }

            return {};
        }

        [[nodiscard]] size_t findFramePosition(float time) const
        {
            auto pos = std::upper_bound(m_time.begin(), m_time.end(), time);
            if (pos == m_time.end())
                return m_time.size() - 1;

            return std::distance(m_time.begin(), pos);
        }
    };

    using AnimationNodeId = size_t;


    class Animation;


    class AnimationCollection
    {
        std::unordered_map<std::span<const std::byte>, std::pair<std::span<const std::byte>, std::any>> m_dataSources;
        std::vector<Animation> m_animations;

        Animation* m_activeAnimation = nullptr;

        friend class Animation;

    public:
        bool setCurrentAnimation(size_t animationIndex);

        Animation& addAnimation(std::string name);
        const std::vector<Animation>& getAnimationsList() const noexcept { return m_animations; }

        void updateNode(AnimationNodeId nodeId, Node& nodeInstance, double time); //TODO: dt!
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

            auto& newChannel = m_channels.emplace_back(std::make_unique<Channel<T, F>>(getTrustedSpan(keySpan), getTrustedSpan(valueSpan), std::forward<F>(affector)));
            newChannel->setInterpolationMode(interpolation);

            m_timeRange = {std::min(m_timeRange.first, keySpan.front()), std::max(m_timeRange.second, keySpan.back())};
            m_channelsByNode.emplace(animationNodeId, newChannel.get());

            return m_channels.size() - 1;
        }

        void updateNode(AnimationNodeId nodeId, Node& nodeInstance, double time)
        {
            auto [rangeBegin, rangeEnd] = m_channelsByNode.equal_range(nodeId);
            for (auto it = rangeBegin; it != rangeEnd; ++it)
                it->second->performUpdate(nodeInstance, wrapValue(time, m_timeRange.first, m_timeRange.second));
        }

        [[nodiscard]] std::pair<float, float> getTimeRange() const noexcept { return m_timeRange; }

        [[nodiscard]] float getDuration() const noexcept { return m_timeRange.second - m_timeRange.first; }

        [[nodiscard]] const ChannelBase& getTrack(size_t trackIndex) const;
    };

    using AnimationRef = std::shared_ptr<AnimationCollection>;

    class AnimationComponent : public NodeComponent
    {
        AnimationRef m_animation;
        //std::vector<size_t> m_trackIndices;
        AnimationNodeId m_animationNodeId;

    public:
        AnimationComponent(AnimationRef animation, AnimationNodeId nodeId) : m_animation(std::move(animation)), m_animationNodeId(nodeId) {}

        void update(double time) override
        {
            if (!m_animation || !getParent())
                return;

            m_animation->updateNode(m_animationNodeId, *getParent(), time);
        }
    };

} // namespace AT2::Animation