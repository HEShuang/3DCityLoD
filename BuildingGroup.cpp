#include "StdAfx.h"
#include "BuildingGroup.h"

BuildingGroup::BuildingGroup(std::vector<Building*>& bldgs):_index(-1)
{
	_roofs = new OGRMultiPolygon;
	_walls = new OGRMultiPolygon;
	grouping(bldgs);
	extrusion();
}

BuildingGroup::~BuildingGroup(void)
{
}

osg::ref_ptr<osg::Group> BuildingGroup::getModel()
{
   osg::ref_ptr<osg::Group> group = new osg::Group();
   group->addChild(osg_assemble(_walls,false));
   group->addChild(osg_assemble(_roofs,true,RED));
   return group;
}

osg::ref_ptr<osg::Geode> BuildingGroup::getFootprint()
{
   return osg_assemble(_footprints);
}

////////////////////////////////////////////////////////////////////////////
void BuildingGroup::grouping(std::vector<Building*>& bldgs)
{
	if(bldgs.size()<2)
		return;

	_idBldgs.clear();
	_idBldgs.push_back(bldgs[0]->_index);

	std::vector<Footprint> fps1,fps2(bldgs[0]->_footprints_lod10);

	for(int i=1;i<bldgs.size()-1;i++)
	{
		fps1.assign(fps2.begin(),fps2.end());
		bldgs[i]->merge(fps1,fps2);	
		_idBldgs.push_back(bldgs[i]->_index);
	}

	_footprints.assign(fps2.begin(),fps2.end());
}	

void BuildingGroup::extrusion()
{
	for(int i=0;i<_footprints.size();i++)
	{
		std::vector<OGRPoint> pts_foot;
		_footprints[i].getVerticesExteriorRing(pts_foot);
		
		int n_pts= pts_foot.size();
		for(int j=0;j<n_pts-1;j++)   
		{		
			OGRPolygon wall;
			OGRLinearRing* ring_wall = new OGRLinearRing();

			OGRPoint p1(pts_foot[j].getX(),pts_foot[j].getY(),_footprints[i]._altitude);
			OGRPoint p2(pts_foot[j+1].getX(),pts_foot[j+1].getY(),_footprints[i]._altitude);
			OGRPoint p3(pts_foot[j+1].getX(),pts_foot[j+1].getY(),_footprints[i]._height);
			OGRPoint p4(pts_foot[j].getX(),pts_foot[j].getY(),_footprints[i]._height);
			ring_wall->addPoint(&p1);
			ring_wall->addPoint(&p2);
			ring_wall->addPoint(&p3);
		    ring_wall->addPoint(&p4);
			ring_wall->addPoint(&p1);
			wall.addRing(ring_wall);
			_walls->addGeometry(&wall);
		} 


		OGRPolygon roof;
		OGRLinearRing* ring = new OGRLinearRing();
		for(int j=0;j<n_pts;j++)
		{
			OGRPoint pt(pts_foot[j].getX(),pts_foot[j].getY(),_footprints[i]._height);
			ring->addPoint(&pt);
		}
		roof.addRing(ring);
	   _roofs->addGeometry(&roof);
	}
}


