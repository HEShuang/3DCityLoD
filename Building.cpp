#include "StdAfx.h"
#include "Building.h"


Building::Building(std::string id):_id(id),_altitude(0),_height(0),_visited(false),_index(-1)
{
   
   _roofs_lod15 = new OGRMultiPolygon();
   _roofs_lod10 = new OGRMultiPolygon();

   _walls_lod15 = new OGRMultiPolygon();
   _walls_lod10 = new OGRMultiPolygon();

}

Building::~Building(void)
{
}
double Building::getAltitude()
{
   if(!_altitude)
   {
	   int n = _parts.size();
	   double sumA = 0;
	   for(int i = 0;i< n;i++)
		   sumA += _parts[i].getAltitude();
	   _altitude = sumA/n;
   }
   return _altitude;
}


double Building::getHeight()
{
   if(!_height)
   {
	   int n = _parts.size();
	   double sumH = 0;
	   for(int i = 0;i< n;i++)
		  sumH += _parts[i].getHeight();
	   _height = sumH/n;
   }
   return _height;
}



void Building::generalization_3d()
{
  generalization_2d_lod15();
  extrusion_lod15();

  generalization_2d_lod10();
  extrusion_lod10();
}

osg::ref_ptr<osg::Group> Building::getModel(LOD lod)
{
	osg::ref_ptr<osg::Group> group = new osg::Group();

	switch(lod)
	{
	case Lod10:
		group->addChild(osg_assemble(_walls_lod10,osg::PrimitiveSet::POLYGON,false,GRAY2));
		group->addChild(osg_assemble(_roofs_lod10,osg::PrimitiveSet::POLYGON,true,GRAY1));
	//	group->addChild(osg_assemble(_roofs_lod10,osg::PrimitiveSet::LINE_STRIP,false,RED));
		break;

	case Lod15:
		group->addChild(osg_assemble(_walls_lod15,osg::PrimitiveSet::POLYGON,false,GRAY2));
		group->addChild(osg_assemble(_roofs_lod15,osg::PrimitiveSet::POLYGON,true,GRAY1));
	//	group->addChild(osg_assemble(_roofs_lod15,osg::PrimitiveSet::LINE_STRIP,false,RED));
		break;

	case Lod20:
		for(int i = 0;i<_parts.size();i++)
		{	
			group->addChild(osg_assemble(_parts[i].getWalls(),osg::PrimitiveSet::POLYGON,false,GRAY2));
			group->addChild(osg_assemble(_parts[i].getRoofs(),osg::PrimitiveSet::POLYGON,true,GRAY1));
		  //  group->addChild(osg_assemble(_parts[i].getRoofs(),osg::PrimitiveSet::LINE_STRIP,false,RED));
		}
		break;
	}

	return group;
}

osg::ref_ptr<osg::Geode> Building::getFootprint(LOD lod)
{
	switch(lod)
	{
	case Lod10:
		return osg_assemble(_footprints_lod10);   
	case Lod15:
		return osg_assemble(_footprints_lod15);
	case Lod20:
		return osg_assemble(_footprints_lod20);
	}
}



bool Building::isAdjacent(Building *other)
{
	int n1 = _footprints_lod10.size(), n2 = other->_footprints_lod10.size();
	for(int i=0;i<n1;i++)
		for(int j=0;j<n2;j++)
			if(_footprints_lod10[i].isAdjacent(&(other->_footprints_lod10[j])))
				return true;
	return false;
}

void Building::merge(std::vector<Footprint> &input, std::vector<Footprint> &output)
{
	std::vector<Footprint> temp1,temp2(input);

	for(int i=0;i<_footprints_lod10.size();i++)
	{  	
		temp1.assign(temp2.begin(),temp2.end());
		temp2.clear();
		
		Footprint fp1,fp2(_footprints_lod10[i]);
		
		for(int j=0;j< temp1.size();j++)
		{	
			if(fp2.isAdjacent(&temp1[j]))
			{
				fp1 = fp2;
				fp1.merge(&temp1[j],fp2,Average);
				continue;
			}
			temp2.push_back(temp1[j]);

		}
		temp2.push_back(fp2);

	} 

	output.assign(temp2.begin(),temp2.end());
}


