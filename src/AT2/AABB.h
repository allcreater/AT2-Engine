#ifndef AT2_AAABB_HEADER
#define AT2_AAABB_HEADER

#include <glm/glm.hpp>
#include <limits>

struct AABB2d
{
	glm::vec2 MinBound;
	glm::vec2 MaxBound;

public:
	AABB2d(const glm::vec2& min, const glm::vec2& max) noexcept: MinBound(min), MaxBound(max) {}
	AABB2d () = default;
	AABB2d (const AABB2d&) = default;

	bool operator ==(const AABB2d& other) const noexcept
	{
		return (MinBound == other.MinBound) && (MaxBound == other.MaxBound);
	}

	void Reset(bool invalidate = false) noexcept
	{
		if (invalidate)
		{
			MinBound.x = MinBound.y = std::numeric_limits<float>().max();
			MaxBound.x = MaxBound.y = std::numeric_limits<float>().min();
		}
		else
		{
			MinBound.x = 0.0f;
			MinBound.y = 0.0f;
			MaxBound.x = 0.0f;
			MaxBound.y = 0.0f;
		}
	}

	//TODO: add tests to special cases such as NaN and Infinity!!!
	bool Valid() const noexcept
	{
		return MaxBound.x >= MinBound.x && MaxBound.y >= MinBound.y;
	}

	bool Empty() const noexcept
	{
		return MaxBound.x == MinBound.x && MaxBound.y == MinBound.y;
	}

	bool IsPointInside(const glm::vec2& point) const noexcept
	{
		return (point.x >= MinBound.x && point.y >= MinBound.y && point.x <= MaxBound.x && point.y <= MaxBound.y);
	}

	float GetWidth() const noexcept
	{
		return MaxBound.x - MinBound.x;
	}

	float GetHeight() const noexcept
	{
		return MaxBound.y - MinBound.y;
	}

	glm::vec2 GetSize() const noexcept
	{
		return MaxBound - MinBound;
	}

	void Extend(const glm::vec2& point) noexcept
	{
		MinBound.x = glm::min(MinBound.x, point.x);
		MinBound.y = glm::min(MinBound.y, point.y);
		MaxBound.x = glm::max(MaxBound.x, point.x);
		MaxBound.y = glm::max(MaxBound.y, point.y);
	}

	void UniteWith(const AABB2d& other) 
	{
		assert(other.Valid());

		MinBound.x = glm::min(MinBound.x, other.MinBound.x);
		MinBound.y = glm::min(MinBound.y, other.MinBound.y);
		MaxBound.x = glm::max(MaxBound.x, other.MaxBound.x);
		MaxBound.y = glm::max(MaxBound.y, other.MaxBound.y);
	}

	void IntersectWith(const AABB2d& other)
	{
		assert(other.Valid());

		MinBound.x = glm::max(MinBound.x, other.MinBound.x);
		MinBound.y = glm::max(MinBound.y, other.MinBound.y);
		MaxBound.x = glm::min(MaxBound.x, other.MaxBound.x);
		MaxBound.y = glm::min(MaxBound.y, other.MaxBound.y);
	}


	AABB2d GetUnion(const AABB2d& other) const
	{
		AABB2d aabb(*this);
		aabb.UniteWith(other);
		return aabb;
	}

	AABB2d GetIntersection(const AABB2d& other) const
	{
		AABB2d aabb(*this);
		aabb.IntersectWith(other);
		return aabb;
	}

	AABB2d GetExtended(const glm::vec2& point) const
	{
		AABB2d aabb(*this);
		aabb.Extend(point);
		return aabb;
	}
};

#endif