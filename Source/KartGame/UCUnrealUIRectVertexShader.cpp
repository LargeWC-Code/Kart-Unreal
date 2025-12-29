/********************************************************************
created:	2025/12/18
filename: 	UCUnrealUIRectVertexShader.cpp

purpose:	UI 矩形渲染 Vertex Shader 实现
*********************************************************************/

#include "UCUnrealUIRectVertexShader.h"
#include "PipelineStateCache.h"
#include "RHIStaticStates.h"

IMPLEMENT_GLOBAL_SHADER(FUCUnrealOverlayRectVS, "/Project/UIRectVertexShader.usf", "MainVS", SF_Vertex);

static FVertexDeclarationRHIRef GUCUnrealOverlayRectVertexDecl;
FVertexDeclarationRHIRef GetUCUnrealOverlayRectVertexDecl()
{
	if (!GUCUnrealOverlayRectVertexDecl.IsValid())
	{
		FVertexDeclarationElementList Elements;
		Elements.Add(FVertexElement(0, STRUCT_OFFSET(FUCUnrealOverlayRectVertex, Position), VET_Float2, 0, sizeof(FUCUnrealOverlayRectVertex)));
		Elements.Add(FVertexElement(0, STRUCT_OFFSET(FUCUnrealOverlayRectVertex, UV),       VET_Float2, 1, sizeof(FUCUnrealOverlayRectVertex)));
		Elements.Add(FVertexElement(0, STRUCT_OFFSET(FUCUnrealOverlayRectVertex, Color),    VET_Color,  2, sizeof(FUCUnrealOverlayRectVertex)));
		GUCUnrealOverlayRectVertexDecl = RHICreateVertexDeclaration(Elements);
	}
	return GUCUnrealOverlayRectVertexDecl;
}