int Building::getNumVertices(LOD lod)
{
	int verts = 0;
	switch(lod)
	{
	case Lod10:
		verts += num_vertices(_roofs_lod10);
		verts += num_vertices(_walls_lod10);
		return verts;

	case Lod15:
		verts += num_vertices(_roofs_lod15);
		verts += num_vertices(_walls_lod15);
		return verts;

	case Lod20:
		for(int i = 0;i<_parts.size();i++)
		{
			verts += num_vertices(_parts[i].getRoofs());
			verts += num_vertices(_parts[i].getWalls());
		}
		return verts;
	}
	return verts;
}


 /////////////////////////////////////////////////////////////////////////////////
 //////////////////////////////////////////////////////////////////////////////////

// private functions

int Building::num_vertices(OGRMultiPolygon* polygons)
{
	int verts = 0;
	for(int i=0;i<polygons->getNumGeometries();i++)
	{
		verts += ((OGRPolygon*)(polygons->getGeometryRef(i)))->getExteriorRing()->getNumPoints() -1;
		
		if(	int n = ((OGRPolygon*)(polygons->getGeometryRef(i)))->getNumInteriorRings())
			for(int j=0;j<n;j++)
				verts += ((OGRPolygon*)(polygons->getGeometryRef(i)))->getInteriorRing(j)->getNumPoints()-1;
	}
	return verts;
}

typedef struct _FootprintNode{	// for building adjacency table
	Footprint* pFt;
	_FootprintNode* next;
	_FootprintNode():pFt(NULL),next(NULL){}
}FootprintNode;

void Building::generalization_2d_lod15()
{
	_footprints_lod15.clear();
	
	int n = _parts.size();
	if(n==1)  	//if this building has only one part, no need of merging
	{
		_footprints_lod15.push_back(_parts.at(0).getFootprint());
		return;
	}

 	
	//build adjacent-coequal table. The height ratio of two coequal parts stays within 0.5-2
	std::vector<FootprintNode> adjGraph(n);
	FootprintNode* p;
	for(int i=0;i<n;i++)
	{
		adjGraph[i].pFt = &(_parts[i].getFootprint());
		_parts[i].getFootprint()._index =i;
		for(int j=0;j<n;j++)
		{
			if (i==j)
				continue;
			if(_parts.at(i).getFootprint().isAdjacent(&(_parts.at(j).getFootprint()))&& 
				_parts.at(i).getFootprint().isCoequal(&(_parts.at(j).getFootprint())) )
			{
				FootprintNode* adjNode = new FootprintNode();
				adjNode->pFt = &(_parts[j].getFootprint());
				p = &adjGraph[i];
				while(p->next!=NULL)
					p = p->next;
				p->next = adjNode;
			}
		}
	}
	
	//build merge sequence using breadth-first search	
	std::vector<std::vector<Footprint*>> merge_lists;
	for(int i=0;i<n;i++)
	{
		if(adjGraph[i].pFt->_visited)
			continue;
		
		std::vector<Footprint*> list;
		list.push_back(adjGraph[i].pFt);
		adjGraph[i].pFt->_visited = 1;
	
		std::queue<int> qVisited;
		
		int flag;//if id_cur changed
		int id_cur = i;
		do{	
			p = &adjGraph[id_cur];
			flag = 0;
			while(p->next!=NULL)
			{
				p = p->next;
				if(p->pFt->_visited)
					continue;				   
				list.push_back(p->pFt);
				p->pFt->_visited = 1;
				qVisited.push(p->pFt->_index);
			}

			if(!qVisited.empty())
			{
				id_cur = qVisited.front();
				qVisited.pop();
				flag = 1;
			}

		}while(flag);

		merge_lists.push_back(list);
	}

	
	//generalize footprints_lod15 by merging adjacent-coequal parts
	int m = merge_lists.size();
	for(int i=0;i< m;i++)
	{
		Footprint fp1,fp2 = *merge_lists[i][0];

		int k = merge_lists[i].size();
		if( k >1)
		{   
			for(int j = 1; j< k;j++)
			{   	
				fp1 = fp2;
				if(fp1._height>= _height || merge_lists[i][j]->_height >= _height)
					fp1.merge(merge_lists[i][j],fp2,Ceil);
				else
					fp1.merge(merge_lists[i][j],fp2,Floor);
			}
		}

        // delete small inter rings
		if(int nInter = fp2._polygon->getNumInteriorRings())
		{
			for(int q = 0;q< nInter;q++)
				if(fp2._polygon->getInteriorRing(q)->get_Area() < fp2._polygon->getExteriorRing()->get_Area()/10)
					fp2._polygon->getInteriorRing(q)->empty();   

		    //if all inter rings are empty, replace with a new polygon only has its exter ring
			int flag = 1;
			for(int q = 0;q< nInter;q++)
				if(!fp2._polygon->getInteriorRing(q)->IsEmpty())
				{
					flag = 0; 
					break;
				}

			if(flag)
			{
				OGRPolygon* ply = new OGRPolygon();
				ply->addRing(fp2._polygon->getExteriorRing());
				fp2._polygon->empty();
				fp2._polygon = ply;
			}

		//	std::cout<<fp2._polygon->getNumInteriorRings()<<std::endl;
		}

		_footprints_lod15.push_back(fp2);
	
	} 


	
							
	
}

