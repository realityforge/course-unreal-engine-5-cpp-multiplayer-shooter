# TODO

Shorthand notes of where to go next with this experiment:

* Add a mechanism for adding an exclude `DataTable` that will will list assets that will ignore specific rules or all rules for specific assets or all assets or all assets that matchers match?
* Material functions that are Material Layers are prefixed with ML not MF!
* Retargeters should be named as RTG_\[Source\]_To_\[Target\]
* Add rules for "Level (World Partition)" => prefix: `L_`, suffix: `_WP`
* Add check to ensure that every Blueprint (that inherits from a specific class?) has a Thumbnail specified
* Add check to ensure every parameter in a Material has a group/description specified.
* Add check to unreal that verifies every Niagara system has an "Effect Type" associated with it.
* Add validation to ensure every texture is marked with a TextureGroup
* Add metadata to assets to track origin. (i.e. What license and where was it sourced from)
* Rather than statically defining `ETextureSubType`, define it in a table
  * Merge `FTextureSubTypeNameConvention` into it
  * Add component count for each SubType
  * Add default/allowed modes (Color, Linear Color, Alpha etc)
  * Add default/allowed compressions

Add Action that applies a layout for textures based on metadata and types? Maybe something like:
Possible Directory layout:
 `Characters/[CharacterName]/Animations`
 `Characters/[CharacterName]/Materials`
 `Characters/[CharacterName]/Materials/Instances`
 `Characters/[CharacterName]/Materials/Functions`
 `Characters/[CharacterName]/Materials/Layers`
 `Characters/[CharacterName]/Rigs`
 `Characters/[CharacterName]/Textures`
