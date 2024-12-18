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
#include "RuleRangerMatcher.h"
#include "UObject/Object.h"
#include "PathLengthMatcher.generated.h"

/**
 * Matcher that returns true if object has path that is above maximum length.
 */
UCLASS()
class RULERANGER_API UPathLengthMatcher final : public URuleRangerMatcher
{
    GENERATED_BODY()

    /** The length after which the matcher will match. */
    UPROPERTY(EditAnywhere)
    int32 MaxPathLength{ 140 };

public:
    bool Test_Implementation(UObject* Object) const override;
};
