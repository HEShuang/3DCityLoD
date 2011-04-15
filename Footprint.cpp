#include "StdAfx.h"
#include "Footprint.h"

Footprint::Footprint(void):_index(-1),_visited(false),_degenerated(false),_altitude(0),_height(0)
{
	_polygon = new OGRPolygon();
}

Footprint::~Footprint(void)
{
}

Footprint::Footprint(OGRLinearRing* r):_index(-1),_visited(false),_degenerated(false),_altitude(0),_height(0)
{
	if(r->IsEmpty())
		std::cerr<<"Footprint constructor error: empty ring"<<std::endl;
	
	// create polygon with clockwise exterior ring
	OGRLinearRing * ring = new OGRLinearRing(r);
	remove_colinear_edges(ring);
	
	if(ring->getNumPoints()>3)
		adjust_winding(ring,ClockWise);
	else
		_degenerated = true;

	_polygon = new OGRPolygon();
	_polygon->addRingDirectly(ring);	

   	_polygon = new OGRPolygon();
	_polygon->addRing(r);
	_polygon->closeRings();
	

}	

Footprint::Footprint(OGRPolygon* ply):_index(-1),_visited(false),_degenerated(false),_altitude(0),_height(0)
{	 
	if(ply->IsEmpty())
		std::cerr<<"Footprint constructor error: empty polygon"<<std::endl;
	
    // create polygon with clockwise exterior ring
	OGRLinearRing *ring_ex = new OGRLinearRing(ply->getExteriorRing());
	
	remove_colinear_edges(ring_ex);

	if(ring_ex->getNumPoints()>3)
		adjust_winding(ring_ex,ClockWise);
	else
		_degenerated = true;

	_polygon = new OGRPolygon();
	_polygon->addRingDirectly(ring_ex);
	
	// add counterclockwise interior rings 
	if(int n = ply->getNumInteriorRings() && !_degenerated  )
		for(int i=0;i<n;i++)
		{
			OGRLinearRing *ring_in = new OGRLinearRing(ply->getInteriorRing(i));
			
			//remove_colinear_edges(ring_in);

			if(ring_in->getNumPoints()>3)	
			{	
			    adjust_winding(ring_in,CounterClockWise);
				_polygon->addRingDirectly(ring_in);
			}
		}
	_polygon = (OGRPolygon*)ply->clone();
	_polygon->closeRings();
}

Footprint::Footprint(const Footprint& other)
{
	_polygon = (OGRPolygon*)other._polygon->clone();
	_visited = other._visited;
	_degenerated = other._degenerated;
	_index = other._index;
	_altitude = other._altitude;
	_height = other._height;
}

Footprint& Footprint::operator =(const Footprint & other)
{
	_index = other._index;
	_visited = other._visited;
	_degenerated = other._degenerated;
	_height = other._height;
	_polygon->empty();
	_polygon = (OGRPolygon*)other._polygon->clone();
	return *this;
}

bool Footprint::isAdjacent(Footprint *p)
{
	if(!p)
		return false;
	if(!_polygon->Intersects(p->_polygon))
		return false;
	
	// intersecting with muliti points is not adjacent
	OGRGeometry* pGeom = _polygon->Intersection(p->_polygon);
	if( pGeom->getGeometryType() == wkbPoint || pGeom->getGeometryType() == wkbMultiPoint)
		return false;
	return true;
	
}

bool Footprint::isCoequal(Footprint *p)
{
	if(!p) 
		return false;
	if( _height/p->_height>2 || _height/p->_height<0.5) 
		return false; 
	return true;
}

void Footprint::merge(Footprint *p,Footprint& newFp,MergeType type)
{
	OGRPolygon* ply = (OGRPolygon*)(_polygon->Union(p->_polygon));
	newFp = Footprint(ply);
	double a1 = p->_altitude,h1 = p->_height;
	switch(type)
	{
	case Ceil:
		newFp._altitude = _altitude > a1? _altitude:a1;
		newFp._height = _height > h1? _height:h1;
		break;

	case Floor:
		newFp._altitude = _altitude < a1? _altitude:a1;
		newFp._height = _height < h1? _height:h1;
		break;
		
	case Average:
		newFp._altitude = (_altitude + a1)/2;
		newFp._height = (_height + h1)/2;
		break;
	}
}


