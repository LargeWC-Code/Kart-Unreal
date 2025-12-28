/********************************************************************
created:	2024/12/XX
filename: 	ucunreal3d.h
author:		Auto Generated

purpose:	UCUnreal3D - Unreal Engine 后端实现
*********************************************************************/
#ifndef _UC_UCUNREAL3D_H_
#define _UC_UCUNREAL3D_H_

#include "ucunreal3d/basetype.h"
#include "ucunreal3d/ucunreal3ddevice.h"

// 注意：后续挂接 Unreal 时需要包含相关头文件
// #include "ucunreal3d/ucunreal3dtexture.h"
// #include "ucunreal3d/ucunreal3dvertexbuffer.h"
// #include "ucunreal3d/ucunreal3dindexbuffer.h"
// #include "ucunreal3d/ucunreal3dsurface.h"

#if defined _SYS_WIN32_ || defined _SYS_WIN64_ || defined _SYS_WINAPP_

	#if defined UNICODE || defined _UNICODE
	#	ifdef _DEBUG 
	#		pragma comment(lib,"ucunreal3du.lib")
	#	else
	#		pragma comment(lib,"ucunreal3du.lib")
	#	endif
	#else
	#	ifdef _DEBUG 
	#		pragma comment(lib,"ucunreal3d.lib")
	#	else
	#		pragma comment(lib,"ucunreal3d.lib")
	#	endif
	#endif

#elif defined _SYS_WIN64_

	#if defined UNICODE || defined _UNICODE
	#	ifdef _DEBUG 
	#		pragma comment(lib,"ucunreal3du64.lib")
	#	else
	#		pragma comment(lib,"ucunreal3du64.lib")
	#	endif
	#else
	#	ifdef _DEBUG 
	#		pragma comment(lib,"ucunreal3d64.lib")
	#	else
	#		pragma comment(lib,"ucunreal3d64.lib")
	#	endif
	#endif

#endif	//_SYS_WIN32_

#endif	//_UC_UCUNREAL3D_H_


