# TODO

Shorthand notes of where to go next with this experiment:

* Add a mechanism for adding an exclude `DataTable` that will will list assets that will ignore specific rules or all rules for specific assets or all assets or all assets that matchers match?
* Material functions that are Material Layers are prefixed with ML not MF!
* Retargeters should be named as RTG_\[Source\]_To_\[Target\]
* Make a rule for handling texture extensions
  * Base Color: _BC
  * Ambient Occlusion: _AO
  * Roughness: _R
  * Specular: _S
  * Metallic: _M
  * Emissive: _E
  * Mask: _Mask
  * Flow Map: _F
  * Height: _H
  * Displacement: _D
  * Light Map: _L
  * Alpha/Opacity: _A
  * Packed: Use texture type in RGBA order. e.g., ARMH (Ambient Occlusion, Roughness, Metallic, Height)
* Add rules for "Level (World Partition)" => prefix: `L_`, suffix: `_WP`
* Add check to ensure that every Blueprint (that inherits from a specific class?) has a Thumbnail specified
* Add check to ensure every parameter in a Material has a group/description specified.
* Add check to unreal that verifies every Niagara system has an "Effect Type" associated with it.
* Add check to unreal that verifies that "Masks" compression setting set for textures of the _AORMT or similar textures
* Add check to unreal that verifies that "Normalmap" compression setting set for textures of the _N or similar textures
* Add check to ensure sRGB set only BaseColor textures where expected?
* Add metadata to assets to track origin. (i.e. What license and where was it sourced from)

Add Action that applies a layout for textures based on metadata and types? Maybe something like:
Possible Directory layout:
 `Characters/[CharacterName]/Animations`
 `Characters/[CharacterName]/Materials`
 `Characters/[CharacterName]/Materials/Instances`
 `Characters/[CharacterName]/Materials/Functions`
 `Characters/[CharacterName]/Materials/Layers`
 `Characters/[CharacterName]/Rigs`
 `Characters/[CharacterName]/Textures`
