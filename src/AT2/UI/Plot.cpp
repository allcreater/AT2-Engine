#include "UI.h"

#include <algorithm>
#include <limits>

using namespace AT2::UI;

//TODO: unit tests on AABB and probably this
const AABB2d & AT2::UI::Plot::CurveData::GetCurveBounds()
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

void AT2::UI::Plot::CurveData::Dirty() noexcept
{
	m_dirtyFlag = true;
	m_rangeNeedsUpdate = true;
	m_aabb.MaxBound.y = m_aabb.MinBound.y = 0.0f;

	if (auto parent = m_parent.lock())
		parent->DirtyCurves();
	else
		assert(false);
}


size_t Plot::EnumerateCurves(std::function<void(const std::string_view, const std::vector<float>&, bool, std::pair<float, float>)> fn)
{
	size_t counter = 0;
	std::for_each(m_curvesData.begin(), m_curvesData.end(), [&](decltype(m_curvesData)::value_type& x) {
		fn(x.first, x.second.Data, x.second.m_dirtyFlag, std::make_pair(x.second.m_aabb.MinBound.x, x.second.m_aabb.MaxBound.x));
		x.second.m_dirtyFlag = false;  //TODO: remove govnocode!!!
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

void Plot::ComputeAABB()
{
	m_allBounds.Reset();

	for (auto& curve : m_curvesData)
	{
		m_allBounds.UniteWith(curve.second.GetCurveBounds());
	}
}