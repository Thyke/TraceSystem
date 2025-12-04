// Fill out your copyright notice in the Description page of Project Settings.


#include "TraceFunctionLibrary.h"
#include "DrawDebugHelpers.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "Camera/PlayerCameraManager.h"
#include "GameFramework/Pawn.h"
#include "Components/PrimitiveComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Kismet/GameplayStatics.h"
#include "CollisionQueryParams.h"
#include "Components/BoxComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SphereComponent.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(TraceFunctionLibrary)

// ==================== PRIMARY TRACE FUNCTIONS ====================

bool UTraceFunctionLibrary::TraceFromCameraByType(
	const UObject* WorldContextObject,
	APlayerController* Controller,
	ECameraTraceType CameraType,
	float TraceDistance,
	ECollisionChannel TraceChannel,
	bool bTraceComplex,
	const TArray<AActor*>& ActorsToIgnore,
	FHitResult& OutHit,
	bool bDrawDebug,
	FLinearColor TraceColor,
	FLinearColor HitColor,
	float DebugDuration)
{
	if (!Controller || !WorldContextObject)
	{
		return false;
	}

	UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);
	if (!World)
	{
		return false;
	}

	// Camera trace start/end hesapla
	FVector Start, End, Direction;
	if (!GetCameraTraceStartEnd(Controller, CameraType, TraceDistance, Start, End, Direction))
	{
		return false;
	}

	// Collision query params setup
	FCollisionQueryParams QueryParams;
	SetupTraceParams(QueryParams, ActorsToIgnore, bTraceComplex, FName("CameraLineTrace"));

	// Line trace execution
	const bool bHit = World->LineTraceSingleByChannel(
		OutHit,
		Start,
		End,
		TraceChannel,
		QueryParams
	);

	// Debug visualization
	#if ENABLE_DRAW_DEBUG
	if (bDrawDebug)
	{
		const FVector HitLocation = bHit ? OutHit.ImpactPoint : End;
		DrawDebugLine(World, Start, HitLocation, TraceColor.ToFColor(true), false, DebugDuration, 0, 1.0f);
		
		if (bHit)
		{
			DrawDebugPoint(World, OutHit.ImpactPoint, 10.0f, HitColor.ToFColor(true), false, DebugDuration);
			DrawDebugLine(World, OutHit.ImpactPoint, OutHit.ImpactPoint + OutHit.ImpactNormal * 50.0f, 
				FColor::Blue, false, DebugDuration, 0, 2.0f);
		}
	}
	#endif

	return bHit;
}

bool UTraceFunctionLibrary::SphereTraceFromCameraByType(
	const UObject* WorldContextObject,
	APlayerController* Controller,
	ECameraTraceType CameraType,
	float TraceDistance,
	float SphereRadius,
	ECollisionChannel TraceChannel,
	bool bTraceComplex,
	const TArray<AActor*>& ActorsToIgnore,
	FHitResult& OutHit,
	bool bDrawDebug,
	FLinearColor TraceColor,
	FLinearColor HitColor,
	float DebugDuration)
{
	if (!Controller || !WorldContextObject)
	{
		return false;
	}

	UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);
	if (!World)
	{
		return false;
	}

	// Camera trace start/end hesapla
	FVector Start, End, Direction;
	if (!GetCameraTraceStartEnd(Controller, CameraType, TraceDistance, Start, End, Direction))
	{
		return false;
	}

	// Collision query params
	FCollisionQueryParams QueryParams;
	SetupTraceParams(QueryParams, ActorsToIgnore, bTraceComplex, FName("CameraSphereTrace"));

	// Sphere shape
	const FCollisionShape SphereShape = FCollisionShape::MakeSphere(SphereRadius);

	// Sphere sweep execution
	const bool bHit = World->SweepSingleByChannel(
		OutHit,
		Start,
		End,
		FQuat::Identity,
		TraceChannel,
		SphereShape,
		QueryParams
	);

	// Debug visualization
	#if ENABLE_DRAW_DEBUG
	if (bDrawDebug)
	{
		const FVector HitLocation = bHit ? OutHit.Location : End;
		DrawDebugLine(World, Start, HitLocation, TraceColor.ToFColor(true), false, DebugDuration, 0, 1.0f);
		
		if (bHit)
		{
			DrawDebugSphere(World, OutHit.Location, SphereRadius, 12, HitColor.ToFColor(true), false, DebugDuration);
			DrawDebugLine(World, OutHit.ImpactPoint, OutHit.ImpactPoint + OutHit.ImpactNormal * 50.0f,
				FColor::Blue, false, DebugDuration, 0, 2.0f);
		}
		else
		{
			DrawDebugSphere(World, End, SphereRadius, 12, TraceColor.ToFColor(true), false, DebugDuration);
		}
	}
	#endif

	return bHit;
}

