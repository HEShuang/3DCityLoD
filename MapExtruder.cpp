#include "StdAfx.h"
#include "MapExtruder.h"

MapExtruder::MapExtruder(const char* map2dFile,double height)
{
	_map2dFile = map2dFile;
	_height = height;

	OGRRegisterAll();
	OGRDataSource *poDS = OGRSFDriverRegistrar::Open(_map2dFile,FALSE);
	if( poDS == NULL)
	{
		printf("Open failed.\n");
		exit(1);
	}
	_map2d = new OGRMultiPolygon();
	
	OGRLayer *poLayer;
	poLayer = poDS->GetLayer(0);

	OGRFeature *poFeature;
	poLayer->ResetReading();
	while((poFeature = poLayer->GetNextFeature())!=NULL)
	{
		OGRGeometry *poGeometry;
		poGeometry = poFeature ->GetGeometryRef();
		if(poGeometry!=NULL)
		{	
		   	if(poGeometry ->getGeometryType()==wkbPolygon)
			   _map2d->addGeometry((OGRPolygon*) poGeometry);
			if(poGeometry ->getGeometryType()==wkbMultiPolygon)
			{
				int nPolygons = ((OGRMultiPolygon*)poGeometry)->getNumGeometries();
				for(int i=0;i<nPolygons;i++)
					_map2d->addGeometry((OGRPolygon*)(((OGRMultiPolygon*)poGeometry)->getGeometryRef(i)));
			}

		}
	}

   extrusion();
}

MapExtruder::~MapExtruder(void)
{
}

void MapExtruder::extrusion()
{
	_sides = new OGRMultiPolygon();

	int n = _map2d->getNumGeometries();
	for(int i=0;i<n;i++)
	{

		OGRLinearRing* ring = ((OGRPolygon*)_map2d->getGeometryRef(i))->getExteriorRing();
		ring->closeRings();
		int nPoints = ring->getNumPoints();


		for(int j=0;j<nPoints-1;j++)
		{
			
			OGRPolygon side;
			OGRLinearRing* ring_side = new OGRLinearRing();

			OGRPoint p1,p4;

			ring->getPoint(j,&p1);
			ring->getPoint(j+1,&p4);

			OGRPoint p2(p1.getX(),p1.getY(),_height);
			OGRPoint p3(p4.getX(),p4.getY(),_height);
			
			
	
			ring_side->addPoint(&p1);			
		    ring_side->addPoint(&p2);
			ring_side->addPoint(&p3);
			ring_side->addPoint(&p4);
			ring_side->addPoint(&p1);

			side.addRing(ring_side);
			_sides->addGeometry(&side);
		}
	}
}


class ExtrudeVisitor:public osg::NodeVisitor
{
public:
	ExtrudeVisitor(double height):osg::NodeVisitor(osg::NodeVisitor::TRAVERSE_ALL_CHILDREN){h=height;}
	virtual void apply(osg::Geode& node)
	{
		int nDrawable = node.getNumDrawables();
		for(int i=0;i<nDrawable;i++)
		{
			osg::ref_ptr<osg::Vec3Array> verts = dynamic_cast<osg::Vec3Array*>(node.getDrawable(i)->asGeometry()->getVertexArray());
			osg::Vec3Array::iterator iter;
			for(iter = verts->begin();iter!=verts->end();iter++)
			   iter->_v[2]=h;

		}
	}
protected:
 	double h;
};

osg::ref_ptr<osg::Group> MapExtruder::osg_assemble()
{
  	osg::ref_ptr<osg::Group> osg = new osg::Group;
	osg::ref_ptr<osg::Node> surface = osgDB::readNodeFile(_map2dFile);

	ExtrudeVisitor extrude(_height);
	surface->accept(extrude);
	osg->addChild(surface);

	int n = _sides ->getNumGeometries();
	osg::ref_ptr<osg::Geode> geode = new osg::Geode;			


	for(int i=0;i<n;i++)
	{
		osg::ref_ptr<osg::Geometry> geom = new osg::Geometry;
		osg::ref_ptr<osg::Vec3Array> verts = new osg::Vec3Array;

		std::vector<OGRPoint> points;
		OGRLinearRing *ring = ((OGRPolygon*)_sides->getGeometryRef(i))->getExteriorRing();
		int m = ring->getNumPoints();
		for(int j=0;j<m;j++)
		{
			OGRPoint pt;
			ring->getPoint(j,&pt);
			verts->push_back(osg::Vec3(pt.getX(),pt.getY(),pt.getZ()));
		}

		geom->setVertexArray(verts);
	
		osg::Vec4Array* colors = new osg::Vec4Array;
		colors->push_back(YELLOW);
		geom->setColorArray(colors);
		geom->setColorBinding(osg::Geometry::BIND_OVERALL);
    

		geom->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::POLYGON,0,m));
		geode->addDrawable(geom);
	}
	
	osg->addChild(geode);

	return osg;
}