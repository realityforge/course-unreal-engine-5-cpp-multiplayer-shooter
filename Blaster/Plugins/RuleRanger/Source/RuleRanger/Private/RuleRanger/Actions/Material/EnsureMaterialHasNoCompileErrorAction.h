/*
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#pragma once

#include "CoreMinimal.h"
#include "RuleRangerAction.h"
#include "EnsureMaterialHasNoCompileErrorAction.generated.h"

UENUM()
enum class EFeatureLevel
{
    /** Feature level defined by the core capabilities of OpenGL ES3.1 & Metal/Vulkan. */
    ES3_1,

    /**
     * Feature level defined by the capabilities of DX11 Shader Model 5.
     *   Compute shaders with shared memory, group sync, UAV writes, integer atomics
     *   Indirect drawing
     *   Pixel shaders with UAV writes
     *   Cubemap arrays
     *   Read-only depth or stencil views (eg read depth buffer as SRV while depth test and stencil write)
     * Tessellation is not considered part of Feature Level SM5 and has a separate capability flag.
     */
    SM5,

    /**
     * Feature level defined by the capabilities of DirectX 12 hardware feature level 12_2 with Shader Model 6.5
     *   Raytracing Tier 1.1
     *   Mesh and Amplification shaders
     *   Variable rate shading
     *   Sampler feedback
     *   Resource binding tier 3
     */
    SM6
};

/**
 * Action to check that the material does not have a compile error.
 */
UCLASS(DisplayName = "Ensure Material has no compile errors")
class RULERANGER_API UEnsureMaterialHasNoCompileErrorAction final : public URuleRangerAction
{
    GENERATED_BODY()

    /** The feature level to check. */
    UPROPERTY(EditAnywhere)
    EFeatureLevel FeatureLevel{ EFeatureLevel::SM6 };

    /** Should the material generate an error if it is empty? */
    UPROPERTY(EditAnywhere)
    bool bErrorIfEmpty{ true };

public:
    virtual void Apply_Implementation(URuleRangerActionContext* ActionContext, UObject* Object) override;

    virtual UClass* GetExpectedType() override;
};
