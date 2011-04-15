// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#ifndef _WIN32_WINNT		// Allow use of features specific to Windows XP or later.                   
#define _WIN32_WINNT 0x0501	// Change this to the appropriate value to target other versions of Windows.
#endif						

#include <stdio.h>
#include <tchar.h>



// TODO: reference additional headers your program requires here
#ifdef _DEBUG
#pragma comment(lib,"citygmld.lib")
#pragma comment(lib,"xerces-c_3D.lib")
#pragma comment(lib,"gdal_i.lib")

#pragma comment(lib,"OpenThreadsd.lib")
#pragma comment(lib,"osgd.lib")
#pragma comment(lib,"osgDBd.lib")
#pragma comment(lib,"osgFXd.lib")
#pragma comment(lib,"osgGAd.lib")
#pragma comment(lib,"osgManipulatord.lib")
#pragma comment(lib,"osgTextd.lib")
#pragma comment(lib,"osgUtild.lib")
#pragma comment(lib,"osgViewerd.lib")
#endif

#ifdef NDEBUG
#pragma comment(lib,"citygml.lib")
#pragma comment(lib,"xerces-c_3.lib")
#pragma comment(lib,"gdal_i.lib")

#pragma comment(lib,"OpenThreads.lib")
#pragma comment(lib,"osg.lib")
#pragma comment(lib,"osgDB.lib")
#pragma comment(lib,"osgFX.lib")
#pragma comment(lib,"osgGA.lib")
#pragma comment(lib,"osgManipulator.lib")
#pragma comment(lib,"osgText.lib")
#pragma comment(lib,"osgUtil.lib")
#pragma comment(lib,"osgViewer.lib")

#endif