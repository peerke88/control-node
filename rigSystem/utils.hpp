#ifndef UTILS_HPP

#include <maya/MBoundingBox.h>

#include <random>
#include <algorithm>
#include <cmath>
#include <fstream>
#include <iostream>
#include <limits>

inline MBoundingBox getBoundingBox(MPointArray inPoints, double sizeValue, MMatrix offsetMatrix){
	double minX = std::numeric_limits<double>::max();
	double minY = std::numeric_limits<double>::max();
	double minZ = std::numeric_limits<double>::max();
	double maxX = -std::numeric_limits<double>::max();
	double maxY = -std::numeric_limits<double>::max();
	double maxZ = -std::numeric_limits<double>::max();

	for (unsigned int c = 0; c < inPoints.length(); c++) {
		MPoint key = (inPoints[c]* sizeValue)*offsetMatrix;
		minX = std::min(key.x, minX);
		minY = std::min(key.y, minY);
		minZ = std::min(key.z, minZ);
		maxX = std::max(key.x, maxX);
		maxY = std::max(key.y, maxY);
		maxZ = std::max(key.z, maxZ);
	}
	MPoint corner1(minX, minY, minZ);
	MPoint corner2(maxX, maxY, maxZ);
	return MBoundingBox(corner1, corner2);
}

template <typename T>
inline T getPlugValue(const MDagPath& mDagPath, const MObject& mPlug, const T default_value)
{
	MStatus status;
	const MObject mObject = mDagPath.node(&status);
	if (status)
	{
		MPlug plug(mObject, mPlug);
		if (!plug.isNull())
		{
			T value;
			if (plug.getValue(value))
			{
				return value;
			}
		}
	}
	return default_value;
}

#define UTILS_HPP
#endif