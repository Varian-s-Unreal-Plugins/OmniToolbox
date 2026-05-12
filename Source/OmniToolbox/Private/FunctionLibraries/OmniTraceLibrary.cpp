// Copyright (C) Varian Daemon 2025. All Rights Reserved.


#include "FunctionLibraries/OmniTraceLibrary.h"
#include "DrawDebugHelpers.h"
#include "Engine/World.h"
#include "WorldCollision.h"
#include "FunctionLibraries/OmniEditorLibrary.h"
#include "ProfilingDebugging/CpuProfilerTrace.h"


FCollisionResponseContainer UOmniTraceLibrary::CreateResponseContainerFromProfile(FName ProfileName,
    FTraceChannelAndResponseContainer CustomChannelResponses)
{
    ECollisionChannel TraceChannel;
    FCollisionResponseParams ResponseParam;
    UCollisionProfile::GetChannelAndResponseParams(ProfileName, TraceChannel, ResponseParam);

    //Merge our custom responses with the profile
    for(auto& CurrentResponse : CustomChannelResponses.ChannelsAndResponses)
    {
        ResponseParam.CollisionResponse.SetResponse(CurrentResponse.Channel, CurrentResponse.Response);
    }

    return ResponseParam.CollisionResponse;
}

FCollisionResponseContainer UOmniTraceLibrary::CreateResponseContainer(
    FTraceChannelAndResponseContainer CustomChannelResponses)
{
    FCollisionResponseParams ResponseParam;
    
    for(auto& CurrentResponse : CustomChannelResponses.ChannelsAndResponses)
    {
        ResponseParam.CollisionResponse.SetResponse(CurrentResponse.Channel, CurrentResponse.Response);
    }

    return ResponseParam.CollisionResponse;
}

void UOmniTraceLibrary::FilterHitResultsByLineOfSight(TArray<FHitResult>& HitResultsToFilter,
    UObject* WorldContextObject, const FVector& Start, FName Profile,
    FOmniTraceChannelSettings TraceSettings, const TArray<AActor*>& IgnoredActors, bool TraceComplex,
    FTraceDebug DebugOptions)
{
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActors(IgnoredActors);
    QueryParams.TraceTag = DebugOptions.TraceTag;
    QueryParams.bTraceComplex = TraceComplex;
    
    ECollisionChannel TraceChannel;
    FCollisionResponseParams ResponseParam;
    UCollisionProfile::GetChannelAndResponseParams(Profile, TraceChannel, ResponseParam);
    if(TraceSettings.UseTraceType)
    {
        TraceChannel = UEngineTypes::ConvertToCollisionChannel(TraceSettings.TraceType);
    }
    
    for(int i = 0; i < HitResultsToFilter.Num(); ++i)
    {
        bool bHit;
        FHitResult SingleHitResult;
        bHit = WorldContextObject->GetWorld()->LineTraceSingleByChannel(SingleHitResult, Start, HitResultsToFilter[i].ImpactPoint, TraceChannel, QueryParams, ResponseParam);
        if(bHit && SingleHitResult.GetComponent() != HitResultsToFilter[i].GetComponent())
        {
            HitResultsToFilter.RemoveAt(i, EAllowShrinking::No);
            i--;
        }
    }
    
    HitResultsToFilter.Shrink();
}

TArray<FHitResult> UOmniTraceLibrary::LineTrace(UObject* WorldContextObject,
                                                const FVector& Start,
                                                const FVector& End,
                                                TEnumAsByte<EAsyncTraceResultType> ResultType,
                                                UPARAM(Meta=(GetOptions="Engine.KismetSystemLibrary.GetCollisionProfileNames")) FName Profile,
                                                FOmniTraceChannelSettings TraceSettings,
                                                const TArray<AActor*>& IgnoredActors, 
                                                bool TraceComplex,
                                                FTraceDebug DebugOptions)
{
    TRACE_CPUPROFILER_EVENT_SCOPE(UTraceHelpersLibrary::LineTrace);
    
    TArray<FHitResult> HitResult;
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActors(IgnoredActors);
    QueryParams.TraceTag = DebugOptions.TraceTag;
    QueryParams.bTraceComplex = TraceComplex;
    DebugOptions.Start = Start;
    DebugOptions.End = End;
    
    ECollisionChannel TraceChannel;
    FCollisionResponseParams ResponseParam;
    UCollisionProfile::GetChannelAndResponseParams(Profile, TraceChannel, ResponseParam);
    if(TraceSettings.UseTraceType)
    {
        TraceChannel = UEngineTypes::ConvertToCollisionChannel(TraceSettings.TraceType);
    }

    bool bHit;
    FHitResult SingleHitResult;
    switch(ResultType)
    {
    case MultiResult:
        WorldContextObject->GetWorld()->LineTraceMultiByChannel(HitResult, Start, End, TraceChannel, QueryParams, ResponseParam);
        break;
    case SingleResult:
        bHit = WorldContextObject->GetWorld()->LineTraceSingleByChannel(SingleHitResult, Start, End, TraceChannel, QueryParams, ResponseParam);
        if(bHit)
        {
            HitResult.Add(SingleHitResult);
        }
        break;
    case TestResult:
        bHit = WorldContextObject->GetWorld()->LineTraceTestByChannel(Start, End, TraceChannel, QueryParams, ResponseParam);
        if(bHit)
        {
            HitResult.Add(SingleHitResult);
        }
        break;
    default:
        break;
    }
    
    HandleLineTraceDebug(WorldContextObject, DebugOptions, HitResult);

    return HitResult;
}

