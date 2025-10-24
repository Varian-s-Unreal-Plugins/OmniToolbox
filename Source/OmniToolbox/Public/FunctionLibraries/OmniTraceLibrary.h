// Copyright (C) Varian Daemon 2025. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Engine/EngineTypes.h"
#include "OmniTraceLibrary.generated.h"

struct FTraceSetting;

// Define a struct for trace debugging options
USTRUCT(BlueprintType)
struct FTraceDebug
{
    GENERATED_BODY()

    // Whether to enable or disable debug drawing
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
    bool bEnableDebug = false;

    // Color to use if the trace hit something
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
    FLinearColor HitColor = FLinearColor::Green;

    // Color to use if the trace hit nothing
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
    FLinearColor MissColor = FLinearColor::Red;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
    FLinearColor OverlapColor = FLinearColor::Yellow;

    // Whether to draw persistent lines
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
    bool bPersistentLines = false;

    // Lifetime of the debug lines
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
    float LifeTime = 2.5f;

    // Thickness of the debug lines
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
    float Thickness = 10.0f;

    /**Tag used to provide extra information or filtering for debugging of the trace (e.g. Collision Analyzer)*/
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
    FName TraceTag;

    FVector Start;
    FVector End;
    FQuat Rotation;
};

UENUM(BlueprintType)
enum EAsyncTraceResultType
{
    MultiResult,
    SingleResult,
    /**Does not return any valid hit results, but will return
     * one index to indicate *something* was hit.
     * This is slightly faster than the other two options,
     * but highly situational. This is just a simple test to
     * see if this trace hit *something* but returns no data
     * as to what was hit*/
    TestResult,
};

USTRUCT(BlueprintType)
struct FTraceChannelAndResponse
{
    GENERATED_BODY()

    UPROPERTY(Category = "", EditAnywhere, BlueprintReadWrite)
    TEnumAsByte<ECollisionChannel> Channel = ECC_WorldStatic;

    UPROPERTY(Category = "", EditAnywhere, BlueprintReadWrite)
    TEnumAsByte<ECollisionResponse> Response = ECR_Block;
};

USTRUCT(BlueprintType)
struct FTraceChannelAndResponseContainer
{
    GENERATED_BODY()

    UPROPERTY(Category = "", EditAnywhere, BlueprintReadWrite)
    TArray<FTraceChannelAndResponse> ChannelsAndResponses;
};

USTRUCT(BlueprintType)
struct FMassTraceResult
{
    GENERATED_BODY()

    UPROPERTY(Category = "", EditAnywhere, BlueprintReadOnly)
    FGuid TraceGUID;

    UPROPERTY(Category = "", EditAnywhere, BlueprintReadOnly)
    TArray<FHitResult> HitResults;
};

DECLARE_DYNAMIC_DELEGATE_OneParam(FAsyncTraceResultDelegate, const TArray<FHitResult>&, HitResult);
DECLARE_DYNAMIC_DELEGATE_OneParam(FMassTraceResultDelegate, FMassTraceResult, TraceResult);

/**
 * 
 */
UCLASS()
class OMNITOOLBOX_API UOmniTraceLibrary : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()

