---
publish: true
---

# Macros

Some of these macros are only functional while in the editor, and some work on both.\
Some of these macros are meant for .cpp files and placed in the global scope. The rest can be placed inside of regular code.

```cpp
Omni_InsightsTrace_Str("My insights trace")
```

* Wrapper for `TRACE_CPUPROFILER_EVENT_SCOPE_STR`

```cpp
Omni_InsightsTrace()
```

* Wrapper for `TRACE_CPUPROFILER_EVENT_SCOPE(__FUNCTION__)`. If the function this macro is inside of is called `Bar` and the class is named `Foo`, this will display as `Foo::Bar` inside Unreal Insights

```cpp
Omni_OnModuleStarted("AIF_Mass")
{
    UE_LOG(LogTemp, Warning, TEXT("AIF_Mass module has started"));
}
```

* This macro will trigger the code beneath it whenever the specified module is started. You can find module names inside of a plugins `.uplugin` file.
* Place in .cpp file and in global scope.
* You can use the engine's `UE_MODULE_NAME` macro if you aren't waiting for a separate module to start.

```cpp
Omni_SetClassIcon(
AgentIntelligenceFramework, /*Plugin name*/
AgentGameplayEffectConfig, /*Class name (No U prefix)*/
 EffectIconSlate) /*Name of SVG inside of plugins resource folder*/
```

* Automatically assigns an SVG (must be inside the plugins `Resource` folder) for a class as the class icon and class thumbnail.
* Place in .cpp file and in global scope.