void UOmniTraceLibrary::AsyncLineTrace(UObject* WorldContextObject, const FVector& Start, const FVector& End, TEnumAsByte<EAsyncTraceResultType> ResultType,
    const TArray<AActor*>& IgnoredActors, UPARAM(Meta=(GetOptions="Engine.KismetSystemLibrary.GetCollisionProfileNames")) FName Profile, FOmniTraceChannelSettings TraceSettings, FAsyncTraceResultDelegate OnTraceCompleted, bool TraceComplex, FTraceDebug DebugOptions)
{
    if(!WorldContextObject)
    {
        return;
    }
    
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActors(IgnoredActors);
    QueryParams.TraceTag = DebugOptions.TraceTag;
    QueryParams.bTraceComplex = TraceComplex;
    DebugOptions.Start = Start;
    DebugOptions.End = End;

    FTraceDelegate* TraceDelegate = new FTraceDelegate();
    TraceDelegate->BindLambda([OnTraceCompleted, DebugOptions](const FTraceHandle& TraceHandle, FTraceDatum& TraceData)
    {
        HandleLineTraceDebug(TraceData.PhysWorld.Get(), DebugOptions, TraceData.OutHits);

        OnTraceCompleted.Execute(TraceData.OutHits);
    });

    EAsyncTraceType TraceType;
    if(ResultType == TestResult)
    {
        TraceType = EAsyncTraceType::Test;
    }
    else if(ResultType == MultiResult)
    {
        TraceType = EAsyncTraceType::Multi;
    }
    else
    {
        TraceType = EAsyncTraceType::Single;
    }
    
    ECollisionChannel TraceChannel;
    FCollisionResponseParams ResponseParam;
    UCollisionProfile::GetChannelAndResponseParams(Profile, TraceChannel, ResponseParam);
    if(TraceSettings.UseTraceType)
    {
        TraceChannel = UEngineTypes::ConvertToCollisionChannel(TraceSettings.TraceType);
    }
    
    WorldContextObject->GetWorld()->AsyncLineTraceByChannel(TraceType, Start, End, TraceChannel, QueryParams, ResponseParam, TraceDelegate);
}

    TArray<FHitResult> UOmniTraceLibrary::SphereTrace(UObject* WorldContextObject, const FVector& Start,
    const FVector& End, const float Radius, TEnumAsByte<EAsyncTraceResultType> ResultType,
    UPARAM(Meta=(GetOptions="Engine.KismetSystemLibrary.GetCollisionProfileNames")) FName Profile, FOmniTraceChannelSettings TraceSettings, const TArray<AActor*>& IgnoredActors, bool TraceComplex,
    FTraceDebug DebugOptions)
{
    TRACE_CPUPROFILER_EVENT_SCOPE(UTraceHelpersLibrary::SphereTrace);

    TArray<FHitResult> HitResult;
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActors(IgnoredActors);
    QueryParams.TraceTag = DebugOptions.TraceTag;
    QueryParams.bTraceComplex = TraceComplex;
    
    DebugOptions.Start = Start;
    DebugOptions.End = End;
    
    ECollisionChannel TraceChannel;
    FCollisionResponseParams ResponseParam;
    UCollisionProfile::GetChannelAndResponseParams(Profile, TraceChannel, ResponseParam);
    if(TraceSettings.UseTraceType)
    {
        TraceChannel = UEngineTypes::ConvertToCollisionChannel(TraceSettings.TraceType);
    }

    bool bHit;
    FHitResult SingleHitResult;
    switch(ResultType)
    {
    case MultiResult:
        WorldContextObject->GetWorld()->SweepMultiByChannel(HitResult, Start, End, FQuat::Identity, TraceChannel, FCollisionShape::MakeSphere(Radius), QueryParams, ResponseParam);
        break;
    case SingleResult:
        bHit = WorldContextObject->GetWorld()->SweepSingleByChannel(SingleHitResult, Start, End, FQuat::Identity, TraceChannel, FCollisionShape::MakeSphere(Radius), QueryParams, ResponseParam);
        if(bHit)
        {
            HitResult.Add(SingleHitResult);
        }
        break;
    case TestResult:
        bHit = WorldContextObject->GetWorld()->SweepTestByChannel(Start, End, FQuat::Identity, TraceChannel, FCollisionShape::MakeSphere(Radius), QueryParams, ResponseParam);
        if(bHit)
        {
            HitResult.Add(SingleHitResult);
        }
        break;
    default: break;
    }
    
    HandleSphereTraceDebug(WorldContextObject, Radius, DebugOptions, HitResult);

    return HitResult;
}