void Building::generalization_2d_lod10()
{
	if(_parts.size()==1)  	//if this building has only one part, no need of merging
	{
		_footprints_lod10.push_back(_parts.at(0).getFootprint());
		return;
	}	
	
	if(_footprints_lod15.empty())
		generalization_2d_lod15();
    
	int n = _footprints_lod15.size();	
	
	//initialize footprints_lod10 by exterior rings of footprints_lod15
	for(int i = 0;i<n;i++) 
	{   
		Footprint fp(_footprints_lod15[i]._polygon->getExteriorRing());
		fp._altitude =	_footprints_lod15[i]._altitude;
		fp._height = _footprints_lod15[i]._height;
		_footprints_lod10.push_back(fp);
	}

	
	//if there are more than one polygons, generalize footprints_lod10 by merging all its adjacent parts
	if( n > 1)  
	{
		std::vector<Footprint>::iterator it1,it2;
		int flag;
		do
		{
			flag = 0;
			int i,j;
			for( i=0,it1 = _footprints_lod10.begin(); i<_footprints_lod10.size();i++,it1++)
			{
				for( j=i,it2 = it1;j<_footprints_lod10.size();j++,it2++)
				{
					if(i==j)
						continue;

					if( it1->isAdjacent(&_footprints_lod10[j])
						||it1->_polygon->Overlaps(it2->_polygon)
						||it1->_polygon->Contains(it2->_polygon) 
						||it1->_polygon->Within(it2->_polygon))
					{	
						Footprint fp;
						it1->merge(&_footprints_lod10[j],fp,Average);

						_footprints_lod10.erase(it2);
						_footprints_lod10.erase(it1);
						_footprints_lod10.push_back(fp);

						flag = 1;
						break;
					}
				}
				if(flag)
					break;
			}
		}while(flag);
	}
	for(int i=0;i<_footprints_lod10.size();i++)
	{
		if(int m=_footprints_lod10[i]._polygon->getNumInteriorRings())
			for(int j=0;j<m;j++)
				(_footprints_lod10[i]._polygon->getInteriorRing(j))->empty();
	}
	
	////simplify polygons by merging fragment edges
	//for(int i=0;i<_footprints_lod10.size();i++)
	//	_footprints_lod10.at(i).merge_fragment_edges();

}

