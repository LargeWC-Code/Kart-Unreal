/********************************************************************
created:	2025/12/18
filename: 	UCUnrealUIRectPixelShader.h

purpose:	UI 矩形渲染 Pixel Shader
使用着色器变体避免运行时条件判断，提高性能
*********************************************************************/
#pragma once

#include "CoreMinimal.h"
#include "GlobalShader.h"
#include "ShaderParameterStruct.h"
#include "ShaderParameterMacros.h"
#include "ShaderPermutation.h"

// 着色器变体参数
class FUCUnrealOverlayRectPS_UseTexture : SHADER_PERMUTATION_BOOL("USE_TEXTURE");
class FUCUnrealOverlayRectPS_IsA8Format : SHADER_PERMUTATION_BOOL("IS_A8_FORMAT");

using FUCUnrealOverlayRectPSPermutationDomain = TShaderPermutationDomain<
	FUCUnrealOverlayRectPS_UseTexture,
	FUCUnrealOverlayRectPS_IsA8Format
>;

class FUCUnrealOverlayRectPS : public FGlobalShader
{
	DECLARE_GLOBAL_SHADER(FUCUnrealOverlayRectPS);
	SHADER_USE_PARAMETER_STRUCT(FUCUnrealOverlayRectPS, FGlobalShader);
	using FPermutationDomain = FUCUnrealOverlayRectPSPermutationDomain;
	
	BEGIN_SHADER_PARAMETER_STRUCT(FParameters, )
		SHADER_PARAMETER_TEXTURE(Texture2D, UITexture)
		SHADER_PARAMETER_SAMPLER(SamplerState, UITextureSampler)
	END_SHADER_PARAMETER_STRUCT()

	static bool ShouldCompilePermutation(const FGlobalShaderPermutationParameters& Parameters)
	{
		FPermutationDomain PermutationVector(Parameters.PermutationId);
		
		// 如果没有使用纹理，则不需要 A8 格式变体
		if (!PermutationVector.Get<FUCUnrealOverlayRectPS_UseTexture>())
		{
			// 无纹理时，A8 格式标志必须为 false
			if (PermutationVector.Get<FUCUnrealOverlayRectPS_IsA8Format>())
			{
				return false;
			}
		}
		
		return true;
	}
};



