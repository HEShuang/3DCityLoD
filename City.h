#pragma once
#include "BuildingGroup.h"

typedef struct _Edge{

	int id;
	TVec3d v1;			  
	TVec3d v2;
	int visited;
	
	_Edge(TVec3d _v1,TVec3d _v2):v1(_v1),v2(_v2),visited(0){}

}Edge;

typedef struct _EdgeNode{

	Edge* pEdge;
	_EdgeNode* next;

	_EdgeNode(): pEdge(NULL),next(NULL){}

}EdgeNode;

typedef std::vector< TVec3d > PolyLine;
typedef std::vector< PolyLine > PolyLines;

typedef struct _BuildingNode{
	Building *pBldg;
	_BuildingNode *next;

	_BuildingNode(): pBldg(NULL),next(NULL){}
}BuildingNode;

class City
{
public:
	explicit City(const char*);
	~City(void); 

	bool generalization_single_buildings();

	bool generalization_building_groups();

	osg::ref_ptr<osg::Group> getModel(LOD);
	osg::ref_ptr<osg::Group> getFootprint(LOD);

	int countFootprintPolygons(LOD);

private:
	void init();
    Edge get_foot_edge(const citygml::Polygon*);
	void assemble_polyline(std::vector<Edge>&,PolyLines&);

private:
	const char* _citygmlFile;
	citygml::CityModel* _citygmlModel;
	std::vector<Building> _allBldgs;
	std::vector<BuildingGroup> _allBldgGroups;
																	
};