void UOmniTraceLibrary::AsyncSphereTrace(UObject* WorldContextObject, const FVector& Start, const FVector& End,
    const float Radius, TEnumAsByte<EAsyncTraceResultType> ResultType, const TArray<AActor*>& IgnoredActors,
    UPARAM(Meta=(GetOptions="Engine.KismetSystemLibrary.GetCollisionProfileNames")) FName Profile, FOmniTraceChannelSettings TraceSettings, 
    FAsyncTraceResultDelegate OnTraceCompleted, bool TraceComplex, FTraceDebug DebugOptions)
{
    if(!WorldContextObject)
    {
        return;
    }
    
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActors(IgnoredActors);
    QueryParams.TraceTag = DebugOptions.TraceTag;
    QueryParams.bTraceComplex = TraceComplex;
    DebugOptions.Start = Start;
    DebugOptions.End = End;
    
    ECollisionChannel TraceChannel;
    FCollisionResponseParams ResponseParam;
    UCollisionProfile::GetChannelAndResponseParams(Profile, TraceChannel, ResponseParam);
    if(TraceSettings.UseTraceType)
    {
        TraceChannel = UEngineTypes::ConvertToCollisionChannel(TraceSettings.TraceType);
    }

    FTraceDelegate* TraceDelegate = new FTraceDelegate();
    TraceDelegate->BindLambda([OnTraceCompleted, DebugOptions, Radius](const FTraceHandle& TraceHandle, FTraceDatum& TraceData)
    {
        HandleSphereTraceDebug(TraceData.PhysWorld.Get(), Radius, DebugOptions, TraceData.OutHits);

        OnTraceCompleted.Execute(TraceData.OutHits);
    });

    EAsyncTraceType TraceType;
    if(ResultType == TestResult)
    {
        TraceType = EAsyncTraceType::Test;
    }
    else if(ResultType == MultiResult)
    {
        TraceType = EAsyncTraceType::Multi;
    }
    else
    {
        TraceType = EAsyncTraceType::Single;
    }
    
    WorldContextObject->GetWorld()->AsyncSweepByChannel(TraceType, Start, End, FQuat::Identity, TraceChannel, FCollisionShape::MakeSphere(Radius), QueryParams, ResponseParam, TraceDelegate);
}