bool UTraceFunctionLibrary::LineTraceMultiFromCamera(
	const UObject* WorldContextObject,
	APlayerController* Controller,
	ECameraTraceType CameraType,
	float TraceDistance,
	ECollisionChannel TraceChannel,
	bool bTraceComplex,
	const TArray<AActor*>& ActorsToIgnore,
	TArray<FHitResult>& OutHits,
	bool bDrawDebug,
	FLinearColor TraceColor,
	FLinearColor HitColor,
	float DebugDuration)
{
	if (!Controller || !WorldContextObject)
	{
		return false;
	}

	UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);
	if (!World)
	{
		return false;
	}

	// Camera trace start/end
	FVector Start, End, Direction;
	if (!GetCameraTraceStartEnd(Controller, CameraType, TraceDistance, Start, End, Direction))
	{
		return false;
	}

	// Query params
	FCollisionQueryParams QueryParams;
	SetupTraceParams(QueryParams, ActorsToIgnore, bTraceComplex, FName("CameraLineTraceMulti"));

	// Clear previous results - avoid reallocation if capacity sufficient
	OutHits.Reset();

	// Multi line trace
	const bool bHit = World->LineTraceMultiByChannel(
		OutHits,
		Start,
		End,
		TraceChannel,
		QueryParams
	);

	// Debug visualization
	#if ENABLE_DRAW_DEBUG
	if (bDrawDebug)
	{
		DrawDebugLine(World, Start, End, TraceColor.ToFColor(true), false, DebugDuration, 0, 1.0f);
		
		for (const FHitResult& Hit : OutHits)
		{
			DrawDebugPoint(World, Hit.ImpactPoint, 8.0f, HitColor.ToFColor(true), false, DebugDuration);
			DrawDebugLine(World, Hit.ImpactPoint, Hit.ImpactPoint + Hit.ImpactNormal * 30.0f,
				FColor::Blue, false, DebugDuration, 0, 1.5f);
		}
	}
	#endif

	return bHit;
}

