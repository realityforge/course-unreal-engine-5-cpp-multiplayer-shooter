# TODO

Shorthand notes of where to go next with this experiment:

* Consider incorporating naming conventions like Lyra. Although this may be a per-project thing.
  - Phase_	-> Game Phase Abilities
  - AbilitySet_ -> Ability Set
  - InputData_ -> (Lyra) Input Config

* Change the way that Texture conventions are driven so that suffix is driven by variant in NameConventions. Also change `DT_DefaultTextureConventions` so that ehy are driven by variants
* Change naming convention so that if a variant is not specified but the type has variants and the name has a suffix that matches a variant then auto-set variant (i.e. importing a texture named `T_Blah_BC` will automatically set `BaseTexture`). Maybe add a flag per NameConvention row that allows auto-setting variant?

* RuleRanger should ALSO invoke validation on each object ... otherwise problems highlighted via IsDataValid will be missed. Note that by default validation also invokes RuleRanger so we will need to guard this scenario and avoid duplicate invocations

* Remove the "Actor/*" actions and replace with specific `IsDataValid()` implementations in each project. Maybe we expose the functionality as static utility functions.
* Update NameConventions action to also grab data from context
* Add concept of ExclusionSet that is a set of Rules and RuleSets that can be excluded? So we can have a common exclude for all ThirdParty stuff across projects?
* Consider using a separate Plugin+module for Niagara checks so we can have a clean dependency tree.
* Add rules so that we can enforce constraints about asset referencing.
  * Rules stored in DataTable.
  * What color should the folder appear in the Content Browser?
  * Rules can apply to a folder or all subfolders.
  * Folders may be explicitly named or declared as named patterns (i.e. `MyGame/Characters/*/Animations`)
  * Rules about max number of elements in directory
  * Rules indicate which folders you are allowed to reference
  * Rules have a priority order
* Experiment rolling back treating rules/matchers/actions as assets and instead allow them as Blueprint classes
* Support writing rules as blueprints and thus inlining matcher/action functionality into a single call?
* Retargeters should be named as RTG_\[Source\]_To_\[Target\]
* Add the ability to add other validators that are invoked on Scan (i.e. no directories without assets)
* Add tool to record licenses for assets in MetaData
    * A DataTable that indicates the source (Marketplace project name? Some other project tag), description and license
    * Add metadata to assets to link to this table
    * Add report to report on licenses of all assets
    * Add action to apply license to entire folder or individual assets or selected assets/folders
    * Add RuleRanger validation to ensure license is set
* Re-enable `RRR_Texture_AlertOnMissingVariant` when exclusion rules in places
* Common checks:
  * Support RuleRangerRequiredIf="SomeProperty" to meta which will require that the property is non-null if the referenced property is non-null.
* Possibly extract "Config Overrides" to allow us to configure all the settings in a central place?
  * Is this possible? or do we continue to configure on a rule-by-rule basis. Perhaps we could introspect rule sets and automagically build a config panel.
* BlackBoard:
  * Add check to ensure keys follow naming convention
  * Add check to ensure entries are categorized if threshold count reached
  * Add check to ensure entries have descriptions if threshold count reached
* Name checks:
  * Add regex per-type for nested names? or global regex for internal names?
  * Ensure that classes (C++ and Blueprint) have a "basename" that ends with Base rather than starts with Base. i.e. Prefer FooBase over BaseFoo
  * Ensure some name patterns (i.e. *Base) require (Abstract) UCLASS specifier
* Blueprint checks:
  * Add flag to skip description and categorization requirements for private variables/functions
  * Blueprint Macros also have function rules applied. Should we restrict these rules as Macros are mostly internal? or maybe not as used by child classes?
* InputMappingContext:
  * Ensure Description is specified
* InputAction:
  * Ensure ActionDescription is specified
* Blueprint Enumerations:
  * Add requirement that Enumerations are documented
  * Add requirement that Enumerators are documented
* Add the ability to add an exception for a violation from within the MessageLog. Think "Click to exclude."
* Material Function checks:
  * Ensure that names for inputs match a pattern
  * Ensure that descriptions for inputs present
  * Ensure that the override name matches a pattern if exposed to library
* Material Parameter:
  * Add check to ensure that a material parameter exists matches a certain value. So some options can be set during development
    but changed at shipping time. For example, Brightness controls set during the development phase should be changed to adjusting brightness
    on Texture import when you ship.
* Material checks:
  * Ensure that parameters of a specific name/type exist on a material ... or a material property? (Useful when using to create dynamic material instance and use strings to match parameters)
  * Ensure that materials associated with specific Skeletons that have Material type animation curves, have materials with parameters that match.
  * Ensure that there are no dangling nodes in material
  * If the material does not specify roughness value/texture then suggest enabling fully rough (as long as metadata "DefaultRoughnessAllowed" metadata key set)
  * If the material does not specify a normal map, then disable "Tangent Space Normals"
  * Add rules to check which material usages are set
  * Ensure that all Texture Sampler nodes have set the setting "Sampler Source" to "Shared: Wrap" or "Shared: Clamp" if
    the associated Texture has the same sampler settings as the world. Use `FGLTFTextureUtilities::GetDefaultFilter(LODGroup)`
    to derive textures effective filter setting. Or just copy the code:
```c++
TextureFilter FGLTFTextureUtilities::GetDefaultFilter(TextureGroup LODGroup)
{
	const UTextureLODSettings* TextureLODSettings = UDeviceProfileManager::Get().GetActiveProfile()->GetTextureLODSettings();
	const ETextureSamplerFilter Filter = TextureLODSettings->GetTextureLODGroup(LODGroup).Filter;

	switch (Filter)
	{
		case ETextureSamplerFilter::Point:             return TF_Nearest;
		case ETextureSamplerFilter::Bilinear:          return TF_Bilinear;
		case ETextureSamplerFilter::Trilinear:         return TF_Trilinear;
		case ETextureSamplerFilter::AnisotropicPoint:  return TF_Trilinear; // A lot of engine code doesn't result in nearest
		case ETextureSamplerFilter::AnisotropicLinear: return TF_Trilinear;
		default:                                       return TF_MAX;
	}
}
```
* Can we make an asset validator that makes sure that certain assets are not baked/packaged?
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
  * Add "_FunctionLibrary" to ControlRigs that are just collections of functions and no ForwardSolve? Do it via the variants mechanism?
* Blueprint checks:
  * Add check to ensure that every Blueprint (that inherits from a specific class?) has a Thumbnail specified
  * Add check that all private Blueprint functions are called
* AnimationMontage:
  * Section names/orders: https://github.com/TheEmidee/UEDataValidationExtensions/blob/d64320e698e00d8269053edcc7b9f058061c64aa/Source/DataValidationExtensionsEditor/Private/DVEAnimMontageValidationHelpers.cpp
* Animation Sequence:
  * Ensure animation sequence has a certain FPS
  * Ensure animation sequence contains curve data for specific curves
* Skeletal Mesh:
  * Ensure that an AnimationSlot Group and/or name exists
  * Ensure that a Curve with name/type exists
  * Ensure that a Curve is attached/not attached to particular bones
  * Ensure naming conventions for AnimationSlot Group/Name
* Skeleton:
  * Ensure that an AnimationSlot Group and/or name exists
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
