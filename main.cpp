// 3DCityLoD.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "MapExtruder.h"
#include "City.h"

class UseEventHandler:public osgGA::GUIEventHandler
{
    
public:
	virtual bool handle(const osgGA::GUIEventAdapter& ea,osgGA::GUIActionAdapter& aa)
	{
		osgViewer::Viewer* viewer = dynamic_cast<osgViewer::Viewer*>(&aa);
		if(!viewer)return false;

		if (ea.getEventType()==osgGA::GUIEventAdapter::KEYDOWN)
		{

			if(ea.getKey()==osgGA::GUIEventAdapter::KEY_Up)
			{   
				int n = viewer->getSceneData()->asGroup()->getNumChildren();
				for(int i= 0;i<n;i++)
					if(viewer->getSceneData()->asGroup()->getChild(i)->getNodeMask()&& i+1<n)
					{
						viewer->getSceneData()->asGroup()->getChild(i+1)->setNodeMask(1);
						viewer->getSceneData()->asGroup()->getChild(i)->setNodeMask(0);
						break;

					}

			}
			if(ea.getKey()==osgGA::GUIEventAdapter::KEY_Down)
			{
				int n = viewer->getSceneData()->asGroup()->getNumChildren();
				for(int i=n-1;i>=0;i--)
					if(viewer->getSceneData()->asGroup()->getChild(i)->getNodeMask() && i-1>=0)
					{
						viewer->getSceneData()->asGroup()->getChild(i)->setNodeMask(0);
						viewer->getSceneData()->asGroup()->getChild(i-1)->setNodeMask(1);
						break;

					}


			}

		}
		return false;
	}
};

int _tmain(int arge,_TCHAR* argv[])
{
	
//	osg::ref_ptr<osg::Node> root(osgDB::readNodeFile("d:/data/zone1-CityGML/ZoneAExporter.citygml"));
	
	
	std::cout<<"loading citygml file"<<std::endl;
	City zone1("d:/data/zone1-CityGML/ZoneAExporter.citygml");

	std::cout<<"generalizing..."<<std::endl;
	zone1.generalization_single_buildings();
	zone1.generalization_building_groups();

	std::cout<<"producing lod model"<<std::endl;

	osg::ref_ptr<osg::Group> footprints_lod20 = zone1.getFootprint(Lod20);

	osg::ref_ptr<osg::Group> footprints_lod15 = zone1.getFootprint(Lod15);
	footprints_lod15->setNodeMask(0);

	osg::ref_ptr<osg::Group> footprints_lod10 = zone1.getFootprint(Lod10);
	footprints_lod10->setNodeMask(0);

	osg::ref_ptr<osg::Group> footprints_lod05 = zone1.getFootprint(Lod05);
	footprints_lod05->setNodeMask(0);	
	
	osg::ref_ptr<osg::Group> zone1_lod20 = zone1.getModel(Lod20);
	zone1_lod20->setNodeMask(0);

	osg::ref_ptr<osg::Group> zone1_lod15 = zone1.getModel(Lod15);
	zone1_lod15->setNodeMask(0);

	osg::ref_ptr<osg::Group> zone1_lod10 = zone1.getModel(Lod10);
	zone1_lod10->setNodeMask(0);
	
	osg::ref_ptr<osg::Group> zone1_lod05 = zone1.getModel(Lod05);
	zone1_lod05->setNodeMask(0);



	osg::ref_ptr<osg::Group> root = new osg::Group();
	root->addChild(footprints_lod20);
	root->addChild(footprints_lod15);
	root->addChild(footprints_lod10);
	root->addChild(footprints_lod05);

	root->addChild(zone1_lod20);
	root->addChild(zone1_lod15);
	root->addChild(zone1_lod10);
	root->addChild(zone1_lod05);


														


	//MapExtruder mapNantes1("E:/data/quartiers/IRIS_NANTES_08_region_Aggreg_Lambert93.shp",200);
	//osg::ref_ptr<osg::Group> nantes1= mapNantes1.osg_assemble();
	//	
	//MapExtruder mapNantes2("E:/data/quartiers/IRIS_NANTES_08_region_split11.shp",100);
	//osg::ref_ptr<osg::Group> nantes2 = mapNantes2.osg_assemble();

	//osg::ref_ptr<osg::Node> rivers (osgDB::readNodeFile("E:/data/water/SURFACE_EAU_Selec1.shp"));

	//osg::ref_ptr<osg::Fog> fog(new osg::Fog());
	//fog->setMode(osg::Fog::LINEAR);
	//fog->setColor(WHITE);

	//osg::ref_ptr<osg::StateSet> ss = rivers->getOrCreateStateSet();
	//ss->setAttributeAndModes(fog);
 //
 //   nantes2->addChild(rivers);
	//nantes2->setNodeMask(0);


 //  	MapExtruder mapNantes3("E:/data/quartiers/IRIS_NANTES_08_region_split22.shp",50);
	//osg::ref_ptr<osg::Group> nantes3 = mapNantes3.osg_assemble();
	//nantes3->addChild(rivers);
	//nantes3->setNodeMask(0);


	//MapExtruder mapNantes4("E:/data/quartiers/IRIS_NANTES_08_region_split3.shp",10);
	//osg::ref_ptr<osg::Group> nantes4 = mapNantes4.osg_assemble();
 //   osg::ref_ptr<osg::Node> road(osgDB::readNodeFile("E:/data/roads/nantes_Project.shp"));
	//nantes4->addChild(road);
	//nantes4->addChild(rivers);
	//nantes4->addChild(zone1_lod10);
	//nantes4->setNodeMask(0);

	//osg::ref_ptr<osg::Group> root = new osg::Group();
	//root->addChild(nantes1);
	//root->addChild(nantes2);
 //   root->addChild(nantes3); 	
	//root->addChild(nantes4);



	osgViewer::Viewer viewer;

	viewer.setLightingMode(osg::View::NO_LIGHT);
	osg::StateSet* globalState = viewer.getCamera()->getStateSet();
	if(globalState)
	{
		osg::LightModel* lightModel = new osg::LightModel;
		lightModel->setAmbientIntensity(osg::Vec4(1,1,1,1));
		globalState->setAttributeAndModes(lightModel,osg::StateAttribute::ON);
	}

	// viewer.getCamera()->setClearColor(osg::Vec4f(0.69f,0.76f,0.87f,1.0f));
	viewer.setSceneData(root.get());
	viewer.addEventHandler(new UseEventHandler());
	viewer.realize();
	viewer.run();  

	return 0;
}