bool UTraceFunctionLibrary::SweepFromActorShape(
	const UObject* WorldContextObject,
	AActor* Actor,
	ETraceShape SweepShape,
	FVector Direction,
	float Distance,
	ECollisionChannel TraceChannel,
	bool bTraceComplex,
	const TArray<AActor*>& ActorsToIgnore,
	FHitResult& OutHit,
	bool bDrawDebug,
	FLinearColor TraceColor,
	FLinearColor HitColor,
	float DebugDuration)
{
	if (!Actor || !WorldContextObject)
	{
		return false;
	}

	UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);
	if (!World)
	{
		return false;
	}

	// Actor collision shape extract
	FCollisionShape CollisionShape;
	FVector ShapeExtent;
	if (!GetActorCollisionShape(Actor, CollisionShape, ShapeExtent))
	{
		return false;
	}

	// Override shape if specified
	switch (SweepShape)
	{
		case ETraceShape::Sphere:
			CollisionShape = FCollisionShape::MakeSphere(ShapeExtent.X);
			break;
		case ETraceShape::Box:
			CollisionShape = FCollisionShape::MakeBox(ShapeExtent);
			break;
		case ETraceShape::Capsule:
			CollisionShape = FCollisionShape::MakeCapsule(ShapeExtent.X, ShapeExtent.Z);
			break;
	}

	// Normalize direction
	Direction.Normalize();

	const FVector Start = Actor->GetActorLocation();
	const FVector End = Start + Direction * Distance;
	const FQuat Rotation = Actor->GetActorQuat();

	// Query params
	FCollisionQueryParams QueryParams;
	SetupTraceParams(QueryParams, ActorsToIgnore, bTraceComplex, FName("ActorShapeSweep"));
	QueryParams.AddIgnoredActor(Actor); // Self-ignore

	// Sweep execution
	const bool bHit = World->SweepSingleByChannel(
		OutHit,
		Start,
		End,
		Rotation,
		TraceChannel,
		CollisionShape,
		QueryParams
	);

	// Debug visualization
	#if ENABLE_DRAW_DEBUG
	if (bDrawDebug)
	{
		DrawDebugLine(World, Start, End, TraceColor.ToFColor(true), false, DebugDuration, 0, 2.0f);
		
		if (bHit)
		{
			DrawDebugTraceShape(WorldContextObject, SweepShape, OutHit.Location, 
				Rotation.Rotator(), ShapeExtent, HitColor, DebugDuration, 2.0f);
		}
	}
	#endif

	return bHit;
}

// ==================== UTILITY TRACE FUNCTIONS ====================

bool UTraceFunctionLibrary::TraceGroundBelowActor(
	const UObject* WorldContextObject,
	AActor* Actor,
	float TraceDistance,
	float StartOffset,
	ECollisionChannel TraceChannel,
	bool bTraceComplex,
	FHitResult& OutHit,
	bool bDrawDebug,
	float DebugDuration)
{
	if (!Actor || !WorldContextObject)
	{
		return false;
	}

	UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);
	if (!World)
	{
		return false;
	}

	const FVector ActorLocation = Actor->GetActorLocation();
	const FVector Start = ActorLocation + FVector(0.0f, 0.0f, StartOffset);
	const FVector End = ActorLocation - FVector(0.0f, 0.0f, TraceDistance);

	// Query params
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(Actor);
	QueryParams.bTraceComplex = bTraceComplex;

	// Downward trace
	const bool bHit = World->LineTraceSingleByChannel(
		OutHit,
		Start,
		End,
		TraceChannel,
		QueryParams
	);

	// Debug
	#if ENABLE_DRAW_DEBUG
	if (bDrawDebug)
	{
		const FColor Color = bHit ? FColor::Green : FColor::Red;
		DrawDebugLine(World, Start, bHit ? OutHit.ImpactPoint : End, Color, false, DebugDuration, 0, 2.0f);
		
		if (bHit)
		{
			DrawDebugPoint(World, OutHit.ImpactPoint, 12.0f, FColor::Yellow, false, DebugDuration);
		}
	}
	#endif

	return bHit;
}

bool UTraceFunctionLibrary::TraceCeilingAboveActor(
	const UObject* WorldContextObject,
	AActor* Actor,
	float TraceDistance,
	float StartOffset,
	ECollisionChannel TraceChannel,
	bool bTraceComplex,
	FHitResult& OutHit,
	bool bDrawDebug,
	float DebugDuration)
{
	if (!Actor || !WorldContextObject)
	{
		return false;
	}

	UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);
	if (!World)
	{
		return false;
	}

	const FVector ActorLocation = Actor->GetActorLocation();
	const FVector Start = ActorLocation - FVector(0.0f, 0.0f, StartOffset);
	const FVector End = ActorLocation + FVector(0.0f, 0.0f, TraceDistance);

	// Query params
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(Actor);
	QueryParams.bTraceComplex = bTraceComplex;

	// Upward trace
	const bool bHit = World->LineTraceSingleByChannel(
		OutHit,
		Start,
		End,
		TraceChannel,
		QueryParams
	);

	// Debug
	#if ENABLE_DRAW_DEBUG
	if (bDrawDebug)
	{
		const FColor Color = bHit ? FColor::Green : FColor::Red;
		DrawDebugLine(World, Start, bHit ? OutHit.ImpactPoint : End, Color, false, DebugDuration, 0, 2.0f);
		
		if (bHit)
		{
			DrawDebugPoint(World, OutHit.ImpactPoint, 12.0f, FColor::Yellow, false, DebugDuration);
		}
	}
	#endif

	return bHit;
}

