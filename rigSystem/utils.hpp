#ifndef UTILS_HPP

#include <maya/MBoundingBox.h>

#include <random>
#include <algorithm>
#include <cmath>
#include <fstream>
#include <iostream>
#include <limits>

inline MBoundingBox getBoundingBox(MPointArray inPoints, MDistance sizeValue, MMatrix offsetMatrix){
	double minX = std::numeric_limits<double>::max();
	double minY = std::numeric_limits<double>::max();
	double minZ = std::numeric_limits<double>::max();
	double maxX = -std::numeric_limits<double>::max();
	double maxY = -std::numeric_limits<double>::max();
	double maxZ = -std::numeric_limits<double>::max();

	double multiplier = sizeValue.asCentimeters();

	for (unsigned int c = 0; c < inPoints.length(); c++) {
		MPoint key = (inPoints[c]* multiplier)*offsetMatrix;
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

#define UTILS_HPP
#endif