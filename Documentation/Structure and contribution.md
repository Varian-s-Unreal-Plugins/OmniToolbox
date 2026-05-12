---
publish: true
---

# Structure and contribution

`OmniToolbox` comprises of 2 modules:

* OmniToolbox
  * Contains various helper function libraries and other things that are only relevant to runtime based logic.
  * Runtime module

* OmniToolboxEditor
  * Contains developer settings, thumbnail renderer, content browser modification and many other  editor-only based changes.
  * Editor module

## Creating plugins with OmniToolbox

### Copyright

`OmniToolbox` is published under the MIT license and everyone has permission to fork it, copy the code, or publish their own version.

### Minimal implementation

By minimal, I mean no gameplay features and having the possibility of using your plugin without having OmniToolbox installed. This can be done like so:

1. Add this code to your modules build.cs file:

```cpp
if(Plugins.GetPlugin("OmniToolbox") != null)
{
    PublicDefinitions.Add("OMNITOOLBOX_INSTALLED=1");
    PublicDependencyModuleNames.Add("OmniToolbox");
    PublicDependencyModuleNames.Add("OmniToolboxRuntime");
}
else
{
    PublicDefinitions.Add("OMNITOOLBOX_INSTALLED=0");
}
```

2. Wrap your code with `#if OMNITOOLBOX_INSTALLED`

```cpp
#if OMNITOOLBOX_INSTALLED
Omni_SetClassIcon(AgentIntelligenceFramework, EntityController, EntityControllerSlate)
#endif
```

```cpp
UTexture2D* GetThumbnail_Implementation()
{
    #if OMNITOOLBOX_INSTALLED
    if(FEffectDataUIIcon* EffectIcon = GetStructPtrFromInstancedStructArray<FEffectDataUIIcon, FAgentGameplayEffectData>(EffectDefinition.Effect.Data))
    {
       return EffectIcon->Icon.LoadSynchronous();
    }
    #endif

    return nullptr;
}
```

* Blueprints do NOT have the ability to filter code like this. If you add any blueprint code to your project that originates from `OmniToolbox`, you now have a hard dependency on `OmniToolbox`.
* Instead of checking if `OmniToolbox` is installed, I recommend checking by version

#### Asset details interface

From my testing, there seems to be an issue with Unreal recognizing interfaces while in the editor when it comes to interfaces wrapped with preprocessor macros

```cpp
UCLASS(Abstract, Blueprintable)
class AIF_MASS_API AEntityController : public AController
#if OMNITOOLBOX_INSTALLED
    , public II_AssetDetails
#endif
```

The code above will compile and during runtime will continue to work normally, I've even validated this with my other plugins and using other interfaces. But while in the editor, it will not work. Unfortunately, this interface must work while in the editor.

If you want to make assets utilize the interface, but still be able to compile the plugin without `OmniToolbox` installed, you will have to do this evil hack:

```cpp
#if !OMNITOOLBOX_INSTALLED
class II_AssetDetails {};
#endif
UCLASS(Abstract, Blueprintable)
class AIF_MASS_API AEntityController : public AController, public II_AssetDetails
```

This is essentially just creating a new class with the same name if the plugin isn't installed. It's not ideal, but it's the best we can do for now.

## Making contributions

`OmniToolbox` follows 3 philosophies:

1. Simplify the lives of developers (for example; utility functions, systems or [macro's](https://varian.gitbook.io/varian-docs/omnitoolbox/macros) to reduce repetitive work, such as giving assets custom icons)
2. Adding abstract features (for example; [float providers](https://varian.gitbook.io/varian-docs/omnitoolbox/runtime/float-providers))
3. Exposing C++ logic to blueprints (for example; gameplay tags have many functions that for some reason Epic have not exposed to blueprint)

Any contribution that wants to be accepted should try and follow these goals.

Examples of contributions that would get declined:

* Combat system using GAS
  * A combat system is not abstract enough (not all games have combat) and not everyone wants a dependency on GAS
* Prototyping assets, such as materials, widgets, etc
  * This can quickly clutter up people's projects and asset validators in some projects might not be happy with these assets.
  * While useful, I feel like a separate plugin, specifically designed for prototyping material, should be developed. I might create one to speed up development of my example projects, but there's no guarantee on that.

In some cases, it might be better for some people to simply make a pull/feature request for the engine itself.