bool UTraceFunctionLibrary::TraceFromSocket(
	const UObject* WorldContextObject,
	USkeletalMeshComponent* SkeletalMesh,
	FName SocketName,
	FVector Direction,
	float Distance,
	bool bUseSocketRotation,
	ECollisionChannel TraceChannel,
	bool bTraceComplex,
	const TArray<AActor*>& ActorsToIgnore,
	FHitResult& OutHit,
	bool bDrawDebug,
	FLinearColor TraceColor,
	FLinearColor HitColor,
	float DebugDuration)
{
	if (!SkeletalMesh || !WorldContextObject)
	{
		return false;
	}

	UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);
	if (!World)
	{
		return false;
	}

	// Socket transform query - cached by engine
	if (!SkeletalMesh->DoesSocketExist(SocketName))
	{
		UE_LOG(LogTemp, Warning, TEXT("Socket '%s' does not exist on skeletal mesh"), *SocketName.ToString());
		return false;
	}

	const FTransform SocketTransform = SkeletalMesh->GetSocketTransform(SocketName, RTS_World);
	const FVector Start = SocketTransform.GetLocation();

	// Direction calculation
	FVector TraceDirection = Direction;
	if (bUseSocketRotation)
	{
		TraceDirection = SocketTransform.GetRotation().RotateVector(Direction);
	}
	TraceDirection.Normalize();

	const FVector End = Start + TraceDirection * Distance;

	// Query params
	FCollisionQueryParams QueryParams;
	SetupTraceParams(QueryParams, ActorsToIgnore, bTraceComplex, FName("SocketTrace"));
	
	// Owner actor ignore
	if (AActor* Owner = SkeletalMesh->GetOwner())
	{
		QueryParams.AddIgnoredActor(Owner);
	}

	// Trace execution
	const bool bHit = World->LineTraceSingleByChannel(
		OutHit,
		Start,
		End,
		TraceChannel,
		QueryParams
	);

	// Debug
	#if ENABLE_DRAW_DEBUG
	if (bDrawDebug)
	{
		const FVector HitLoc = bHit ? OutHit.ImpactPoint : End;
		DrawDebugLine(World, Start, HitLoc, TraceColor.ToFColor(true), false, DebugDuration, 0, 2.0f);
		DrawDebugSphere(World, Start, 5.0f, 8, FColor::Cyan, false, DebugDuration);
		
		if (bHit)
		{
			DrawDebugPoint(World, OutHit.ImpactPoint, 10.0f, HitColor.ToFColor(true), false, DebugDuration);
		}
	}
	#endif

	return bHit;
}

// ==================== ADVANCED TRACE FUNCTIONS ====================

