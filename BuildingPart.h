#pragma once
#include "heads.h"
#include "Footprint.h"

class BuildingPart
{
public:
	explicit BuildingPart(std::string);
	~BuildingPart(void);

	inline void addRoof(const OGRPolygon* roof){_roofs->addGeometry(roof);}
	inline void addWall(const OGRPolygon* wall){ _walls->addGeometry(wall);}
	inline void setFootprint(const Footprint fp){_footprint = fp;}

	inline std::string getId()const{return _id;}
	inline int getNumRoofs()const {return _roofs->getNumGeometries();}
	inline int getNumWalls()const {return _walls->getNumGeometries();}
	inline OGRMultiPolygon* getRoofs()const {return _roofs;}
	inline OGRMultiPolygon* getWalls()const {return _walls;}
	inline Footprint& getFootprint() {return _footprint;}
	
    double getAltitude();
	double getHeight();

private:
	std::string _id;
	OGRMultiPolygon* _roofs;
	OGRMultiPolygon* _walls;

	double _altitude;
	double _height;

	Footprint _footprint;
};