TArray<FHitResult> UOmniTraceLibrary::CapsuleTrace(UObject* WorldContextObject, const FVector& Start,
                                                      const FVector& End, FRotator Rotation, const float Radius, const float HalfHeight, TEnumAsByte<EAsyncTraceResultType> ResultType,
                                                      UPARAM(Meta=(GetOptions="Engine.KismetSystemLibrary.GetCollisionProfileNames")) FName Profile, FOmniTraceChannelSettings TraceSettings, 
                                                      const TArray<AActor*>& IgnoredActors, bool TraceComplex, FTraceDebug DebugOptions)
{
    TRACE_CPUPROFILER_EVENT_SCOPE(UTraceHelpersLibrary::CapsuleTrace);

    FQuat QuatRotation = FQuat(Rotation);

    TArray<FHitResult> HitResult;
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActors(IgnoredActors);
    QueryParams.TraceTag = DebugOptions.TraceTag;
    QueryParams.bTraceComplex = TraceComplex;
    DebugOptions.Start = Start;
    DebugOptions.End = End;
    DebugOptions.Rotation = QuatRotation;
    
    ECollisionChannel TraceChannel;
    FCollisionResponseParams ResponseParam;
    UCollisionProfile::GetChannelAndResponseParams(Profile, TraceChannel, ResponseParam);
    if(TraceSettings.UseTraceType)
    {
        TraceChannel = UEngineTypes::ConvertToCollisionChannel(TraceSettings.TraceType);
    }

    bool bHit;
    FHitResult SingleHitResult;
    switch(ResultType)
    {
    case MultiResult:
        WorldContextObject->GetWorld()->SweepMultiByChannel(HitResult, Start, End, QuatRotation, TraceChannel, FCollisionShape::MakeCapsule(Radius, HalfHeight), QueryParams, ResponseParam);
        break;
    case SingleResult:
        bHit = WorldContextObject->GetWorld()->SweepSingleByChannel(SingleHitResult, Start, End, QuatRotation, TraceChannel, FCollisionShape::MakeCapsule(Radius, HalfHeight), QueryParams, ResponseParam);
        if(bHit)
        {
            HitResult.Add(SingleHitResult);
        }
        break;
    case TestResult:
        bHit = WorldContextObject->GetWorld()->SweepTestByChannel(Start, End, QuatRotation, TraceChannel, FCollisionShape::MakeCapsule(Radius, HalfHeight), QueryParams, ResponseParam);
        if(bHit)
        {
            HitResult.Add(SingleHitResult);
        }
        break;
    default: break;
    }

    // UKismetSystemLibrary::DrawDebugCapsuleTraceMulti(World, Start, End, Radius, HalfHeight, DrawDebugType, bHit, OutHits, TraceColor, TraceHitColor, DrawTime);
    HandleCapsuleTraceDebug(WorldContextObject, Radius, HalfHeight, DebugOptions, HitResult);

    return HitResult;
}

void UOmniTraceLibrary::AsyncCapsuleTrace(UObject* WorldContextObject, const FVector& Start, const FVector& End,
    FRotator Rotation, const float Radius, const float HalfHeight, TEnumAsByte<EAsyncTraceResultType> ResultType,
    const TArray<AActor*>& IgnoredActors, UPARAM(Meta=(GetOptions="Engine.KismetSystemLibrary.GetCollisionProfileNames")) FName Profile, 
    FOmniTraceChannelSettings TraceSettings, FAsyncTraceResultDelegate OnTraceCompleted, bool TraceComplex, FTraceDebug DebugOptions)
{
    if(!WorldContextObject)
    {
        return;
    }

    FQuat QuatRotation = FQuat(Rotation);
    
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActors(IgnoredActors);
    QueryParams.TraceTag = DebugOptions.TraceTag;
    QueryParams.bTraceComplex = TraceComplex;
    DebugOptions.Start = Start;
    DebugOptions.End = End;
    DebugOptions.Rotation = QuatRotation;

    FTraceDelegate* TraceDelegate = new FTraceDelegate();
    TraceDelegate->BindLambda([OnTraceCompleted, DebugOptions, Radius, HalfHeight](const FTraceHandle& TraceHandle, FTraceDatum& TraceData)
    {
        HandleCapsuleTraceDebug(TraceData.PhysWorld.Get(), Radius, HalfHeight, DebugOptions, TraceData.OutHits);

        OnTraceCompleted.Execute(TraceData.OutHits);
    });

    EAsyncTraceType TraceType;
    if(ResultType == TestResult)
    {
        TraceType = EAsyncTraceType::Test;
    }
    else if(ResultType == MultiResult)
    {
        TraceType = EAsyncTraceType::Multi;
    }
    else
    {
        TraceType = EAsyncTraceType::Single;
    }
    
    ECollisionChannel TraceChannel;
    FCollisionResponseParams ResponseParam;
    UCollisionProfile::GetChannelAndResponseParams(Profile, TraceChannel, ResponseParam);
    if(TraceSettings.UseTraceType)
    {
        TraceChannel = UEngineTypes::ConvertToCollisionChannel(TraceSettings.TraceType);
    }
    
    WorldContextObject->GetWorld()->AsyncSweepByChannel(TraceType, Start, End, QuatRotation, TraceChannel, FCollisionShape::MakeCapsule(Radius, HalfHeight), QueryParams, ResponseParam, TraceDelegate);
}

