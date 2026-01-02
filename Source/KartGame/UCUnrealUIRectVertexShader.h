/********************************************************************
created:	2025/12/18
filename: 	UCUnrealUIRectVertexShader.h

purpose:	UI 矩形渲染 Vertex Shader
*********************************************************************/
#pragma once

#include "CoreMinimal.h"
#include "GlobalShader.h"
#include "ShaderParameterStruct.h"

// UI 矩形顶点数据结构（与 UCUnreal3DDevice::DrawRectsBatch 中的 FUCUnrealUIRectVertex 匹配）
struct FUCUnrealOverlayRectVertex
{
	FVector2f Position; // ATTRIBUTE0
	FVector2f UV;       // ATTRIBUTE1
	FColor Color;       // ATTRIBUTE2
};

class FUCUnrealOverlayRectVS : public FGlobalShader
{
	DECLARE_GLOBAL_SHADER(FUCUnrealOverlayRectVS);
	SHADER_USE_PARAMETER_STRUCT(FUCUnrealOverlayRectVS, FGlobalShader);
	
	BEGIN_SHADER_PARAMETER_STRUCT(FParameters, )
	END_SHADER_PARAMETER_STRUCT()

	static bool ShouldCompilePermutation(const FGlobalShaderPermutationParameters& Parameters)
	{
		return true;
	}
};

// 获取顶点声明
FVertexDeclarationRHIRef GetUCUnrealOverlayRectVertexDecl();






