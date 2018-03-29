#include "UI.h"

#include <algorithm>
#include <limits>

using namespace AT2::UI;

//TODO: unit tests on AABB and probably this
inline const AABB2d & AT2::UI::Plot::CurveData::GetCurveBounds()
{
	if (m_rangeNeedsUpdate)
	{
		m_aabb.MaxBound.y = m_aabb.MinBound.y = 0.0f;
		
		if (Data.size() >= 2)
		{
			//could be optimised with raw array access but it will be computed lazy[right as me :) ] and only after data has changed
			auto iteratorsPair = std::minmax_element(Data.begin(), Data.end());
			m_aabb.MinBound.y = *iteratorsPair.first;
			m_aabb.MaxBound.y = *iteratorsPair.second;
		}
		else if (Data.size() == 1)
		{
			m_aabb.MaxBound.y = m_aabb.MinBound.y = Data.front();
		}
		m_rangeNeedsUpdate = false;
	}

	return m_aabb;
}

inline void AT2::UI::Plot::CurveData::Dirty() noexcept
{
	m_dirtyFlag = true;
	m_rangeNeedsUpdate = true;
	m_aabb.MaxBound.y = m_aabb.MinBound.y = 0.0f;
}


Plot::CurveData& Plot::GetOrCreateCurve(const std::string& curveName)
{
	auto pair = m_curvesData.try_emplace(curveName, CurveData());
	return pair.first->second;
}

void Plot::ComputeAABB()
{
	m_allBounds.Reset();

	for (auto& curve : m_curvesData)
	{
		m_allBounds.UniteWith(curve.second.GetCurveBounds());
	}
}
