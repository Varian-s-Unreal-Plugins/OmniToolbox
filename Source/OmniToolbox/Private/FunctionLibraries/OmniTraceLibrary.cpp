// Copyright (C) Varian Daemon 2025. All Rights Reserved.


#include "FunctionLibraries/OmniTraceLibrary.h"
#include "DrawDebugHelpers.h"
#include "Engine/World.h"
#include "WorldCollision.h"
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
    
    else
    {
        DrawDebugLine(WorldContext->GetWorld(), HitResult.Last().TraceStart, HitResult.Last().Location,
        HitResult.Last().bBlockingHit ? DebugOptions.HitColor.ToFColor(true) : DebugOptions.MissColor.ToFColor(true), DebugOptions.bPersistentLines, DebugOptions.LifeTime);
        
        for(auto& CurrentHit : HitResult)
        {
            if(CurrentHit.bBlockingHit)
            {
                // DrawDebugLine(WorldContext->GetWorld(), CurrentHit.TraceStart, CurrentHit.TraceEnd, DebugOptions.HitColor.ToFColor(true), DebugOptions.bPersistentLines, DebugOptions.LifeTime);
                DrawDebugLine(WorldContext->GetWorld(), CurrentHit.ImpactPoint, CurrentHit.TraceEnd, DebugOptions.MissColor.ToFColor(true), DebugOptions.bPersistentLines, DebugOptions.LifeTime);
                DrawDebugPoint(WorldContext->GetWorld(), CurrentHit.ImpactPoint, DebugOptions.Thickness, DebugOptions.HitColor.ToFColor(true), DebugOptions.bPersistentLines, DebugOptions.LifeTime);
            }
            else if(CurrentHit.Component.IsValid())
            {
                DrawDebugPoint(WorldContext->GetWorld(), CurrentHit.ImpactPoint, DebugOptions.Thickness, DebugOptions.OverlapColor.ToFColor(true), DebugOptions.bPersistentLines, DebugOptions.LifeTime);
            }
        }
    }
}

void UOmniTraceLibrary::HandleSphereTraceDebug(const UObject* WorldContext, const float Radius,
    const FTraceDebug& DebugOptions, TArray<FHitResult> HitResult)
{
    if(!DebugOptions.bEnableDebug) { return; }
    
    if(HitResult.IsEmpty())
    {
        DrawDebugLine(WorldContext->GetWorld(), DebugOptions.Start, DebugOptions.End, DebugOptions.MissColor.ToFColor(true), DebugOptions.bPersistentLines, DebugOptions.LifeTime);
        DrawDebugSphere(WorldContext->GetWorld(), DebugOptions.End, Radius, 16, DebugOptions.MissColor.ToFColor(true), DebugOptions.bPersistentLines, DebugOptions.LifeTime);
    }
    else
    {
        for(auto& CurrentHit : HitResult)
        {
            if(CurrentHit.bBlockingHit)
            {
                DrawDebugSphere(WorldContext->GetWorld(), CurrentHit.Location, Radius, 16, DebugOptions.HitColor.ToFColor(true), DebugOptions.bPersistentLines, DebugOptions.LifeTime);
                DrawDebugPoint(WorldContext->GetWorld(), CurrentHit.ImpactPoint, DebugOptions.Thickness, DebugOptions.HitColor.ToFColor(true), DebugOptions.bPersistentLines, DebugOptions.LifeTime);
            }
            else if(CurrentHit.Component.IsValid())
            {
                DrawDebugSphere(WorldContext->GetWorld(), CurrentHit.Location, Radius, 16, DebugOptions.OverlapColor.ToFColor(true), DebugOptions.bPersistentLines, DebugOptions.LifeTime);
                DrawDebugPoint(WorldContext->GetWorld(), CurrentHit.ImpactPoint, DebugOptions.Thickness, DebugOptions.OverlapColor.ToFColor(true), DebugOptions.bPersistentLines, DebugOptions.LifeTime);
            }
            else
            {
                DrawDebugSphere(WorldContext->GetWorld(), CurrentHit.Location, Radius, 16, DebugOptions.MissColor.ToFColor(true), DebugOptions.bPersistentLines, DebugOptions.LifeTime);
            }
        }

        DrawDebugLine(WorldContext->GetWorld(), HitResult.Last().TraceStart, HitResult.Last().Location,
            HitResult.Last().bBlockingHit ? DebugOptions.HitColor.ToFColor(true) : DebugOptions.MissColor.ToFColor(true), DebugOptions.bPersistentLines, DebugOptions.LifeTime);
    }
}

