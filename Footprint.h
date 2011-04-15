#pragma once
#include "heads.h"

typedef enum 
{
    Ceil = 1,
	Floor,
	Average,
}MergeType;

typedef enum
{
	ClockWise,
	CounterClockWise,
}WindingType;

class Footprint
{
public:
	Footprint(void);
	~Footprint(void);
	explicit Footprint(OGRLinearRing* );
	explicit Footprint(OGRPolygon* );
	Footprint(const Footprint& );
	Footprint& operator = (const Footprint &);


	bool isAdjacent(Footprint*);
	bool isCoequal(Footprint*);
	void merge(Footprint* ,Footprint& ,MergeType);
	void merge_fragment_edges();
	
	int getNumVertices()const;
	int getNumInteriorRings()const;

	bool getVerticesExteriorRing(std::vector<OGRPoint>&) const;
	bool getVerticesInteriorRing(int,std::vector<OGRPoint>&) const;


private:

	void remove_colinear_edges(OGRLinearRing*);
	void adjust_winding(OGRLinearRing*,WindingType);
	void reverse(OGRLinearRing*);


	//void merge_fragment_edges();

	//inline setId(int id){_id = id;}
	//inline setVisited(bool visited) {_visited = visited;}
	//inline setHeight(double height) {_height = height;}

	//inline int getId(void) const {return _id;}
	//inline bool isVisited(void)	const {return _visited;}
	//inline double getHeight(void) const {return _height;}
	//inline OGRPolygon* getPolygon(void) const {return _polygon;}
	
public:
	int _index;
	bool _visited;
	bool _degenerated;	
	double _altitude;
	double _height;

	// if exterior ring is a segment '_degenerated' is set true; 
	// if a interior ring is a segment the ring is deleted, 
	OGRPolygon* _polygon;

};