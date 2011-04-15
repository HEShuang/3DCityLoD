#include "StdAfx.h"
#include "City.h"

City::City(const char* citygmlFile) 
{
	_citygmlFile = citygmlFile;

	_citygmlModel = citygml::load(_citygmlFile,citygml::ParserParams());
	
	if(!_citygmlModel)
	{
		std::cout<<"failure in loading citygml file"<<std::endl;
		return;
	}

	init();

}

City::~City(void)
{
}

bool City::generalization_single_buildings()
{
	int n = _allBldgs.size();
	for(int i=0;i<n;i++)			
		_allBldgs.at(i).generalization_3d();
	return true;		
}



bool City::generalization_building_groups()
{
	//build adjacent table
	int n = _allBldgs.size();
	std::vector<BuildingNode> adjGraph(n);
	BuildingNode *p;
	for(int i=0;i<n;i++)
	{	
		adjGraph[i].pBldg = &_allBldgs[i];
		_allBldgs[i]._index = i;
		for(int j=0;j<n;j++)
		{														  
			if(i==j)
				continue;
			if(_allBldgs[i].isAdjacent(&_allBldgs[j]))
			{
				BuildingNode* adjNode = new BuildingNode;
				adjNode->pBldg = &_allBldgs[j];
				p = &adjGraph[i];
				while(p->next!=NULL)
					p = p->next;
				p->next = adjNode;
			}
		}
	}


	//build merge sequence using breadth-first search	
	std::vector<std::vector<Building*>> merge_lists;
	for(int i=0;i<n;i++)		   
	{								 
		if(adjGraph[i].pBldg->_visited)
			continue;
		
		std::vector<Building*> list;
		list.push_back(adjGraph[i].pBldg);
		adjGraph[i].pBldg->_visited = 1;
	
		std::queue<int> qVisited;
		
		int flag;//if id_cur changed
		int id_cur = i;
		do{	
			p = &adjGraph[id_cur];
			flag = 0;
			while(p->next!=NULL)
			{
				p = p->next;
				if(p->pBldg->_visited)
					continue;				   
				list.push_back(p->pBldg);
				p->pBldg->_visited = 1;
				qVisited.push(p->pBldg->_index);
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

	int m = merge_lists.size();
	for(int i=0;i< m;i++)
		if(merge_lists[i].size()>1)
			_allBldgGroups.push_back(BuildingGroup(merge_lists[i]));

	return true;
}

osg::ref_ptr<osg::Group> City::getModel(LOD lod)
{
	osg::ref_ptr<osg::Group> group = new osg::Group();
	
	if(lod==Lod05)
	{
		for(int i=0;i<_allBldgs.size();i++)
			group->addChild(_allBldgs[i].getModel(Lod10));

		for(int j=0;j<_allBldgGroups.size();j++)
		{
			std::vector<int>& ids = _allBldgGroups[j]._idBldgs;
			for	(int k=0;k<ids.size();k++)
				group->getChild(ids[k])->setNodeMask(0);
			group->addChild(_allBldgGroups[j].getModel());
		}

	}								  
	
	else
	{
		for(int i=0;i<_allBldgs.size();i++)
			group->addChild(_allBldgs.at(i).getModel(lod));
	}

	return group;
}


osg::ref_ptr<osg::Group> City::getFootprint(LOD lod)
{
	osg::ref_ptr<osg::Group> group = new osg::Group();
	
	if(lod==Lod05)
	{
		for(int i=0;i<_allBldgs.size();i++)
			group->addChild(_allBldgs[i].getFootprint(Lod10));

		for(int j=0;j<_allBldgGroups.size();j++)
		{
			std::vector<int>& ids = _allBldgGroups[j]._idBldgs;
			for	(int k=0;k<ids.size();k++)
				group->getChild(ids[k])->setNodeMask(0);
			group->addChild(_allBldgGroups[j].getFootprint());
		}

	}								  
	
	else
	{
		for(int i=0;i<_allBldgs.size();i++)
			group->addChild(_allBldgs[i].getFootprint(lod));
	}

	return group;
}
/////////////////////////////////////////////////////////////////////////////////////////// 
//                                    init()	                                        
///////////////////////////////////////////////////////////////////////////////////////////
// private function only called by constructor
//
// create buildings stored in _allBldgs based on imported citygml model(_citygmlModel)
// 
// each building consists of many building parts,and each part consists of roofs and walls
// for each building part:
//     only vertices are preserved,no texutre 
//     only one footprint is generated which has only one polygon
// 
///////////////////////////////////////////////////////////////////////////////////////////
void City::init()
{		
	// traverse all buildings of citygml model to create our building
	const citygml::CityObjects* gml_bldgs = _citygmlModel->getCityObjectsByType(citygml::COT_Building);
	citygml::CityObjects::const_iterator gml_bldg_it;	
	for(gml_bldg_it = gml_bldgs->begin(); gml_bldg_it != gml_bldgs->end(); gml_bldg_it++)
	{
		
		Building bldg((*gml_bldg_it)->getId());  
		
		// traverse all parts of current citygml building
		citygml::CityObjects gml_parts = (*gml_bldg_it)->getChildren();
		citygml::CityObjects::const_iterator gml_part_it;	
		for(gml_part_it = gml_parts.begin(); gml_part_it != gml_parts.end(); gml_part_it++)
		{  		

			BuildingPart part((*gml_part_it)->getId());  	
		
			double h_part = 0;  
 			std::vector<Edge> footEdges;

			// traverse all geometries of current citygml part. only two geometry types are handled: GT_Roof and GT_Wall
			int nGeom = (*gml_part_it)->size();  			
			for(int i=0;i<nGeom;i++)
			{
				const citygml::Geometry* geom = (*gml_part_it)->getGeometry(i);

				if(geom->getType()==citygml::GT_Roof)  	 
				{	
					// create a roof polygon
					OGRPolygon roof;
					OGRLinearRing *ring = new OGRLinearRing();
					std::vector<TVec3d> verts = (*geom)[0]->getVertices();
					int m = verts.size();
					for(int j=0;j<m;j++)
						ring->addPoint(verts[j].x,verts[j].y,verts[j].z);
					roof.addRing(ring);
					part.addRoof(&roof);
			
					// caculate the average height					
					double sumZ = 0;
					for(int j=0;j<m;j++)
						sumZ += verts[j].z;
			
					if(h_part ==0)
						h_part = sumZ/m;
					else
						h_part = (h_part + sumZ/m)/2;
					
					continue;
				}

				if(geom->getType()==citygml::GT_Wall)
				{
					//create a wall polygon
					OGRPolygon wall;
					OGRLinearRing *ring = new OGRLinearRing();
					std::vector<TVec3d> verts = (*geom)[0]->getVertices();
					int m = verts.size();
					for(int j=0;j<m;j++)
						ring->addPoint(verts[j].x,verts[j].y,verts[j].z);
					wall.addRing(ring);
					part.addWall(&wall);
					
					//find and store all foot edges of current building part
					footEdges.push_back(get_foot_edge((*geom)[0]));
				}
			}			
			
			// connect all foot edges into polylines
			PolyLines plys; 	
			assemble_polyline(footEdges,plys);

			// if more than one polygon are produced, we consider this building part as borderline data that we'd better cast off
			if(plys.size()>1) 
				continue;
			
			// create footprint
			OGRLinearRing ring;
			double sumZ = 0; // to caculate altitude
			int m=(int)plys[0].size();
			for(int j=0;j<m;j++)
			{
			  sumZ += plys[0].at(j).z;
			  ring.addPoint(plys[0].at(j).x,plys[0].at(j).y);
			}

			Footprint fp(&ring);
			if(fp._degenerated)// drop current part
				
			{
				std::cout<<"degenerated"<<std::endl;
				continue;
			}

			fp._altitude =	sumZ/m;
			fp._height = h_part; 
			part.setFootprint(fp);		

			bldg.addPart(part); 

		}

		//bldg.gen_footprints();

		//bldg.extrusion_lod15();
		//bldg.extrusion_lod1();

		_allBldgs.push_back(bldg);  
	}

}


/////////////////////////////////////////////////////////////////////////////////////////// 
//                                    get_foot_edge()	                                        
///////////////////////////////////////////////////////////////////////////////////////////
// private function only called by init()
//
// return the input wall's foot edge by finding the two vertices with the least Z values
///////////////////////////////////////////////////////////////////////////////////////////
Edge City::get_foot_edge(const citygml::Polygon* wall)
{
	if(!wall)
		std::cerr << "find foot edge error: wall empty " << std::endl;	
	
	std::vector<TVec3d> verts(wall->getVertices());
	std::vector<TVec3d>::iterator iter = verts.begin();
	
	TVec3d vecMinZ1 = *iter;

	for( iter= verts.begin();iter!= verts.end();++iter)
		if( iter->z < vecMinZ1.z)
			vecMinZ1 = *iter;

	iter =  verts.begin();
	TVec3d vecMinZ2 = *iter;

	while(vecMinZ2 == vecMinZ1)
		vecMinZ2 =*(++iter);

    for( iter= verts.begin();iter!= verts.end();++iter)
		if( iter->z < vecMinZ2.z && (*iter)!=vecMinZ1)
			vecMinZ2 = *iter;

	return Edge(vecMinZ1,vecMinZ2);

}


/////////////////////////////////////////////////////////////////////////////////////////// 
//                                    assemble_polyline()	                                        
///////////////////////////////////////////////////////////////////////////////////////////
// private function only called by init()
//
// connect all foot edges into polylines
// footEdges: a reference of a vector containing foot edges as input
// polyLines: a reference of PolyLines for storing the result  
// 
///////////////////////////////////////////////////////////////////////////////////////////
void City::assemble_polyline(std::vector<Edge>& footEdges,PolyLines& polyLines )
{
		
	    // build adjacency table
		int n = (int)footEdges.size();
		std::vector<EdgeNode> adjGraph(n);
		EdgeNode* p;
		for(int i=0;i<n;i++)
		{
			adjGraph[i].pEdge = &footEdges[i];
			footEdges[i].id =i;

			for(int j=0;j<n;j++)
			{
				if (i==j)
					continue;
				
				if (footEdges[i].v1 == footEdges[j].v1 || footEdges[i].v1 == footEdges[j].v2 || footEdges[i].v2 == footEdges[j].v1 || footEdges[i].v2 == footEdges[j].v2)
				{
					EdgeNode* adjNode = new EdgeNode();
					adjNode->pEdge = &footEdges[j];

					p = &adjGraph[i];
					while(p->next!=NULL)
						p = p->next;
					p->next = adjNode;
				}
			}
		}

	
		for(int i=0;i<n;i++)
		{
			if(adjGraph[i].pEdge->visited)
				continue;
		
			PolyLine ply;
			ply.push_back(adjGraph[i].pEdge->v1);
		    ply.push_back(adjGraph[i].pEdge->v2);

			// append adjacent edge to the last vertex of polyline

			int flag;
			int id_cur = i;
			do{
				adjGraph[id_cur].pEdge->visited = 1;
				p = &adjGraph[id_cur];
				flag = 0;

				while(p->next!=NULL)
				{
					p = p->next;
					if(p->pEdge->visited)
						continue;
					//
					if(ply.back() == p->pEdge->v1)
					{
						ply.push_back(p->pEdge->v2);
						id_cur = p->pEdge->id;
						flag = 1;
						break;
					}
					if(ply.back() == p->pEdge->v2)
					{
						ply.push_back(p->pEdge->v1);
						id_cur = p->pEdge->id;
						flag = 1;
						break;
					}
				}
			}while (flag);

			// insert adjacent edge to the first vertex of polyline 

			id_cur = i;

			do{
				adjGraph[id_cur].pEdge->visited = 1;
				p = &adjGraph[id_cur];
				flag = 0;
		        while(p->next!=NULL)
				{
					p = p->next;
					if(p->pEdge->visited)
						continue;
					//
					if(ply.front() == p->pEdge->v1)
					{
						ply.insert(ply.begin(),p->pEdge->v2);
						id_cur = p->pEdge->id;
						flag = 1;
						break;
					}
					if(ply.front() == p->pEdge->v2)
					{
						ply.insert(ply.begin(),p->pEdge->v1);
						id_cur = p->pEdge->id;
						flag = 1;
						break;
					}
				}

			}while (flag);


			polyLines.push_back(ply);
		}
}	