TArray<FHitResult> UOmniTraceLibrary::BoxTrace(UObject* WorldContextObject, const FVector& Start, const FVector& End, FRotator Rotation, const FVector& Extent,
    TEnumAsByte<EAsyncTraceResultType> ResultType,
    UPARAM(Meta=(GetOptions="Engine.KismetSystemLibrary.GetCollisionProfileNames")) FName Profile, FOmniTraceChannelSettings TraceSettings, const TArray<AActor*>& IgnoredActors,
    bool TraceComplex, FTraceDebug DebugOptions)
{
    TRACE_CPUPROFILER_EVENT_SCOPE(UTraceHelpersLibrary::BoxTrace);

    FQuat QuatRotation = FQuat(Rotation);

    TArray<FHitResult> HitResult;
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActors(IgnoredActors);
    QueryParams.TraceTag = DebugOptions.TraceTag;
    QueryParams.bTraceComplex = TraceComplex;
    DebugOptions.Start = Start;
    DebugOptions.End = End;
    DebugOptions.Rotation = QuatRotation;
    
    ECollisionChannel TraceChannel;
    FCollisionResponseParams ResponseParam;
    UCollisionProfile::GetChannelAndResponseParams(Profile, TraceChannel, ResponseParam);
    if(TraceSettings.UseTraceType)
    {
        TraceChannel = UEngineTypes::ConvertToCollisionChannel(TraceSettings.TraceType);
    }

    bool bHit;
    FHitResult SingleHitResult;
    switch(ResultType)
    {
    case MultiResult:
        WorldContextObject->GetWorld()->SweepMultiByChannel(HitResult, Start, End, QuatRotation, TraceChannel, FCollisionShape::MakeBox(Extent), QueryParams, ResponseParam);
        break;
    case SingleResult:
        bHit = WorldContextObject->GetWorld()->SweepSingleByChannel(SingleHitResult, Start, End, QuatRotation, TraceChannel, FCollisionShape::MakeBox(Extent), QueryParams, ResponseParam);
        if(bHit)
        {
            HitResult.Add(SingleHitResult);
        }
        break;
    case TestResult:
        bHit = WorldContextObject->GetWorld()->SweepTestByChannel(Start, End, QuatRotation, TraceChannel, FCollisionShape::MakeBox(Extent), QueryParams, ResponseParam);
        if(bHit)
        {
            HitResult.Add(SingleHitResult);
        }
        break;
    default:
        break;
    }
    
    HandleBoxTraceDebug(WorldContextObject, Extent, DebugOptions, HitResult);

    return HitResult;
}

void UOmniTraceLibrary::AsyncBoxTrace(UObject* WorldContextObject, const FVector& Start, const FVector& End,
    FRotator Rotation, const FVector& Extent, TEnumAsByte<EAsyncTraceResultType> ResultType,
    const TArray<AActor*>& IgnoredActors, UPARAM(Meta=(GetOptions="Engine.KismetSystemLibrary.GetCollisionProfileNames")) FName Profile,
    FOmniTraceChannelSettings TraceSettings, FAsyncTraceResultDelegate OnTraceCompleted, bool TraceComplex, FTraceDebug DebugOptions)
{
    if(!WorldContextObject)
    {
        return;
    }

    FQuat QuatRotation = FQuat(Rotation);
    
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActors(IgnoredActors);
    QueryParams.TraceTag = DebugOptions.TraceTag;
    QueryParams.bTraceComplex = TraceComplex;
    DebugOptions.Start = Start;
    DebugOptions.End = End;
    DebugOptions.Rotation = QuatRotation;

    FTraceDelegate* TraceDelegate = new FTraceDelegate();
    TraceDelegate->BindLambda([OnTraceCompleted, DebugOptions, Extent](const FTraceHandle& TraceHandle, FTraceDatum& TraceData)
    {
        HandleBoxTraceDebug(TraceData.PhysWorld.Get(), Extent, DebugOptions, TraceData.OutHits);

        OnTraceCompleted.Execute(TraceData.OutHits);
    });

    EAsyncTraceType TraceType;
    if(ResultType == TestResult)
    {
        TraceType = EAsyncTraceType::Test;
    }
    else if(ResultType == MultiResult)
    {
        TraceType = EAsyncTraceType::Multi;
    }
    else
    {
        TraceType = EAsyncTraceType::Single;
    }
    
    ECollisionChannel TraceChannel;
    FCollisionResponseParams ResponseParam;
    UCollisionProfile::GetChannelAndResponseParams(Profile, TraceChannel, ResponseParam);
    if(TraceSettings.UseTraceType)
    {
        TraceChannel = UEngineTypes::ConvertToCollisionChannel(TraceSettings.TraceType);
    }
    
    WorldContextObject->GetWorld()->AsyncSweepByChannel(TraceType, Start, End, QuatRotation, TraceChannel, FCollisionShape::MakeBox(Extent), QueryParams, ResponseParam, TraceDelegate);
}

