#include <any>
#include "Scene.h"

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

        size_t findFramePosition(float time) const
        {
            auto pos = std::upper_bound(m_time.begin(), m_time.end(), time);
            if (pos == m_time.end())
                return m_time.size() - 1;

            return std::distance(m_time.begin(), pos);
        }
    };

    //
    class Animation
    {
        std::unordered_map<std::span<const std::byte>, std::pair<std::span<const std::byte>, std::any>> m_dataSources;
        std::vector<std::unique_ptr<ChannelBase>> m_channels;

        std::pair<float, float> m_timeRange {0.0f, 0.0f};

    public:
        template <typename T, typename F>
        size_t addTrack(std::span<const float> keySpan, std::span<const T> valueSpan, F&& affector)
        {
            auto getTrustedSpan = [&]<typename T>(std::span<const T> data) -> std::span<const T> {
                const auto key = std::as_bytes(data);
                if (auto it = m_dataSources.find(key); it != m_dataSources.end())
                    return Utils::reinterpret_span_cast<const T>(it->second.first);

                auto buffer = std::vector<T> {data.begin(), data.end()};
                auto span = std::span<const T> {buffer};
                m_dataSources.emplace(key, std::pair {std::as_bytes(span), std::move(buffer)});

                return span;
            };

            m_timeRange = {std::min(m_timeRange.first, keySpan.front()), std::max(m_timeRange.second, keySpan.back())};

            m_channels.emplace_back(std::make_unique<Channel<T, F>>(getTrustedSpan(keySpan), getTrustedSpan(valueSpan), std::forward<F>(affector)));

            return m_channels.size() - 1;
        }

        std::pair<float, float> getTimeRange() const noexcept
        {
            return m_timeRange;
        }

        float getDuration () const noexcept
        {
            return m_timeRange.second - m_timeRange.first;
        }

        const ChannelBase& getTrack(size_t trackIndex) const
        {
            return *m_channels.at(trackIndex);
        }
    };

    using AnimationRef = std::shared_ptr<Animation>;

    class AnimationComponent : public NodeComponent
    {
        AnimationRef m_animation;
        std::vector<size_t> m_trackIndices;

    public:
        AnimationComponent() = default;

        void update(double time) override
        {
            if (!m_animation || !getParent())
                return;

            auto [minTime, maxTime] = m_animation->getTimeRange();

            for (const auto trackIndex : m_trackIndices)
                m_animation->getTrack(trackIndex).performUpdate(*getParent(), wrapValue(time, minTime, maxTime));
        }

        //checks if animation reference are same
        void addTrack(const AnimationRef& animation, size_t trackIndex)
        {
            if (m_animation != animation)
            {
                if (m_animation != nullptr)
                    throw std::logic_error("Animation component could handle just one animation");
                m_animation = animation;
            }

            m_trackIndices.push_back(trackIndex);
        }

    };

} // namespace AT2::Animation