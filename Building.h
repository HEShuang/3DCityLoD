#pragma once
#include "BuildingPart.h"



class Building
{
public:
	explicit Building(std::string);
	~Building(void);

	inline void addPart(BuildingPart& part){_parts.push_back(part);_footprints_lod20.push_back(part.getFootprint());}
	inline std::vector<BuildingPart>& getParts() {return _parts;}

	double getAltitude();
	double getHeight();
	
	void generalization_3d();										   
	osg::ref_ptr<osg::Group> getModel(LOD);
	osg::ref_ptr<osg::Geode> getFootprint(LOD);

	bool isAdjacent(Building *);
	void merge(std::vector<Footprint>& input,std::vector<Footprint>& output);

//	inline const std::string& getId(void) const {return _id;}											
private:
	void generalization_2d_lod15();
	void generalization_2d_lod10();
	bool extrusion_lod15();
	bool extrusion_lod10();

	osg::ref_ptr<osg::Geode> osg_assemble(OGRMultiPolygon*,bool doTesselate,osg::Vec4 color = GRAY1);	
	osg::ref_ptr<osg::Geode> osg_assemble(std::vector<Footprint>&,osg::Vec4 color = WHITE);

public:
	bool _visited;
	int _index;
	std::vector<Footprint> _footprints_lod10;

private:
	std::string _id;
	std::vector<BuildingPart> _parts;
	double _altitude;
	double _height;

	std::vector<Footprint> _footprints_lod20;
	std::vector<Footprint> _footprints_lod15;		
	OGRMultiPolygon* _roofs_lod15;
	OGRMultiPolygon* _walls_lod15;

 	OGRMultiPolygon* _roofs_lod10;
	OGRMultiPolygon* _walls_lod10;
};
