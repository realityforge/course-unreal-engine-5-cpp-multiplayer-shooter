﻿/*
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
#include "SourcePathMatcherBase.h"
#include "UObject/Object.h"
#include "SourceFilenameEndsWithMatcher.generated.h"

/**
 * Match the filename that the asset was imported from if it ends with the specified text.
 */
UCLASS()
class RULERANGER_API USourceFilenameEndsWithMatcher final : public USourcePathMatcherBase
{
    GENERATED_BODY()

    /** The text to match. */
    UPROPERTY(EditAnywhere)
    FString Text;

protected:
    virtual bool Match(UObject* Object, const FString& SourcePath, bool bInCaseSensitive) override;
};
