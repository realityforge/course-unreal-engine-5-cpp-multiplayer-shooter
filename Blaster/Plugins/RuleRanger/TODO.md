# TODO

Shorthand notes of where to go next with this experiment:

* Ensure Matchers can be written in Blueprints
* Add rules so that we can enforce constraints about directories.
  * Rules stored in DataTable.
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
* Add check to ensure that every Blueprint (that inherits from a specific class?) has a Thumbnail specified
* Add check to ensure every parameter in a Material has a group/description specified.
* Add check to unreal that verifies every Niagara system has an "Effect Type" associated with it.
* Add validation to ensure every texture is marked with a TextureGroup
* Add metadata to assets to track origin. (i.e. What license and where was it sourced from)
* Add tool to record licenses for assets in MetaData
    * A DataTable that indicates the source (Marketplace project name? Some other project tag), description and license
    * Add metadata to assets to link to this table
    * Add report to report on licenses of all assets
    * Add action to apply license to entire folder or individual assets or selected assets/folders
    * Add RuleRanger validation to ensure license is set
* Rather than statically defining `ETextureSubType`, define it in a table
  * Merge `FTextureSubTypeNameConvention` into it
  * Add component count for each SubType
  * Add default/allowed modes (Color, Linear Color, Alpha etc)
  * Add default/allowed compressions
* Look at https://github.com/benui-dev/UE-BUIValidator/blob/main/Source/BUIValidator/Public/BUIValidatorSettings.h and see if it offers anything other than what is already planned.

Add Action that applies a layout for textures based on metadata and types? Maybe something like:
Possible Directory layout:
 `Characters/[CharacterName]/Animations`
 `Characters/[CharacterName]/Materials`
 `Characters/[CharacterName]/Materials/Instances`
 `Characters/[CharacterName]/Materials/Functions`
 `Characters/[CharacterName]/Materials/Layers`
 `Characters/[CharacterName]/Rigs`
 `Characters/[CharacterName]/Textures`