bool Building::extrusion_lod15()
{
    if(_footprints_lod15.empty())
		return false;

	int n = _footprints_lod15.size();
	for(int i=0;i<n;i++)
	{
		if(!_footprints_lod15[i].getNumVertices())
			continue;
		
		// get exterior ring vertices of footprint (clockwise)
		std::vector<OGRPoint> pts_foot_exter;
		_footprints_lod15[i].getVerticesExteriorRing(pts_foot_exter);
		
		// extrude exterior walls (clockwise) 
		int n_pts_exter = pts_foot_exter.size();
		for(int j=0;j<n_pts_exter-1;j++)   
		{		
			OGRPolygon wall;
			OGRLinearRing* ring_wall = new OGRLinearRing();

			OGRPoint p1(pts_foot_exter[j].getX(),pts_foot_exter[j].getY(),_footprints_lod15[i]._altitude);
			OGRPoint p2(pts_foot_exter[j+1].getX(),pts_foot_exter[j+1].getY(),_footprints_lod15[i]._altitude);
			OGRPoint p3(pts_foot_exter[j+1].getX(),pts_foot_exter[j+1].getY(),_footprints_lod15[i]._height);
			OGRPoint p4(pts_foot_exter[j].getX(),pts_foot_exter[j].getY(),_footprints_lod15[i]._height);
			ring_wall->addPoint(&p1);		    
			ring_wall->addPoint(&p2);
			ring_wall->addPoint(&p3);
			ring_wall->addPoint(&p4);
			ring_wall->addPoint(&p1);
			wall.addRing(ring_wall);
			_walls_lod15->addGeometry(&wall);
		}

		
		// create roof with exterior ring  (clockwise)
		
		OGRPolygon roof;
		OGRLinearRing* ring_exter = new OGRLinearRing();
		for(int j=0;j<n_pts_exter;j++)
		{
			OGRPoint pt(pts_foot_exter[j].getX(),pts_foot_exter[j].getY(),_footprints_lod15[i]._height);
			ring_exter->addPoint(&pt);
		}
		roof.addRing(ring_exter);
	
											  
		// if there are interior rings (counterclockwise)
	    if(_footprints_lod15[i].getNumInteriorRings())
		{	
			int n_rings_inter = _footprints_lod15[i].getNumInteriorRings();
			for(int j =0;j< n_rings_inter;j++)
			{   
				// extrude interior walls (clockwise)

				std::vector<OGRPoint> pts_foot_inter_j;
				_footprints_lod15[i].getVerticesInteriorRing(j,pts_foot_inter_j);
		  		int n_pts_inter_j = pts_foot_inter_j.size();  
				for(int k=0;k<n_pts_inter_j-1;k++)   
				{		
					OGRPolygon wall;
					OGRLinearRing* ring_wall = new OGRLinearRing();

					OGRPoint p1(pts_foot_inter_j[k].getX(),pts_foot_inter_j[k].getY(),_footprints_lod15[i]._altitude);
					OGRPoint p2(pts_foot_inter_j[k+1].getX(),pts_foot_inter_j[k+1].getY(),_footprints_lod15[i]._altitude);
					OGRPoint p3(pts_foot_inter_j[k+1].getX(),pts_foot_inter_j[k+1].getY(),_footprints_lod15[i]._height);
					OGRPoint p4(pts_foot_inter_j[k].getX(),pts_foot_inter_j[k].getY(),_footprints_lod15[i]._height);
					ring_wall->addPoint(&p1);
					ring_wall->addPoint(&p2);
					ring_wall->addPoint(&p3);
					ring_wall->addPoint(&p4);
					ring_wall->addPoint(&p1);
					wall.addRing(ring_wall);
					_walls_lod15->addGeometry(&wall);
				}

				// add interior ring to roof (counterclockwise)
				OGRLinearRing* ring_inter_j = new OGRLinearRing();
				for(int k=0;k<n_pts_inter_j;k++)
				{
					OGRPoint pt(pts_foot_inter_j[k].getX(),pts_foot_inter_j[k].getY(),_footprints_lod15[i]._height);
					ring_inter_j->addPoint(&pt);

				}
				roof.addRing(ring_inter_j);
			
			}
			
		}

		_roofs_lod15->addGeometry(&roof);

	}

	return true;

}

