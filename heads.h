#ifndef HEADS_H
#define HEADS_H


#include <citygml.h> 

#include <GL\glut.h>
#include <ogr_geometry.h>
#include <ogrsf_frmts.h>

#include <string>
#include <vector>
#include <math.h>
#include <queue> 
#include <map>

#include <osgDB/ReadFile>
#include <osgDB/Registry>
#include <osgDB/WriteFile>
#include <osgDB/ReaderWriter>
#include <osg/Notify>
#include <osg/Fog>
#include <osg/PolygonMode>

#include <osg/ref_ptr>
#include <osg/Group>
#include <osg/Geode>
#include <osg/Geometry>
#include <osg/Material>
#include <osg/Texture2D>
#include <osg/LightModel>

#include <osgText/Font>
#include <osgText/Text>

#include <osgUtil/Tessellator>

#include <osgViewer/Viewer>

#define GRAY1 osg::Vec4f(0.85f,0.85f,0.85f,1.0f)
#define GRAY2 osg::Vec4f(0.4f,0.4f,0.4f,1.0f)
#define WHITE osg::Vec4f(1.0f,1.0f,1.0f,0.94f)
#define BLACK osg::Vec4f(0.0f,0.0f,0.0f,1.0f)
#define RED osg::Vec4f(1.0f,0.0f,0.0f,1.0f)
#define ORANGE osg::Vec4f(1.0f,0.14f,0.0f,1.0f)
#define GREEN osg::Vec4f(0.0f,1.0f,0.0f,1.0f)
#define YELLOW osg::Vec4f(1.0f,1.0f,0.0f,0.5f)
#define KHAKI osg::Vec4f(1.0f,0.96f,0.0f,0.56f)


typedef enum 
{
	Lod20,
    Lod15,
	Lod10,
	Lod05,
}LOD;

#endif