int Footprint::getNumVertices() const
{
   	if( !_polygon->getExteriorRing())
		return 0;
 	
	if(!_polygon->getNumInteriorRings())
		return _polygon->getExteriorRing()->getNumPoints();

	int n=_polygon->getNumInteriorRings();
	int verts = _polygon->getExteriorRing()->getNumPoints();
	for(int i=0;i<n;i++)
		verts+=	_polygon->getInteriorRing(i)->getNumPoints();
	return verts; 
}
int Footprint::getNumInteriorRings() const
{
	return _polygon->getNumInteriorRings();
}

bool Footprint::getVerticesExteriorRing(std::vector<OGRPoint>& points) const
{
	if( !_polygon->getExteriorRing())
		return false ;
		
	int n =_polygon->getExteriorRing()->getNumPoints();
		
	for(int i=0;i<n;i++)
	{	
		OGRPoint pt;
		_polygon->getExteriorRing()->getPoint(i,&pt);
		points.push_back(pt);
	}
	return true;
}

bool Footprint::getVerticesInteriorRing(int i,std::vector<OGRPoint> & points) const 
{
	if(i<0 || i>=_polygon->getNumInteriorRings())
		return false;


	int nPoints = _polygon->getInteriorRing(i)->getNumPoints();
	for(int j=0;j< nPoints;j++)
	{
		OGRPoint pt;
		_polygon->getInteriorRing(i)->getPoint(j,&pt);
		points.push_back(pt);
	}

	return true;		
}


/////////////////////////////////////////////////////////////////////////
// private functions only called by constructors

void Footprint::remove_colinear_edges(OGRLinearRing* ring)
{
	ring->closeRings();
	if(ring->getNumPoints()<4)
		return;
	
	std::vector<OGRPoint> pts1,pts2; //pts1 contains the original points while pts2 contains the results
	int flag =0; // to check if any vertex is removed
	
	int n=ring->getNumPoints();
	for(int i=0;i<n;i++)
	{
		OGRPoint pt;
		ring->getPoint(i,&pt);
		pts1.push_back(pt);
	}	

	pts2.push_back(pts1[0]);
	pts2.push_back(pts1[1]);									
	
	for(int i=2;i<n;i++)
	{																
		int j = pts2.size()-1;
	    OGRPoint* p1 = &pts2[j-1],*p2 = &pts2[j],*p3 = &pts1[i];
		OGRPoint v12(p2->getX()-p1->getX(),p2->getY()-p1->getY());
		OGRPoint v23(p3->getX()-p2->getX(),p3->getY()-p2->getY());

		double cos =( v12.getX() * v23.getX() + v12.getY() * v23.getY() )  / (p2->Distance(p1))*(p3->Distance(p2));

		if(cos==1)
		{
			pts2.pop_back();
			flag = 1;
		}
	
		pts2.push_back(*p3);					
	}

	if(flag)
	{
		ring->empty();
		ring = new OGRLinearRing();
		for(int i=0;i<pts2.size();i++)
			ring->addPoint(&pts2[i]);
	}

}


void Footprint::adjust_winding(OGRLinearRing *ring,WindingType winding)
{
	ring->closeRings();
	switch(winding)
	{
	case ClockWise:
		if(!ring->isClockwise())
			reverse(ring);
		break;
	case CounterClockWise:
		if(ring->isClockwise())
			reverse(ring);
		break;
	}
}

void Footprint::reverse(OGRLinearRing* ring)
{
	std::vector<OGRPoint> pts;
 	int n=ring->getNumPoints();
	for(int i=0;i<n;i++)
	{
		OGRPoint pt;
		ring->getPoint(i,&pt);
		pts.push_back(pt);
	}

	ring->empty();
	ring = new OGRLinearRing();
	std::vector<OGRPoint>::reverse_iterator rit;
	for(rit = pts.rbegin(); rit < pts.rend(); ++rit)
	   ring->addPoint(&(*rit));


}

