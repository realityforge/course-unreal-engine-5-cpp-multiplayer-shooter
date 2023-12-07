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

class FRuleRangerUtilities
{
public:
    /** Method to perform the rename of an asset. */
    static bool RenameAsset(UObject* Object, const FString& NewName);

    /**
     * Add the types of an object into a classes array, starting with the most specific and moving to least specific at
     * the end of the list. "Alternative" type hierarchies such as via Blueprint type system are also incorporated into
     * the list.
     */
    static void CollectTypeHierarchy(const UObject* Object, TArray<UClass*>& Classes);

    /**
     * Return true if the specified Object is an instance of the specified Class. This also traverses alternate
     * type hierarchies such as Blueprint type system.
     *
     * @param Object The Object to Test
     * @param Class The Class To Test For
     * @return true if the specified object is instance of the specified class.
     */
    static bool IsA(const UObject* Object, const UClass* Class);

    /**
     * Collect the "parent" instances of an object into an objects array, starting with the passed in class and moving
     * to parent objects at the end of the list. This is to support the scenario where a MaterialInstance may have a
     * chain of parent Material instances before being parented by a Material.
     */
    static void CollectInstanceHierarchy(UObject* Object, TArray<UObject*>& Instances);
};