bool Building::extrusion_lod10()
{
	if(_footprints_lod10.empty())
		return false;

	int n = _footprints_lod10.size();
	for(int i=0;i<n;i++)
	{		
		if(!_footprints_lod10[i].getNumVertices())
			continue;
		
		std::vector<OGRPoint> pts_foot;
		_footprints_lod10[i].getVerticesExteriorRing(pts_foot);
		
		int n_pts= pts_foot.size();
		for(int j=0;j<n_pts-1;j++)   
		{		
			OGRPolygon wall;
			OGRLinearRing* ring_wall = new OGRLinearRing();

			OGRPoint p1(pts_foot[j].getX(),pts_foot[j].getY(),_footprints_lod10[i]._altitude);
			OGRPoint p2(pts_foot[j+1].getX(),pts_foot[j+1].getY(),_footprints_lod10[i]._altitude);
			OGRPoint p3(pts_foot[j+1].getX(),pts_foot[j+1].getY(),_footprints_lod10[i]._height);
			OGRPoint p4(pts_foot[j].getX(),pts_foot[j].getY(),_footprints_lod10[i]._height);
			ring_wall->addPoint(&p1);
			ring_wall->addPoint(&p2);
			ring_wall->addPoint(&p3);
		    ring_wall->addPoint(&p4);
			ring_wall->addPoint(&p1);
			wall.addRing(ring_wall);
			_walls_lod10->addGeometry(&wall);
		} 


		OGRPolygon roof;
		OGRLinearRing* ring = new OGRLinearRing();
		for(int j=0;j<n_pts;j++)
		{
			OGRPoint pt(pts_foot[j].getX(),pts_foot[j].getY(),_footprints_lod10[i]._height);
			ring->addPoint(&pt);
		}
		roof.addRing(ring);
	   _roofs_lod10->addGeometry(&roof);	

	}



}



osg::ref_ptr<osg::Geode> Building::osg_assemble(OGRMultiPolygon* polygons,osg::PrimitiveSet::Mode mode,bool doTesselate,osg::Vec4 color)
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
	    
			geom->addPrimitiveSet(new osg::DrawArrays(mode,0,m));
			geode->addDrawable(geom);
		}  
	}
	else
	{
	 	for(int i=0;i<n;i++)
		{
		//	osg::ref_ptr<osg::Geometry> geom = new osg::Geometry();
		//	osg::ref_ptr<osg::Vec3Array> verts = new osg::Vec3Array();

		//	RDPolygon ply((OGRPolygon*)polygons->getGeometryRef(i));
		//	
		//	// set vertices		
		//	const std::vector<TVec3d>& vertices = ply.getVertices();

		//	for(int j=0;j<vertices.size();j++)
		//		verts->push_back(osg::Vec3(vertices[j].x,vertices[j].y,vertices[j].z));

		//	geom->setVertexArray(verts);

		//	// set indices
		//	osg::DrawElementsUInt* indices = new osg::DrawElementsUInt( osg::PrimitiveSet::TRIANGLES, 0 );
		//	const std::vector<unsigned int>& ind = ply.getIndices();
		//	for ( int j = 0 ; j < ind.size() / 3; j++ )
		//	{
		//		indices->push_back( ind[ j * 3 + 0 ] );
		//		indices->push_back( ind[ j * 3 + 1 ] );
		//		indices->push_back( ind[ j * 3 + 2 ] );	
		//	}

		//	geom->addPrimitiveSet( indices );

		//	// set colors
		//	
		//	osg::Vec4Array* colors = new osg::Vec4Array();
		//	colors->push_back(color);
		//	geom->setColorArray(colors);
		//	geom->setColorBinding(osg::Geometry::BIND_OVERALL);
	 //   
		//	geode->addDrawable(geom);
		////////////////////////////////////////////////////////////////////
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
			geom->addPrimitiveSet(new osg::DrawArrays(mode,n_start,n_pts));
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


osg::ref_ptr<osg::Geode> Building::osg_assemble(std::vector<Footprint> &footprints,osg::Vec4 color)
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