public:

    /**Create a response container using the settings from a collision profile.
     * You can customize this container by assigning responses to specific channels
     * through the @CustomChannelResponses struct.*/
    UFUNCTION(Category = "Trace Helpers", BlueprintCallable, BlueprintPure)
    static FCollisionResponseContainer CreateResponseContainerFromProfile(
        UPARAM(Meta=(GetOptions="Engine.KismetSystemLibrary.GetCollisionProfileNames")) FName ProfileName,
        FTraceChannelAndResponseContainer CustomChannelResponses = FTraceChannelAndResponseContainer());

    UFUNCTION(Category = "Trace Helpers", BlueprintCallable, BlueprintPure)
    static FCollisionResponseContainer CreateResponseContainer(FTraceChannelAndResponseContainer CustomChannelResponses);
    
    /**Perform a line trace.
     * @ResultType Is this a multi, single or test trace?
     * @CustomChannelResponses Allows you to assign unique responses to specific channels.
     * You will want to call CreateResponseContainerFromProfile() first, then modify that
     * and pass the final result to this function.*/
    UFUNCTION(BlueprintCallable, Category = "Helpers Library|Trace Helpers", meta = (WorldContext = "WorldContextObject", DefaultToSelf = "WorldContextObject"))
    static TArray<FHitResult> LineTrace(UObject* WorldContextObject, const FVector& Start, const FVector& End, TEnumAsByte<EAsyncTraceResultType> ResultType,
        const FCollisionResponseContainer& CustomChannelResponses, const TArray<AActor*>& IgnoredActors, bool TraceComplex = false, FTraceDebug DebugOptions = FTraceDebug());

    /**Start an async line trace.
     * @ResultType Async traces do not separate the test, multi and single functions.
     * This allows you to control whether the trace is just a test, returns an array or a single result.
     * Test is always the fastest, but the hit result will be empty. The only way to tell if it hit something
     * is to test if the hit result has an index in it. But the data inside those hit results will be nothing.*/
    UFUNCTION(BlueprintCallable, Category = "Helpers Library|Trace Helpers", meta = (WorldContext = "WorldContextObject", DefaultToSelf = "WorldContextObject"))
    static void AsyncLineTrace(UObject* WorldContextObject, const FVector& Start, const FVector& End, TEnumAsByte<EAsyncTraceResultType> ResultType,
        const TArray<AActor*>& IgnoredActors, const FCollisionResponseContainer& CustomChannelResponses, FAsyncTraceResultDelegate OnTraceCompleted, bool TraceComplex = false, FTraceDebug DebugOptions = FTraceDebug());

    /**Perform a sphere trace.
     * @ResultType Is this a multi, single or test trace?
     * @CustomChannelResponses Allows you to assign unique responses to specific channels.
     * You will want to call CreateResponseContainerFromProfile() first, then modify that
     * and pass the final result to this function.*/
    UFUNCTION(BlueprintCallable, Category = "Helpers Library|Trace Helpers", meta = (WorldContext = "WorldContextObject", DefaultToSelf = "WorldContextObject"))
    static TArray<FHitResult> SphereTrace(UObject* WorldContextObject, const FVector& Start, const FVector& End, const float Radius, TEnumAsByte<EAsyncTraceResultType> ResultType,
        const FCollisionResponseContainer& CustomChannelResponses, const TArray<AActor*>& IgnoredActors, bool TraceComplex = false, FTraceDebug DebugOptions = FTraceDebug());

    /**Start an async sphere trace.
     * @ResultType Async traces do not separate the test, multi and single functions.
     * This allows you to control whether the trace is just a test, returns an array or a single result.
     * Test is always the fastest, but the hit result will be empty. The only way to tell if it hit something
     * is to test if the hit result has an index in it. But the data inside those hit results will be nothing.*/
    UFUNCTION(BlueprintCallable, Category = "Helpers Library|Trace Helpers", meta = (WorldContext = "WorldContextObject", DefaultToSelf = "WorldContextObject"))
    static void AsyncSphereTrace(UObject* WorldContextObject, const FVector& Start, const FVector& End, const float Radius, TEnumAsByte<EAsyncTraceResultType> ResultType,
        const TArray<AActor*>& IgnoredActors, const FCollisionResponseContainer& CustomChannelResponses, FAsyncTraceResultDelegate OnTraceCompleted, bool TraceComplex = false, FTraceDebug DebugOptions = FTraceDebug());

    /**Perform a capsule trace.
     * @ResultType Is this a multi, single or test trace?
     * @CustomChannelResponses Allows you to assign unique responses to specific channels.
     * You will want to call CreateResponseContainerFromProfile() first, then modify that
     * and pass the final result to this function.*/
    UFUNCTION(BlueprintCallable, Category = "Helpers Library|Trace Helpers", meta = (WorldContext = "WorldContextObject", DefaultToSelf = "WorldContextObject"))
    static TArray<FHitResult> CapsuleTrace(UObject* WorldContextObject, const FVector& Start, const FVector& End, FRotator Rotation, const float Radius, const float HalfHeight, TEnumAsByte<EAsyncTraceResultType> ResultType,
        const FCollisionResponseContainer& CustomChannelResponses, const TArray<AActor*>& IgnoredActors, bool TraceComplex = false, FTraceDebug DebugOptions = FTraceDebug());

    /**Start an async sphere trace.
     * @ResultType Async traces do not separate the test, multi and single functions.
     * This allows you to control whether the trace is just a test, returns an array or a single result.
     * Test is always the fastest, but the hit result will be empty. The only way to tell if it hit something
     * is to test if the hit result has an index in it. But the data inside those hit results will be nothing.*/
    UFUNCTION(BlueprintCallable, Category = "Helpers Library|Trace Helpers", meta = (WorldContext = "WorldContextObject", DefaultToSelf = "WorldContextObject"))
    static void AsyncCapsuleTrace(UObject* WorldContextObject, const FVector& Start, const FVector& End, FRotator Rotation, const float Radius, const float HalfHeight, TEnumAsByte<EAsyncTraceResultType> ResultType,
        const TArray<AActor*>& IgnoredActors, const FCollisionResponseContainer& CustomChannelResponses, FAsyncTraceResultDelegate OnTraceCompleted, bool TraceComplex = false, FTraceDebug DebugOptions = FTraceDebug());

    /**Perform a box trace.
     * @ResultType Is this a multi, single or test trace?
     * @CustomChannelResponses Allows you to assign unique responses to specific channels.
     * You will want to call CreateResponseContainerFromProfile() first, then modify that
     * and pass the final result to this function.*/
    UFUNCTION(BlueprintCallable, Category = "Helpers Library|Trace Helpers", meta = (WorldContext = "WorldContextObject", DefaultToSelf = "WorldContextObject"))
    static TArray<FHitResult> BoxTrace(UObject* WorldContextObject, const FVector& Start, const FVector& End, FRotator Rotation, const FVector& Extent, TEnumAsByte<EAsyncTraceResultType> ResultType,
        const FCollisionResponseContainer& CustomChannelResponses, const TArray<AActor*>& IgnoredActors, bool TraceComplex = false, FTraceDebug DebugOptions = FTraceDebug());

    /**Start an async box trace.
     * @ResultType Async traces do not separate the test, multi and single functions.
     * This allows you to control whether the trace is just a test, returns an array or a single result.
     * Test is always the fastest, but the hit result will be empty. The only way to tell if it hit something
     * is to test if the hit result has an index in it. But the data inside those hit results will be nothing.*/
    UFUNCTION(BlueprintCallable, Category = "Helpers Library|Trace Helpers", meta = (WorldContext = "WorldContextObject", DefaultToSelf = "WorldContextObject"))
    static void AsyncBoxTrace(UObject* WorldContextObject, const FVector& Start, const FVector& End, FRotator Rotation, const FVector& Extent, TEnumAsByte<EAsyncTraceResultType> ResultType,
        const TArray<AActor*>& IgnoredActors, const FCollisionResponseContainer& CustomChannelResponses, FAsyncTraceResultDelegate OnTraceCompleted, bool TraceComplex = false, FTraceDebug DebugOptions = FTraceDebug());
    


#pragma region Debug

    static void HandleLineTraceDebug(const UObject* WorldContext, const FTraceDebug& DebugOptions, TArray<FHitResult> HitResult);
    static void HandleSphereTraceDebug(const UObject* WorldContext, const float Radius, const FTraceDebug& DebugOptions, TArray<FHitResult> HitResult);
    static void HandleCapsuleTraceDebug(const UObject* WorldContext, const float Radius, const float HalfHeight, const FTraceDebug& DebugOptions, TArray<FHitResult> HitResult);
    static void HandleBoxTraceDebug(const UObject* WorldContext, const FVector& Shape, const FTraceDebug& DebugOptions, TArray<FHitResult> HitResult);
    
#pragma endregion
};
