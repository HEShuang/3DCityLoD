#pragma once
#include "heads.h"

class MapExtruder
{
public:
	MapExtruder(const char*,double);
	~MapExtruder(void);
	osg::ref_ptr<osg::Group> osg_assemble();

private:
	void extrusion(void);


private:
	const char* _map2dFile;
	double _height;

	OGRMultiPolygon* _map2d;
	OGRMultiPolygon* _sides;
};