bool UOmniTraceLibrary::DebugHitResults(const UObject* WorldContext, const FTraceDebug& DebugOptions,
    TArray<FHitResult> HitResult)
{
    bool BlockingHitFound = false;
    
    if(HitResult.IsEmpty() == false)
    {
        int32 Index = -1;
        for(auto& CurrentHit : HitResult)
        {
            Index++;
            if(CurrentHit.bBlockingHit)
            {
                BlockingHitFound = true;
                UOmniEditorLibrary::DrawAndLogBox(WorldContext->GetWorld(), CurrentHit.ImpactPoint, FVector(5), FString::Printf(TEXT("%s_%i"), *DebugOptions.TraceTag.ToString(), Index), "", DebugOptions.HitColor);
            }
            else if(CurrentHit.Component.IsValid())
            {
                UOmniEditorLibrary::DrawAndLogBox(WorldContext->GetWorld(), CurrentHit.ImpactPoint, FVector(5), FString::Printf(TEXT("%s_%i"), *DebugOptions.TraceTag.ToString(), Index), "", DebugOptions.OverlapColor);
            }
        }
    }
    
    return BlockingHitFound;
}

void UOmniTraceLibrary::HandleLineTraceDebug(const UObject* WorldContext, const FTraceDebug& DebugOptions, TArray<FHitResult> HitResult)
{
    if(!DebugOptions.bEnableDebug) { return; }
    
    UOmniEditorLibrary::DrawAndLogLine(WorldContext->GetWorld(), DebugOptions.Start, DebugOptions.End, DebugOptions.TraceTag.ToString(), "",
    DebugHitResults(WorldContext, DebugOptions, HitResult) ? DebugOptions.HitColor : HitResult.IsEmpty() ? DebugOptions.MissColor : DebugOptions.OverlapColor);
}

void UOmniTraceLibrary::HandleSphereTraceDebug(const UObject* WorldContext, const float Radius,
    const FTraceDebug& DebugOptions, TArray<FHitResult> HitResult)
{
    if(!DebugOptions.bEnableDebug) { return; }
    
    UOmniEditorLibrary::DrawAndLogSphere(WorldContext->GetWorld(), DebugOptions.Start, Radius, DebugOptions.TraceTag.ToString(), "", 
        DebugHitResults(WorldContext, DebugOptions, HitResult) ? DebugOptions.HitColor : HitResult.IsEmpty() ? DebugOptions.MissColor : DebugOptions.OverlapColor);
}

void UOmniTraceLibrary::HandleCapsuleTraceDebug(const UObject* WorldContext, const float Radius,
    const float HalfHeight, const FTraceDebug& DebugOptions, TArray<FHitResult> HitResult)
{
    if(!DebugOptions.bEnableDebug) { return; }
    
    UOmniEditorLibrary::DrawAndLogCapsule(WorldContext->GetWorld(), DebugOptions.Start, HalfHeight, Radius, DebugOptions.Rotation, DebugOptions.TraceTag.ToString(), "", 
        DebugHitResults(WorldContext, DebugOptions, HitResult) ? DebugOptions.HitColor : HitResult.IsEmpty() ? DebugOptions.MissColor : DebugOptions.OverlapColor);
}

void UOmniTraceLibrary::HandleBoxTraceDebug(const UObject* WorldContext, const FVector& Shape, const FTraceDebug& DebugOptions, TArray<FHitResult> HitResult)
{
    if(!DebugOptions.bEnableDebug) { return; }
    
    UOmniEditorLibrary::DrawAndLogBox(WorldContext->GetWorld(), DebugOptions.Start, Shape, DebugOptions.TraceTag.ToString(), "", 
    DebugHitResults(WorldContext, DebugOptions, HitResult) ? DebugOptions.HitColor : HitResult.IsEmpty() ? DebugOptions.MissColor : DebugOptions.OverlapColor);
}