void UOmniTraceLibrary::HandleCapsuleTraceDebug(const UObject* WorldContext, const float Radius,
    const float HalfHeight, const FTraceDebug& DebugOptions, TArray<FHitResult> HitResult)
{
    if(!DebugOptions.bEnableDebug) { return; }
    
    if(HitResult.IsEmpty())
    {
        DrawDebugLine(WorldContext->GetWorld(), DebugOptions.Start, DebugOptions.End, DebugOptions.MissColor.ToFColor(true), DebugOptions.bPersistentLines, DebugOptions.LifeTime);
        DrawDebugCapsule(WorldContext->GetWorld(), DebugOptions.End, HalfHeight, Radius, DebugOptions.Rotation, DebugOptions.MissColor.ToFColor(true), DebugOptions.bPersistentLines, DebugOptions.LifeTime);
    }
    else
    {
        for(auto& CurrentHit : HitResult)
        {
            if(CurrentHit.bBlockingHit)
            {
                DrawDebugCapsule(WorldContext->GetWorld(), CurrentHit.Location, HalfHeight, Radius, DebugOptions.Rotation, DebugOptions.HitColor.ToFColor(true), DebugOptions.bPersistentLines, DebugOptions.LifeTime);
                DrawDebugPoint(WorldContext->GetWorld(), CurrentHit.ImpactPoint, DebugOptions.Thickness, DebugOptions.HitColor.ToFColor(true), DebugOptions.bPersistentLines, DebugOptions.LifeTime);
            }
            else if(CurrentHit.Component.IsValid())
            {
                DrawDebugCapsule(WorldContext->GetWorld(), CurrentHit.Location, HalfHeight, Radius, DebugOptions.Rotation, DebugOptions.OverlapColor.ToFColor(true), DebugOptions.bPersistentLines, DebugOptions.LifeTime);
                DrawDebugPoint(WorldContext->GetWorld(), CurrentHit.ImpactPoint, DebugOptions.Thickness, DebugOptions.OverlapColor.ToFColor(true), DebugOptions.bPersistentLines, DebugOptions.LifeTime);
            }
            else
            {
                DrawDebugCapsule(WorldContext->GetWorld(), CurrentHit.Location, HalfHeight, Radius, DebugOptions.Rotation, DebugOptions.MissColor.ToFColor(true), DebugOptions.bPersistentLines, DebugOptions.LifeTime);
            }
        }

        DrawDebugLine(WorldContext->GetWorld(), HitResult.Last().TraceStart, HitResult.Last().Location,
            HitResult.Last().bBlockingHit ? DebugOptions.HitColor.ToFColor(true) : DebugOptions.MissColor.ToFColor(true), DebugOptions.bPersistentLines, DebugOptions.LifeTime);
    }
}

void UOmniTraceLibrary::HandleBoxTraceDebug(const UObject* WorldContext, const FVector& Shape, const FTraceDebug& DebugOptions, TArray<FHitResult> HitResult)
{
    if(!DebugOptions.bEnableDebug) { return; }
    
    if(HitResult.IsEmpty())
    {
        DrawDebugLine(WorldContext->GetWorld(), DebugOptions.Start, DebugOptions.End, DebugOptions.MissColor.ToFColor(true), DebugOptions.bPersistentLines, DebugOptions.LifeTime);
        DrawDebugBox(WorldContext->GetWorld(), DebugOptions.End, Shape, DebugOptions.MissColor.ToFColor(true), DebugOptions.bPersistentLines, DebugOptions.LifeTime);
    }
    else
    {
        for(auto& CurrentHit : HitResult)
        {
            if(CurrentHit.bBlockingHit)
            {
                DrawDebugBox(WorldContext->GetWorld(), CurrentHit.Location, Shape, DebugOptions.Rotation, DebugOptions.HitColor.ToFColor(true), DebugOptions.bPersistentLines, DebugOptions.LifeTime);
                DrawDebugPoint(WorldContext->GetWorld(), CurrentHit.ImpactPoint, DebugOptions.Thickness, DebugOptions.HitColor.ToFColor(true), DebugOptions.bPersistentLines, DebugOptions.LifeTime);
            }
            else if(CurrentHit.Component.IsValid())
            {
                DrawDebugBox(WorldContext->GetWorld(), CurrentHit.Location, Shape, DebugOptions.Rotation, DebugOptions.OverlapColor.ToFColor(true), DebugOptions.bPersistentLines, DebugOptions.LifeTime);
                DrawDebugPoint(WorldContext->GetWorld(), CurrentHit.ImpactPoint, DebugOptions.Thickness, DebugOptions.OverlapColor.ToFColor(true), DebugOptions.bPersistentLines, DebugOptions.LifeTime);
            }
            else
            {
                DrawDebugBox(WorldContext->GetWorld(), CurrentHit.Location, Shape, DebugOptions.Rotation, DebugOptions.MissColor.ToFColor(true), DebugOptions.bPersistentLines, DebugOptions.LifeTime);
            }
        }

        DrawDebugLine(WorldContext->GetWorld(), HitResult.Last().TraceStart, HitResult.Last().Location,
            HitResult.Last().bBlockingHit ? DebugOptions.HitColor.ToFColor(true) : DebugOptions.MissColor.ToFColor(true), DebugOptions.bPersistentLines, DebugOptions.LifeTime);
    }
}