bool UTraceFunctionLibrary::ConeTraceMulti(
	const UObject* WorldContextObject,
	FVector StartLocation,
	FVector Direction,
	float MaxDistance,
	float ConeAngle,
	int32 NumRays,
	ECollisionChannel TraceChannel,
	bool bTraceComplex,
	const TArray<AActor*>& ActorsToIgnore,
	TArray<FHitResult>& OutHits,
	bool bDrawDebug,
	FLinearColor TraceColor,
	FLinearColor HitColor,
	float DebugDuration)
{
	if (!WorldContextObject || NumRays <= 0)
	{
		return false;
	}

	UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);
	if (!World)
	{
		return false;
	}

	// Normalize direction
	Direction.Normalize();

	// Generate cone ray directions
	TArray<FVector> RayDirections;
	RayDirections.Reserve(NumRays);
	GenerateConeDirections(Direction, ConeAngle, NumRays, RayDirections);

	// Query params
	FCollisionQueryParams QueryParams;
	SetupTraceParams(QueryParams, ActorsToIgnore, bTraceComplex, FName("ConeTraceMulti"));

	// Clear and reserve output
	OutHits.Reset();
	OutHits.Reserve(NumRays);

	bool bAnyHit = false;

	// Trace each ray
	for (const FVector& RayDir : RayDirections)
	{
		const FVector End = StartLocation + RayDir * MaxDistance;
		
		FHitResult Hit;
		if (World->LineTraceSingleByChannel(Hit, StartLocation, End, TraceChannel, QueryParams))
		{
			OutHits.Add(Hit);
			bAnyHit = true;

			#if ENABLE_DRAW_DEBUG
			if (bDrawDebug)
			{
				DrawDebugLine(World, StartLocation, Hit.ImpactPoint, HitColor.ToFColor(true), false, DebugDuration, 0, 1.0f);
				DrawDebugPoint(World, Hit.ImpactPoint, 6.0f, HitColor.ToFColor(true), false, DebugDuration);
			}
			#endif
		}
		else
		{
			#if ENABLE_DRAW_DEBUG
			if (bDrawDebug)
			{
				DrawDebugLine(World, StartLocation, End, TraceColor.ToFColor(true), false, DebugDuration, 0, 0.5f);
			}
			#endif
		}
	}

	// Draw cone shape
	#if ENABLE_DRAW_DEBUG
	if (bDrawDebug)
	{
		DrawDebugCone(World, StartLocation, Direction, MaxDistance, 
			FMath::DegreesToRadians(ConeAngle), FMath::DegreesToRadians(ConeAngle),
			16, TraceColor.ToFColor(true), false, DebugDuration, 0, 1.0f);
	}
	#endif

	return bAnyHit;
}

bool UTraceFunctionLibrary::GetRandomGroundPointNearby(
	const UObject* WorldContextObject,
	FVector CenterLocation,
	float SearchRadius,
	float MaxTraceDistance,
	float MinGroundAngle,
	ECollisionChannel TraceChannel,
	FVector& OutGroundLocation,
	FVector& OutGroundNormal,
	bool bDrawDebug,
	float DebugDuration)
{
	if (!WorldContextObject)
	{
		return false;
	}

	UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);
	if (!World)
	{
		return false;
	}

	// Rejection sampling - max 10 attempts
	const int32 MaxAttempts = 10;
	const float MinCosAngle = FMath::Cos(FMath::DegreesToRadians(MinGroundAngle));

	FCollisionQueryParams QueryParams;
	QueryParams.bTraceComplex = true;

	for (int32 Attempt = 0; Attempt < MaxAttempts; ++Attempt)
	{
		// Random point in circle (uniform distribution)
		const float Angle = FMath::FRandRange(0.0f, 2.0f * PI);
		const float Radius = FMath::Sqrt(FMath::FRand()) * SearchRadius; // Sqrt for uniform area distribution
		
		const FVector RandomOffset = FVector(
			FMath::Cos(Angle) * Radius,
			FMath::Sin(Angle) * Radius,
			0.0f
		);

		const FVector TraceStart = CenterLocation + RandomOffset + FVector(0.0f, 0.0f, 100.0f);
		const FVector TraceEnd = TraceStart - FVector(0.0f, 0.0f, MaxTraceDistance);

		FHitResult Hit;
		if (World->LineTraceSingleByChannel(Hit, TraceStart, TraceEnd, TraceChannel, QueryParams))
		{
			// Check ground angle
			const float DotProduct = FVector::DotProduct(Hit.ImpactNormal, FVector::UpVector);
			if (DotProduct >= MinCosAngle)
			{
				OutGroundLocation = Hit.ImpactPoint;
				OutGroundNormal = Hit.ImpactNormal;

				#if ENABLE_DRAW_DEBUG
				if (bDrawDebug)
				{
					DrawDebugSphere(World, OutGroundLocation, 25.0f, 12, FColor::Green, false, DebugDuration);
					DrawDebugLine(World, OutGroundLocation, OutGroundLocation + OutGroundNormal * 100.0f,
						FColor::Blue, false, DebugDuration, 0, 2.0f);
				}
				#endif

				return true;
			}
		}

		#if ENABLE_DRAW_DEBUG
		if (bDrawDebug)
		{
			DrawDebugLine(World, TraceStart, TraceEnd, FColor::Red, false, DebugDuration * 0.5f, 0, 0.5f);
		}
		#endif
	}

	return false;
}

