#include "Scene.h"
#include <any>

namespace std
{
    template <>
    struct hash<std::span<const std::byte>>
    {
        size_t operator()(std::span<const std::byte> value) const noexcept
        {
            return std::hash<decltype(value)::const_pointer> {}(value.data()) ^ std::hash<size_t>{}(value.size_bytes());
        }

    };

    template <>
    struct equal_to<std::span<const std::byte>>
    {
        bool operator ()(std::span<const std::byte> lhv, std::span<const std::byte> rhv) const noexcept
        {
            return lhv.data() == rhv.data() && lhv.size_bytes() == rhv.size_bytes();
        }
    };
}


namespace AT2::Animation
{

enum class InterpolationMode
{
    Step,
    Linear,
    CubicSpline
};

float WrapValue(float x, float x_min, float x_max)
{
    const auto biased_x = x - x_min;
    const auto range = x_max - x_min;

    return x - range * floor(biased_x / range);
}

class ChannelBase
{
public:
    virtual ~ChannelBase() = default;

    virtual void Update(float t, Node& node) = 0;
};

template <typename T, typename F = std::function(void(Node&))>
class Channel : public ChannelBase
{
    std::span<const float> m_time;
    std::span<const T> m_values;

    InterpolationMode m_interpolationMode;
    F m_effector;

public:
    Channel(std::span<const float> timeSpan, std::span<const T> valuesSpan, F&& effector) :
        m_time(timeSpan), m_values(valuesSpan), m_effector(std::forward<F>(effector))
    {
        if (m_time.empty() || m_values.size() < m_time.size() || m_time.front() >= m_time.back())
            throw std::range_error("Animation channel must be initialized with at least two different time values");

        assert(std::is_sorted(m_time.begin(), m_time.end()));
        assert(std::is_sorted(m_values.begin(), m_values.end()));
    }

    const T& GetValueCyclic(float time) const
    {
        return GetValueInternal(false, time);
    }

    const T& GetValue(float time) const
    {
        return GetValueInternal(true, time);
    }

private:
    const T& GetValueInternal(bool clamped, float time) const
    {
        time = clamped ? std::clamp(time, m_time.front(), m_time.back())
                        : WrapValue(time, m_time.front(), m_time.back());

        const auto frame = FindFramePosition(time);
        const auto nextFrame = clamped ? std::max(frame, m_time.size() - 1) : (frame % m_time.size());
        const auto t = (time - m_time[frame]) / (m_time[nextFrame] - m_time[frame]);

        switch (m_interpolationMode)
        {
        case InterpolationMode::Step:
            return m_values[frame];
        case InterpolationMode::Linear: 
            return glm::mix(m_values[frame], m_values[nextFrame], t);
        case InterpolationMode::CubicSpline:
            return glm::mix(m_values[frame], m_values[nextFrame], glm::smoothstep(0.0f, 1.0f, t));
        }

        return {};
    }

    const size_t FindFramePosition(float time) const
    {
        auto pos = std::upper_bound(m_time.begin(), m_time.end(), time);
        if (pos == m_time.end())
            return m_time.size() - 1;

        return std::distance(m_time.begin(), pos);
    }
};

class Animation
{
    std::unordered_map<std::span<const std::byte>, std::any> m_dataSources;
    //std::vector<Channel

public:
    template <typename T, typename F>
    void AddTrack(std::span<T> data, F&& affector)
    {
        const auto key = std::as_bytes(data);
        if (m_dataSources.count(key) == 0) {}
            //m_dataSources.emplace(key, std::vector<T> {data.begin(), data.end()});


    }
};

}