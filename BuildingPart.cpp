#include "StdAfx.h"
#include "BuildingPart.h"

BuildingPart::BuildingPart(std::string id)
{
	_id = id;
	_roofs = new OGRMultiPolygon();
	_walls = new OGRMultiPolygon();

	_altitude = 0;
	_height = 0;
}

BuildingPart::~BuildingPart(void)
{
}

double BuildingPart::getAltitude()					
{
	if(!_altitude)
	{
		if(_footprint._altitude)
			_altitude = _footprint._altitude;
		else
		{
			// caculate from _walls 
		}
	}
	return _altitude;
}

double BuildingPart::getHeight()
{
	if(!_height)
	{
		if(_footprint._height)
			_height = _footprint._height;
		else
		{
			// caculate from _roofs 
		}
	}
	return _height;
}