// ==================== HELPER FUNCTIONS ====================

bool UTraceFunctionLibrary::GetCameraTraceStartEnd(
	APlayerController* Controller,
	ECameraTraceType CameraType,
	float TraceDistance,
	FVector& OutStart,
	FVector& OutEnd,
	FVector& OutDirection)
{
	if (!Controller)
	{
		return false;
	}

	switch (CameraType)
	{
		case ECameraTraceType::FPS:
		{
			// FPS: Camera viewpoint
			if (APlayerCameraManager* CameraManager = Controller->PlayerCameraManager)
			{
				OutStart = CameraManager->GetCameraLocation();
				OutDirection = CameraManager->GetCameraRotation().Vector();
				OutEnd = OutStart + OutDirection * TraceDistance;
				return true;
			}
			break;
		}

		case ECameraTraceType::ThirdPerson:
		{
			// Third Person: Pawn location + rotation
			if (APawn* Pawn = Controller->GetPawn())
			{
				OutStart = Pawn->GetActorLocation();
				OutDirection = Pawn->GetActorForwardVector();
				OutEnd = OutStart + OutDirection * TraceDistance;
				return true;
			}
			break;
		}

		case ECameraTraceType::RTS:
		{
			// RTS: Mouse cursor deproject
			// Performance note: Deproject ~0.1ms, cache if calling multiple times per frame
			FVector WorldLocation, WorldDirection;
			if (Controller->DeprojectMousePositionToWorld(WorldLocation, WorldDirection))
			{
				OutStart = WorldLocation;
				OutDirection = WorldDirection;
				OutEnd = OutStart + OutDirection * TraceDistance;
				return true;
			}
			break;
		}
	}

	return false;
}

void UTraceFunctionLibrary::DrawDebugTraceShape(
	const UObject* WorldContextObject,
	ETraceShape Shape,
	FVector Location,
	FRotator Rotation,
	FVector Extent,
	FLinearColor Color,
	float Duration,
	float Thickness)
{
	#if ENABLE_DRAW_DEBUG
	if (!WorldContextObject)
	{
		return;
	}

	UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);
	if (!World)
	{
		return;
	}

	const FColor DebugColor = Color.ToFColor(true);

	switch (Shape)
	{
		case ETraceShape::Sphere:
			DrawDebugSphere(World, Location, Extent.X, 16, DebugColor, false, Duration, 0, Thickness);
			break;

		case ETraceShape::Box:
			DrawDebugBox(World, Location, Extent, Rotation.Quaternion(), DebugColor, false, Duration, 0, Thickness);
			break;

		case ETraceShape::Capsule:
			DrawDebugCapsule(World, Location, Extent.Z, Extent.X, Rotation.Quaternion(), 
				DebugColor, false, Duration, 0, Thickness);
			break;
	}
	#endif
}

// ==================== INTERNAL HELPERS ====================

