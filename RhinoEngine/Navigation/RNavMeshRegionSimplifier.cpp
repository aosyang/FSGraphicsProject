//=============================================================================
// RNavMeshRegionSimplifier.cpp by Shiyang Ao, 2020 All Rights Reserved.
//
// 
//=============================================================================

#include "Core/CoreTypes.h"

#include "RNavMeshRegionSimplifier.h"
#include "RNavMeshGenerator.h"
#include "Core/StdHelper.h"

void RNavMeshRegionSimplifier::Execute(const std::set<int>& UniqueRegionIds, std::vector<EdgePointCollection>& InOutRegionEdgePoints)
{
	for (const auto& RegionId : UniqueRegionIds)
	{
		EdgePointCollection& RegionEdges = InOutRegionEdgePoints[RegionId];

		// Find a first mandatory point
		int FirstIdx;
		FirstIdx = FindFirstMandatoryPoint(RegionEdges);

		if (FirstIdx != -1)
		{
			int StartIdx = FirstIdx;
			int EndIdx = -1;
			std::vector<RVec3> SimplifiedRegionPoints;
			std::vector<int> MandatoryIndices;
			MandatoryIndices.push_back(0);

			do
			{
				std::vector<RVec3> Points;
				std::vector<int> EdgeIndices;

				// Collect points along the edge until meeting a next mandatory point
				for (int i = 0; i <= (int)RegionEdges.size(); i++)
				{
					int Idx = (StartIdx + i) % RegionEdges.size();
					Points.push_back(RegionEdges[Idx].Point);
					EdgeIndices.push_back(Idx);

					if (i != 0 && RegionEdges[Idx].bIsMandatory)
					{
						EndIdx = Idx;
						break;
					}
				}

				assert(EndIdx != -1);

				// Note: In some cases, the point array will form a loop with the starting point being the end point.
				//		 Avoid this so the edge simplification behaves properly.
				if (StartIdx == EndIdx)
				{
					Points.pop_back();
					EdgeIndices.pop_back();
					EndIdx = *(EdgeIndices.end() - 1);
				}

				MandatoryIndices.push_back((int)SimplifiedRegionPoints.size());

				std::vector<RVec3> SimplifiedEdge = SimplifyEdges(Points);
				SimplifiedRegionPoints.insert(SimplifiedRegionPoints.end(), SimplifiedEdge.begin(), SimplifiedEdge.end());

				StartIdx = EndIdx;
			} while (StartIdx != FirstIdx);

			// For debug draw regions
			std::vector<EdgePointData> NewEdgePoints;
			for (int i = 0; i < (int)SimplifiedRegionPoints.size(); i++)
			{
				const RVec3& p = SimplifiedRegionPoints[i];
				bool bIsMandatory = StdContains(MandatoryIndices, i);
				NewEdgePoints.push_back({ p,  bIsMandatory });
			}

			RegionEdges = NewEdgePoints;
		}
		else
		{
			// When a region is not sharing any edges with other regions, it doesn't have a mandatory edge point.
			// We will simplify geometries by combining shorter line segments into a longer one. 
			int StartIndex = 0;
			do
			{
				const RVec3& p0 = RegionEdges[StartIndex].Point;
				const RVec3& p1 = RegionEdges[(StartIndex + 1) % RegionEdges.size()].Point;
				const RVec3& p2 = RegionEdges[(StartIndex + 2) % RegionEdges.size()].Point;

				if (FLT_EQUAL(RVec3::Dot((p1 - p0).GetNormalized(), (p2 - p0).GetNormalized()), 1.0f))
				{
					RegionEdges.erase(RegionEdges.begin() + StartIndex + 1);
				}
				else
				{
					StartIndex++;
				}
			} while (StartIndex < RegionEdges.size());
		}
	}
}

int RNavMeshRegionSimplifier::FindFirstMandatoryPoint(const EdgePointCollection &RegionEdges) const
{
	for (int FirstIdx = 0; FirstIdx < (int)RegionEdges.size(); FirstIdx++)
	{
		if (RegionEdges[FirstIdx].bIsMandatory)
		{
			return FirstIdx;
		}
	}
	
	return -1;
}

std::vector<RVec3> RNavMeshRegionSimplifier::SimplifyEdges(const std::vector<RVec3>& Edges)
{
	assert(Edges.size() >= 2);

	if (Edges.size() == 2)
	{
		return std::vector<RVec3>{ Edges[0] };
	}

	const RVec3& Start = Edges[0];
	const RVec3& End = Edges[Edges.size() - 1];
	float MaxSqrDist = 0.0f;
	int MaxPointIdx = -1;

	for (int i = 1; i < (int)Edges.size() - 1; i++)
	{
		float SqrDist = CalculateSquaredDistanceOfPointToLineSegment(Edges[i], Start, End);
		if (SqrDist > MaxSqrDist)
		{
			MaxSqrDist = SqrDist;
			MaxPointIdx = i;
		}
	}

	static const float Threshold = 70.0f;
	if (MaxSqrDist <= Threshold * Threshold)
	{
		// All distances from the line segment to points are smaller than the threshold. The edge list can be simplified as one edge.
		return std::vector<RVec3>{ Edges[0] };
	}
	else
	{
		// Taking the point with a max distance to the line segment as a middle point,
		// split the edges into two sublists and perform the algorithm on each of them.
		auto Left = SimplifyEdges(std::vector<RVec3>(Edges.begin(), Edges.begin() + MaxPointIdx + 1));
		auto Right = SimplifyEdges(std::vector<RVec3>(Edges.begin() + MaxPointIdx, Edges.end()));

		std::vector<RVec3> Result;
		Result.reserve(Left.size() + Right.size());
		Result.insert(Result.end(), Left.begin(), Left.end());
		Result.insert(Result.end(), Right.begin(), Right.end());

		return Result;
	}
}

float RNavMeshRegionSimplifier::CalculateSquaredDistanceOfPointToLineSegment(const RVec3& p, const RVec3& a, const RVec3& b)
{
	assert(a - b != RVec3::Zero());

	RVec3 ap = p - a;
	RVec3 ab = b - a;

	float f = RVec3::Dot(ap, ab) / RVec3::Dot(ab, ab);
	if (f <= 0.0f)
	{
		return ap.SquaredMagitude();
	}
	else if (f >= 1.0f)
	{
		return (p - b).SquaredMagitude();
	}
	else
	{
		RVec3 q = a + ab * f;
		return (p - q).SquaredMagitude();
	}
}
