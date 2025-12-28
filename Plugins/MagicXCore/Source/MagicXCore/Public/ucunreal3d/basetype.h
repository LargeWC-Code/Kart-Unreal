#pragma once

/********************************************************************
created:	2024/12/XX
filename: 	basetype.h
author:		Auto Generated

purpose:	UCUnreal3D 基础类型定义
*********************************************************************/
#ifndef _UC_UCUNREAL3D_BASETYPE_H_
#define _UC_UCUNREAL3D_BASETYPE_H_

// UC3D 基础类型/宏（UC3D_BEGIN/ucHANDLE/RTTI_DECLARE 等）
#include "uc3d.h"

// 判断是否在 Unreal Engine 环境中（用于条件编译）
#if defined(UE_BUILD_DEVELOPMENT) || defined(UE_BUILD_DEBUG) || defined(UE_BUILD_SHIPPING) || defined(WITH_EDITOR)
	#define UC_UNREAL_ENGINE 1
#else
	#define UC_UNREAL_ENGINE 0
#endif

#endif // _UC_UCUNREAL3D_BASETYPE_H_