osg::ref_ptr<osg::Geode> BuildingGroup::osg_assemble(OGRMultiPolygon* polygons,bool doTesselate,osg::Vec4 color)
{
	if(!polygons)
		return 0;
	
	osg::ref_ptr<osg::Geode> geode = new osg::Geode();
	int n = polygons->getNumGeometries();

	if(!doTesselate)
	{
		for(int i=0;i<n;i++)
		{
			osg::ref_ptr<osg::Geometry> geom = new osg::Geometry();
			osg::ref_ptr<osg::Vec3Array> verts = new osg::Vec3Array();

			OGRLinearRing *ring = ((OGRPolygon*)polygons->getGeometryRef(i))->getExteriorRing();
			int m = ring->getNumPoints();
			for(int j=0;j<m;j++)
			{
				OGRPoint pt;
				ring->getPoint(j,&pt);
				verts->push_back(osg::Vec3(pt.getX(),pt.getY(),pt.getZ()));
			}

			geom->setVertexArray(verts);
			
			osg::Vec4Array* colors = new osg::Vec4Array();
			colors->push_back(color);
			geom->setColorArray(colors);
			geom->setColorBinding(osg::Geometry::BIND_OVERALL);
	    
			geom->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::POLYGON,0,m));
			geode->addDrawable(geom);
		}  
	}
	else
	{
	 	for(int i=0;i<n;i++)
		{
			osg::Geometry *geom = new osg::Geometry();
			osg::Vec3Array *verts = new osg::Vec3Array();			
			geom->setVertexArray(verts);

			OGRLinearRing *ring = ((OGRPolygon*)polygons->getGeometryRef(i))->getExteriorRing();
			int n_pts = ring->getNumPoints();
			for(int j=0;j<n_pts;j++)
			{
				OGRPoint pt;
				ring->getPoint(j,&pt);
				verts->push_back(osg::Vec3(pt.getX(),pt.getY(),pt.getZ()));
			}
			int n_start = 0;
			geom->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::POLYGON,n_start,n_pts));
			n_start += n_pts;

			if(int n_rings_inter = ((OGRPolygon*)polygons->getGeometryRef(i))->getNumInteriorRings())
			{	for(int j=0;j<n_rings_inter;j++)
				{
					OGRLinearRing *ring_j = ((OGRPolygon*)polygons->getGeometryRef(i))->getInteriorRing(j);
					int n_pts_j = ring_j->getNumPoints();
					for(int k=0;k<n_pts_j;k++)
					{
						OGRPoint pt;
						ring_j->getPoint(k,&pt);
						verts->push_back(osg::Vec3(pt.getX(),pt.getY(),pt.getZ()));
					}
					geom->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::POLYGON,n_start,n_pts_j));
					n_start += n_pts_j;

				}
			}

			osg::ref_ptr<osgUtil::Tessellator> tess = new osgUtil::Tessellator;
			tess->setTessellationType(osgUtil::Tessellator::TESS_TYPE_GEOMETRY);
			tess->setBoundaryOnly(false);
			tess->setWindingType(osgUtil::Tessellator::TESS_WINDING_ODD);
			tess->retessellatePolygons(*geom);
		
			osg::Vec4Array* colors = new osg::Vec4Array();
			colors->push_back(color);
			geom->setColorArray(colors);
			geom->setColorBinding(osg::Geometry::BIND_OVERALL);

			geode->addDrawable(geom);

		}

	}

	return geode;
}	



osg::ref_ptr<osg::Geode> BuildingGroup::osg_assemble(std::vector<Footprint> &footprints,osg::Vec4 color)
{

	osg::ref_ptr<osg::Geode> geode = new osg::Geode();

	for(int i=0;i<footprints.size();i++)
	{
		osg::ref_ptr<osg::Geometry> geom = new osg::Geometry();
		osg::ref_ptr<osg::Vec3Array> verts = new osg::Vec3Array();

		OGRLinearRing *ring = footprints[i]._polygon->getExteriorRing();
		for(int j=0;j<ring->getNumPoints();j++)								  
		{
			OGRPoint pt;
			ring->getPoint(j,&pt);
			verts->push_back(osg::Vec3(pt.getX(),pt.getY(),pt.getZ()));
		}

		geom->setVertexArray(verts);
		
		osg::Vec4Array* colors = new osg::Vec4Array();
		colors->push_back(color);
		geom->setColorArray(colors);
		geom->setColorBinding(osg::Geometry::BIND_OVERALL);
    
		geom->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::LINE_STRIP,0,ring->getNumPoints()));
		geode->addDrawable(geom);


		if(	int n_in = footprints[i]._polygon->getNumInteriorRings())
			for(int j=0;j<n_in;j++)
			{
				osg::ref_ptr<osg::Geometry> geom_j = new osg::Geometry();
				osg::ref_ptr<osg::Vec3Array> verts_j = new osg::Vec3Array();

				OGRLinearRing *ring_j = footprints[i]._polygon->getInteriorRing(j);
				for(int k=0;k<ring_j->getNumPoints();k++)
				{									
					OGRPoint pt;
					ring_j->getPoint(k,&pt);
					verts_j->push_back(osg::Vec3(pt.getX(),pt.getY(),pt.getZ()));
				}
				geom_j->setVertexArray(verts_j);
				
				osg::Vec4Array* colors = new osg::Vec4Array();
				colors->push_back(color);
				geom_j->setColorArray(colors);
				geom_j->setColorBinding(osg::Geometry::BIND_OVERALL);
		    
				geom_j->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::LINE_STRIP,0,ring_j->getNumPoints()));
				geode->addDrawable(geom_j);
			}
	} 
	return geode;


}								 