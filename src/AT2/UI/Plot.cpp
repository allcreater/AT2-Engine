#include "UI.h"

#include <algorithm>
#include <limits>

using namespace AT2::UI;

void Plot::CurveData::SetData(std::vector<float>&& data, bool autoRange) 
{
	m_data = std::move(data); 
	Dirty();

	if (autoRange)
		SetXRange(0.0, m_data.size());
}

void Plot::CurveData::SetXRange(float startX, float endX) 
{
	m_aabb.MinBound.x = startX; m_aabb.MaxBound.x = endX;
}

//TODO: unit tests on AABB and probably this
const AABB2d & Plot::CurveData::GetCurveBounds() const
{
	if (m_rangeNeedsUpdate)
	{
		m_aabb.MaxBound.y = m_aabb.MinBound.y = 0.0f;
		
		if (m_data.size() >= 2)
		{
			//could be optimised with raw array access but it will be computed lazy[right as me :) ] and only after data has changed
			auto iteratorsPair = std::minmax_element(m_data.begin(), m_data.end());
			m_aabb.MinBound.y = *iteratorsPair.first;
			m_aabb.MaxBound.y = *iteratorsPair.second;
		}
		else if (m_data.size() == 1)
		{
			m_aabb.MaxBound.y = m_aabb.MinBound.y = m_data.front();
		}
		m_rangeNeedsUpdate = false;
	}

	return m_aabb;
}

void AT2::UI::Plot::CurveData::Dirty() noexcept
{
	m_dataInvalidatedFlag = true;
	m_rangeNeedsUpdate = true;
	m_aabb.MaxBound.y = m_aabb.MinBound.y = 0.0f;

	if (auto parent = m_parent.lock())
		parent->DirtyCurves();
	else
		assert(false);
}


size_t Plot::EnumerateCurves(std::function<bool(const std::string_view, const CurveData&, bool)> fn)
{
	size_t counter = 0;
	std::for_each(m_curvesData.begin(), m_curvesData.end(), [&](decltype(m_curvesData)::value_type& x) 
	{
		x.second.m_dataInvalidatedFlag &= !fn(x.first, x.second, x.second.m_dataInvalidatedFlag); //invalidation flag could be resetted, but not setted
		counter++; 
	});

	return counter;
}

Plot::CurveData& Plot::GetOrCreateCurve(const std::string& curveName)
{
	auto pair = m_curvesData.try_emplace(curveName, CurveData(weak_from_this()));
	return pair.first->second;
}

const AABB2d & AT2::UI::Plot::GetAABB()
{
	if (m_boundsShouldBeRecalculated)
	{
		ComputeAABB();
		m_boundsShouldBeRecalculated = false;
	}

	return m_allBounds;
}

void Plot::SetObservingZone(const AABB2d & zone)
{
	if (GetAABB().GetIntersection(zone).Valid())
		m_observingZone = zone;
}

void Plot::ComputeAABB()
{
	m_allBounds.Reset();

	for (auto& curve : m_curvesData)
	{
		m_allBounds.UniteWith(curve.second.GetCurveBounds());
	}
}