bool UTraceFunctionLibrary::GetActorCollisionShape(AActor* Actor, FCollisionShape& OutShape, FVector& OutShapeExtent)
{
	if (!Actor)
	{
		return false;
	}

	// Root component collision
	UPrimitiveComponent* RootPrimitive = Cast<UPrimitiveComponent>(Actor->GetRootComponent());
	if (!RootPrimitive)
	{
		// Fallback: bounds-based sphere
		FVector Origin, BoxExtent;  // ✅ FIXED: const kaldırıldı - output parameters
		Actor->GetActorBounds(false, Origin, BoxExtent);
		OutShape = FCollisionShape::MakeSphere(BoxExtent.GetMax());
		OutShapeExtent = FVector(BoxExtent.GetMax());
		return true;
	}

	// Extract shape from primitive
	const FVector Scale = RootPrimitive->GetComponentScale();
	
	// Sphere component check
	if (USphereComponent* SphereComp = Cast<USphereComponent>(RootPrimitive))
	{
		const float Radius = SphereComp->GetScaledSphereRadius();
		OutShape = FCollisionShape::MakeSphere(Radius);
		OutShapeExtent = FVector(Radius);
		return true;
	}
	// Capsule component check
	else if (UCapsuleComponent* CapsuleComp = Cast<UCapsuleComponent>(RootPrimitive))
	{
		const float Radius = CapsuleComp->GetScaledCapsuleRadius();
		const float HalfHeight = CapsuleComp->GetScaledCapsuleHalfHeight();
		OutShape = FCollisionShape::MakeCapsule(Radius, HalfHeight);
		OutShapeExtent = FVector(Radius, Radius, HalfHeight);
		return true;
	}
	// Box component check
	else if (UBoxComponent* BoxComp = Cast<UBoxComponent>(RootPrimitive))
	{
		const FVector BoxExtent = BoxComp->GetScaledBoxExtent();
		OutShape = FCollisionShape::MakeBox(BoxExtent);
		OutShapeExtent = BoxExtent;
		return true;
	}
	else
	{
		// Default: box from bounds
		const FVector BoxExtent = RootPrimitive->Bounds.BoxExtent;
		OutShape = FCollisionShape::MakeBox(BoxExtent);
		OutShapeExtent = BoxExtent;
		return true;
	}
}
void UTraceFunctionLibrary::GenerateConeDirections(
	const FVector& CenterDirection,
	float ConeAngleDegrees,
	int32 NumRays,
	TArray<FVector>& OutDirections)
{
	OutDirections.Reset(NumRays);

	if (NumRays <= 0)
	{
		return;
	}

	// Center ray always included
	OutDirections.Add(CenterDirection);

	if (NumRays == 1)
	{
		return;
	}

	// Spherical coordinate distribution
	// Golden spiral for uniform distribution
	const float ConeAngleRad = FMath::DegreesToRadians(ConeAngleDegrees);
	const float GoldenRatio = (1.0f + FMath::Sqrt(5.0f)) / 2.0f;
	const float AngleIncrement = 2.0f * PI * GoldenRatio;

	// Find perpendicular vectors for cone base
	FVector Right, Up;
	CenterDirection.FindBestAxisVectors(Right, Up);

	for (int32 i = 1; i < NumRays; ++i)
	{
		// Polar coordinates
		const float t = static_cast<float>(i) / static_cast<float>(NumRays - 1);
		const float Inclination = t * ConeAngleRad;
		const float Azimuth = static_cast<float>(i) * AngleIncrement;

		// Spherical to Cartesian
		const float SinInclination = FMath::Sin(Inclination);
		const float CosInclination = FMath::Cos(Inclination);

		const FVector LocalDir = FVector(
			CosInclination,
			SinInclination * FMath::Cos(Azimuth),
			SinInclination * FMath::Sin(Azimuth)
		);

		// Transform to world space
		const FVector WorldDir = CenterDirection * LocalDir.X + Right * LocalDir.Y + Up * LocalDir.Z;
		OutDirections.Add(WorldDir.GetSafeNormal());
	}
}

void UTraceFunctionLibrary::SetupTraceParams(
	FCollisionQueryParams& OutParams,
	const TArray<AActor*>& ActorsToIgnore,
	bool bTraceComplex,
	const FName& TraceName)
{
	OutParams = FCollisionQueryParams(TraceName, bTraceComplex);
	OutParams.AddIgnoredActors(ActorsToIgnore);
	
	// Performance: MoveTemp kullanımı consideration
	// bReturnPhysicalMaterial = false by default (physics material query pahalı)
}