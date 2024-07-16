# TODO

Shorthand notes of where to go next with this experiment:

* Consider using a separate Plugin+module for Niagara checks so we can have a clean dependency tree.
* Add rules so that we can enforce constraints about asset referencing.
  * Rules stored in DataTable.
  * What color should the folder appear in Content Browser!
  * Rules can apply to a folder or all sub-folders.
  * Folders may be explicitly named or declared as named patterns (i.e. `MyGame/Characters/*/Animations`)
  * Rules about max number of elements in directory
  * Rules indicate which folders you are allowed to reference
  * Rules have a priority order
* Experiment rolling back treating rules/matchers/actions as assets and instead allow them as Blueprint classes
* Support writing rules as blueprints and thus inlining matcher/action functionality into a single call?
* Add a mechanism for adding an exclude `DataTable` that will list assets that will ignore specific rules or all rules for specific assets or all assets or all assets that matchers match?
* Material functions that are Material Layers are prefixed with ML not MF!
* Retargeters should be named as RTG_\[Source\]_To_\[Target\]
* Add ability to add other validators that are invoked on Scan (i.e. no directories without assets)
* Add tool to record licenses for assets in MetaData
    * A DataTable that indicates the source (Marketplace project name? Some other project tag), description and license
    * Add metadata to assets to link to this table
    * Add report to report on licenses of all assets
    * Add action to apply license to entire folder or individual assets or selected assets/folders
    * Add RuleRanger validation to ensure license is set
* Re-enable `RRR_Texture_AlertOnMissingVariant` when exclusion rules in places
* Common checks:
  * Support RuleRangerRequiredIf="SomeProperty" to meta which will require that the property is non-null if the referenced property is non-null.
* Name checks:
  * Add regex per-type for nested names? or global regex for internal names?
  * Ensure that classes (C++ and Blueprint) have a "basename" that ends with Base rather than starts with Base. i.e. Prefer FooBase over BaseFoo
  * Ensure some name patterns (i.e. *Base) require (Abstract) UCLASS specifier
* Material checks:
  * Ensure parameters have group/categories
  * Ensure parameters have descriptions
  * Ensure parameters follow naming convention
  * Ensure that parameters of specific name/type exist on a material ... or a material property? (Useful when using to create dynamic material instance and use strings to match parameters)
  * Ensure that materials associated with Skeletons that have Material type animation curves, have materials with parameters that match.
  * Ensure that there are no gangling nodes in material
  * If the material does not specify roughness value/texture then suggest enabling fully rough (as long as metadata "DefaultRoughnessAllowed" metadata key set)
  * If the material does not specify normal map then disable "Tangent Space Normals"
  * Add rule to enforce setting of "Automatically set usage" setting
  * Add rules to check which materials usages are set
* Native Class checks:
  * Add naming convention check for native classes/structs
  * Add naming convention check for native classes/structs properties and functions
  * Add check to ensure classes are abstract (So if meta=(IsBlueprintBase) then all concrete ones are all in blueprints?)
  * Add check to ensure DisplayName set
  * Add check to ensure Descriptions for types
  * Add check to ensure properties are either DefaultsOnly etc
  * Add matcher so can match native classes from specific modules
  * Add matcher so can match meta=(IsBlueprintBase=true)
* ControlRig checks:
  * Ensure functions have groups/categories and descriptions
  * Ensure variables have groups/categories and descriptions
  * Ensure that there are no loose nodes
  * Ensure that Blueprint is compiled
  * Add "_FunctionLibrary" to ControlRigs that are just collections of functions and no ForwardSolve? Do it via Variants mechanism
* Blueprint checks:
  * Add check to ensure that every Blueprint (that inherits from a specific class?) has a Thumbnail specified
  * Add check that all private Blueprint functions are called
* AnimationMontage:
  * Section names/orders: https://github.com/TheEmidee/UEDataValidationExtensions/blob/d64320e698e00d8269053edcc7b9f058061c64aa/Source/DataValidationExtensionsEditor/Private/DVEAnimMontageValidationHelpers.cpp
* Animation Sequence:
  * Ensure animation sequence has a certain FPS
  * Ensure animation sequence contains curve data for specific curves
* Skeletal Mesh:
  * Ensure that a AnimationSlot Group and/or name exists
  * Ensure that a Curve with name/type exists
  * Ensure that a Curve is attached/not attached to particular bones
  * Ensure naming conventions for AnimationSlot Group/Name
* Skeleton:
  * Ensure that a AnimationSlot Group and/or name exists
  * Ensure that a Curve with name/type exists
  * Ensure that a Curve is attached/not attached to particular bones
  * Ensure naming conventions for AnimationSlot Group/Name
* Niagara checks:
  * Ensure every Niagara system has an "Effect Type" associated with it.
* Texture type checks:
  * "_R": Roughness
  * "_AO": AmbientOcclusion
  * "_H": Height
  * "_F": FlowMap
  * "_D": Displacement
  * "_L": LightMap
* Static Mesh checks:
  * Ensure StaticMesh has valid/non-overlapping Lightmap UVs
  * Ensure StaticMesh has NO Lightmap UVs (when not using lightmass)
  * Ensure StaticMesh has UVs in Channel0 (for when material only uses world space coordinates)

Add Action that applies a layout for textures based on metadata and types? Maybe something like:
Possible Directory layout:
 `Characters/[CharacterName]/Animations`
 `Characters/[CharacterName]/Materials`
 `Characters/[CharacterName]/Materials/Instances`
 `Characters/[CharacterName]/Materials/Functions`
 `Characters/[CharacterName]/Materials/Layers`
 `Characters/[CharacterName]/Rigs`
 `Characters/[CharacterName]/Textures`
