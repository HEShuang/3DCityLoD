#pragma once
#include "heads.h"
#include "Building.h"

class BuildingGroup
{
public:
	explicit BuildingGroup(std::vector<Building*>&);
	~BuildingGroup(void);

	osg::ref_ptr<osg::Group> getModel();
	osg::ref_ptr<osg::Geode> getFootprint();
	int getNumVertices();

private:
	void grouping(std::vector<Building*>&);
	void extrusion();
	osg::ref_ptr<osg::Geode> osg_assemble(OGRMultiPolygon*,bool doTesselate,osg::Vec4);
	osg::ref_ptr<osg::Geode> osg_assemble(std::vector<Footprint>&,osg::Vec4 color = WHITE);

public:
	int _index;
	std::vector<int> _idBldgs;
	std::vector<Footprint> _footprints;

private:
	OGRMultiPolygon* _roofs;
	OGRMultiPolygon* _walls;


};					
