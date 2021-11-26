#include <glm/gtx/spline.hpp>
#include <variant>

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

    //TODO: unit tests
    using Time = float;

    class ChannelBase
    {
    public:
        virtual ~ChannelBase() = default;

        virtual void performUpdate(Scene::Node& node, Time t) const = 0;
    };

    template <typename T, typename ConcreteImplementation, typename F = std::function<void(const T&, Scene::Node&)>>
    class Channel : public ChannelBase
    {
        F m_effector;

    protected:
        std::span<const Time> m_time;
        std::span<const T> m_values;

    public:
        Channel(std::span<const Time> timeSpan, std::span<const T> valuesSpan, F&& effector) :
            m_effector(std::forward<F>(effector)), m_time(timeSpan), m_values(valuesSpan)
        {
            if (m_time.empty() || m_values.size() < m_time.size() || m_time.front() >= m_time.back())
                throw std::range_error("Animation channel must be initialized with at least two different time values");

            assert(std::is_sorted(m_time.begin(), m_time.end()));
        }

        //TODO: remember current frame and time to get rid of searching frame index. Use dt instead of t.
        void performUpdate(Scene::Node& node, Time t) const override
        {
            if (t <= m_time.front() || t > m_time.back())
                return;

            m_effector(static_cast<const ConcreteImplementation*>(this)->getValue(t), node);
        }

    protected:
        [[nodiscard]] size_t findFramePosition(Time time) const
        {
            auto pos = std::upper_bound(m_time.begin(), m_time.end(), time);
            if (pos == m_time.end())
                return m_time.size() - 1;

            return std::distance(m_time.begin(), pos);
        }
        
        //[[nodiscard]] size_t advanceFramePosition(size_t currentPos, Time currentTime, Time newTime) const
        //{ 
        //    assert(actualPos < m_time.size());

        //    if (newTime > currentTime)
        //    {
        //        for (; m_time[currentPos] < newTime && currentPos < m_time.size(); currentPos++);
        //        return std::min(currentPos-1, m_time.size() - 1);
        //    }
        //    else
        //    {
        //        for (; m_time[currentPos] >= newTime && currentPos > 1; currentPos--);
        //        return std::max(currentPos, m_time.size() - 1);
        //    }
        //    
        //    auto comparer = [=](Time t) { return t >= newTime; };
        //    auto iterator = [&] {
        //        auto begin = std::next(m_time.begin(), currentPos);
        //        if (newTime > currentTime)
        //            return std::find_if(begin, m_time.end(), comparer);
        //        return std::find_if_not(m_time.rbegin(), begin, comparer);
        //    }();

        //    //if (dt > 0.0f)
        //    //{
        //    //    return std::distance(m_time.begin(), );
        //    //}
        //}
    };

    template <typename T, typename F>
    class Channel<T, Step, F> : public Channel<T, Channel<T, Step, F>, F>
    {
    public:
        using Channel<T, Channel<T, Step, F>, F>::Channel;
        const T& getValue(Time time) const noexcept
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
        const T& getValue(Time time) const noexcept
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
        const T& getValue(Time time) const noexcept
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
}
