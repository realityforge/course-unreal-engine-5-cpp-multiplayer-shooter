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
* Add a mechanism for adding an exclude `DataTable` that will will list assets that will ignore specific rules or all rules for specific assets or all assets or all assets that matchers match?
* Material functions that are Material Layers are prefixed with ML not MF!
* Retargeters should be named as RTG_\[Source\]_To_\[Target\]
* Add ability to add other validators that are invoked on Scan (i.e. no directories without assets)
* Add tool to record licenses for assets in MetaData
    * A DataTable that indicates the source (Marketplace project name? Some other project tag), description and license
    * Add metadata to assets to link to this table
    * Add report to report on licenses of all assets
    * Add action to apply license to entire folder or individual assets or selected assets/folders
    * Add RuleRanger validation to ensure license is set
* Name checks:
  * Add regex per-type for nested names? or global regex for internal names?
  * Ensure that classes (C++ and Blueprint) have a "basename" that ends with Base rather than starts with Base. i.e. Prefer FooBase over BaseFoo
  * Ensure some name patterns (i.e. *Base) require (Abstract) UCLASS specifier
* Material checks:
  * Ensure parameters have group/categories
  * Ensure parameters have descriptions
  * Ensure parameters follow naming convention
  * Ensure that parameters of specific name/type exist on a material ... or a material property? (Useful when using to create dynamic material instance and use strings to match parameters)
* Blueprint checks:
  * Add check to ensure that every Blueprint (that inherits from a specific class?) has a Thumbnail specified
  * Add check that all private Blueprint functions are called
  * Add check that protected Blueprint functions are in non-final blueprint classes?
* Texture checks:
  * Add default/allowed modes (Color, Linear Color, Alpha etc)
  * Add rules for well known texture variants:
    * AORMT
    * "M": Compression = TextureCompressionSettings::TC_Masks or TextureCompressionSettings::TC_Grayscale (Greyscale implies sRGB=true while Masks implies sRGB=false)
  * Other rules:
    * If TextureCompressionSettings::TC_Masks then sRGB = false
    * * If TextureCompressionSettings::TC_Grayscale then sRGB = true (huh?)
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
