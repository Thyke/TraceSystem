// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "TraceTypes.h"
#include "Engine/EngineTypes.h"
#include "TraceFunctionLibrary.generated.h"

/**
 * Trace Function Library
 * 
 * Performance Considerations:
 * - Tüm trace fonksiyonlar inline optimize edilmiş helper'lar kullanıyor
 * - Debug drawing optional ve runtime'da disable edilebilir
 * - Multi-hit trace'ler için TArray reserve stratejisi kullanılıyor
 * - Camera calculations cached edilip reuse ediliyor
 * 
 * Memory Management:
 * - Stack allocation preferred (TArray yerine static array kullanımı)
 * - No dynamic allocations in hot paths
 * - FHitResult'lar output param olarak geçiliyor (copy overhead yok)
 * 
 * Thread Safety:
 * - Tüm fonksiyonlar game thread'de çalışmalı (UWorld requirement)
 * - Camera queries için const correctness garantisi
 */
UCLASS()
class TRACESYSTEM_API UTraceFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
public:
	
	// ==================== PRIMARY TRACE FUNCTIONS ====================
	
	/**
	 * Camera'dan belirtilen mesafeye line trace yapar
	 * 
	 * @param Controller - Player controller (camera bilgisi için)
	 * @param CameraType - Trace başlangıç hesaplama modu
	 * @param TraceDistance - Maximum trace mesafesi
	 * @param TraceChannel - Collision channel (ECC_Visibility, ECC_Camera, etc.)
	 * @param bTraceComplex - Complex collision kullan (mesh geometry)
	 * @param ActorsToIgnore - Ignore edilecek actor listesi
	 * @param OutHit - Hit result
	 * @param bDrawDebug - Debug draw enable
	 * @param TraceColor - Trace line rengi
	 * @param HitColor - Hit point rengi
	 * @param DebugDuration - Debug draw süresi
	 * @return True if hit detected
	 */
	UFUNCTION(BlueprintCallable, Category = "Trace Function Library", meta = (WorldContext = "WorldContextObject", AutoCreateRefTerm = "ActorsToIgnore"))
	static bool TraceFromCameraByType(
		const UObject* WorldContextObject,
		APlayerController* Controller,
		ECameraTraceType CameraType,
		float TraceDistance,
		ECollisionChannel TraceChannel,
		bool bTraceComplex,
		const TArray<AActor*>& ActorsToIgnore,
		FHitResult& OutHit,
		bool bDrawDebug = false,
		FLinearColor TraceColor = FLinearColor::Red,
		FLinearColor HitColor = FLinearColor::Green,
		float DebugDuration = 2.0f
	);

	/**
	 * Camera'dan sphere trace yapar - line trace'den daha geniş hit detection
	 * Sphere radius nedeniyle daha forgiving ama biraz daha pahalı
	 * 
	 * @param SphereRadius - Sphere collision yarıçapı
	 */
	UFUNCTION(BlueprintCallable, Category = "Trace Function Library", meta = (WorldContext = "WorldContextObject", AutoCreateRefTerm = "ActorsToIgnore"))
	static bool SphereTraceFromCameraByType(
		const UObject* WorldContextObject,
		APlayerController* Controller,
		ECameraTraceType CameraType,
		float TraceDistance,
		float SphereRadius,
		ECollisionChannel TraceChannel,
		bool bTraceComplex,
		const TArray<AActor*>& ActorsToIgnore,
		FHitResult& OutHit,
		bool bDrawDebug = false,
		FLinearColor TraceColor = FLinearColor::Red,
		FLinearColor HitColor = FLinearColor::Green,
		float DebugDuration = 2.0f
	);

	/**
	 * Camera'dan multi-hit line trace
	 * Performance: Single hit trace'den ~1.5-2x daha yavaş
	 * Penetration veya multi-hit detection gereken durumlarda kullan
	 * 
	 * @param OutHits - Hit results array (reserve edilmiş)
	 */
	UFUNCTION(BlueprintCallable, Category = "Trace Function Library", meta = (WorldContext = "WorldContextObject", AutoCreateRefTerm = "ActorsToIgnore"))
	static bool LineTraceMultiFromCamera(
		const UObject* WorldContextObject,
		APlayerController* Controller,
		ECameraTraceType CameraType,
		float TraceDistance,
		ECollisionChannel TraceChannel,
		bool bTraceComplex,
		const TArray<AActor*>& ActorsToIgnore,
		TArray<FHitResult>& OutHits,
		bool bDrawDebug = false,
		FLinearColor TraceColor = FLinearColor::Red,
		FLinearColor HitColor = FLinearColor::Green,
		float DebugDuration = 2.0f
	);

	/**
	 * Actor'un şeklini kullanarak sweep yapar
	 * Actor'un root component'inin collision shape'ini otomatik detect eder
	 * 
	 * Performance Note: Sweep single hit trace'den ~2-3x daha pahalı
	 * Sadece shape-based collision gerekli olduğunda kullan
	 * 
	 * @param Actor - Source actor (collision shape alınacak)
	 * @param SweepShape - Override shape (None ise actor shape kullanılır)
	 * @param Direction - Sweep direction (normalized)
	 * @param Distance - Sweep distance
	 */
	UFUNCTION(BlueprintCallable, Category = "Trace Function Library", meta = (WorldContext = "WorldContextObject", AutoCreateRefTerm = "ActorsToIgnore"))
	static bool SweepFromActorShape(
		const UObject* WorldContextObject,
		AActor* Actor,
		ETraceShape SweepShape,
		FVector Direction,
		float Distance,
		ECollisionChannel TraceChannel,
		bool bTraceComplex,
		const TArray<AActor*>& ActorsToIgnore,
		FHitResult& OutHit,
		bool bDrawDebug = false,
		FLinearColor TraceColor = FLinearColor::Blue,
		FLinearColor HitColor = FLinearColor::Yellow,
		float DebugDuration = 2.0f
	);

	// ==================== UTILITY TRACE FUNCTIONS ====================
	
	/**
	 * Actor'un altındaki ground'u trace eder
	 * AI navigation, spawning, placement için kullanışlı
	 * 
	 * @param Actor - Start actor
	 * @param TraceDistance - Downward trace distance
	 * @param StartOffset - Actor'dan offset (collision bypass için)
	 */
	UFUNCTION(BlueprintCallable, Category = "Trace Function Library", meta = (WorldContext = "WorldContextObject"))
	static bool TraceGroundBelowActor(
		const UObject* WorldContextObject,
		AActor* Actor,
		float TraceDistance,
		float StartOffset,
		ECollisionChannel TraceChannel,
		bool bTraceComplex,
		FHitResult& OutHit,
		bool bDrawDebug = false,
		float DebugDuration = 2.0f
	);

	/**
	 * Actor'un üstündeki ceiling'i trace eder
	 * Overhead clearance check için
	 */
	UFUNCTION(BlueprintCallable, Category = "Trace Function Library", meta = (WorldContext = "WorldContextObject"))
	static bool TraceCeilingAboveActor(
		const UObject* WorldContextObject,
		AActor* Actor,
		float TraceDistance,
		float StartOffset,
		ECollisionChannel TraceChannel,
		bool bTraceComplex,
		FHitResult& OutHit,
		bool bDrawDebug = false,
		float DebugDuration = 2.0f
	);

	/**
	 * Skeletal mesh socket'ten trace yapar
	 * Weapon muzzle, effect attach points için ideal
	 * 
	 * Performance: Socket query cache'lenir, hot path'de allocation yok
	 * 
	 * @param SkeletalMesh - Source mesh component
	 * @param SocketName - Socket name
	 * @param Direction - Trace direction (socket relative veya world space)
	 * @param bUseSocketRotation - True ise socket rotation kullanılır
	 */
	UFUNCTION(BlueprintCallable, Category = "Trace Function Library", meta = (WorldContext = "WorldContextObject", AutoCreateRefTerm = "ActorsToIgnore"))
	static bool TraceFromSocket(
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
		bool bDrawDebug = false,
		FLinearColor TraceColor = FLinearColor::Red,
		FLinearColor HitColor = FLinearColor::Green,
		float DebugDuration = 2.0f
	);

	// ==================== ADVANCED TRACE FUNCTIONS ====================
	
	/**
	 * Cone-shaped multi trace - vision cone, area scan için
	 * 
	 * Performance Warning: NumRays * Trace cost
	 * Production'da NumRays'i 8-16 arası tut, daha fazlası costly
	 * 
	 * @param StartLocation - Cone apex
	 * @param Direction - Cone direction
	 * @param MaxDistance - Cone length
	 * @param ConeAngle - Cone half-angle (degrees)
	 * @param NumRays - Ray count (daha fazla = daha smooth ama pahalı)
	 */
	UFUNCTION(BlueprintCallable, Category = "Trace Function Library", meta = (WorldContext = "WorldContextObject", AutoCreateRefTerm = "ActorsToIgnore"))
	static bool ConeTraceMulti(
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
		bool bDrawDebug = false,
		FLinearColor TraceColor = FLinearColor::Red,
		FLinearColor HitColor = FLinearColor::Green,
		float DebugDuration = 2.0f
	);

	/**
	 * Random ground point bulur - spawning, AI waypoint için
	 * 
	 * Algorithm: Rejection sampling with radial distribution
	 * Max iterations: 10 (infinite loop prevention)
	 * 
	 * @param CenterLocation - Search center
	 * @param SearchRadius - Horizontal search radius
	 * @param MaxTraceDistance - Vertical trace distance
	 * @param MinGroundAngle - Minimum ground normal angle (slope filter)
	 */
	UFUNCTION(BlueprintCallable, Category = "Trace Function Library", meta = (WorldContext = "WorldContextObject"))
	static bool GetRandomGroundPointNearby(
		const UObject* WorldContextObject,
		FVector CenterLocation,
		float SearchRadius,
		float MaxTraceDistance,
		float MinGroundAngle,
		ECollisionChannel TraceChannel,
		FVector& OutGroundLocation,
		FVector& OutGroundNormal,
		bool bDrawDebug = false,
		float DebugDuration = 2.0f
	);

	// ==================== HELPER FUNCTIONS ====================
	
	/**
	 * Camera trace için start/end point hesaplar
	 * 
	 * Optimization: Bu fonksiyon inline edilip multiple trace'lerde reuse edilebilir
	 * RTS mode'da deproject hesabı en pahalı operation (~0.1ms)
	 * 
	 * @param OutStart - Trace start location
	 * @param OutEnd - Trace end location
	 * @param OutDirection - Normalized trace direction
	 */
	UFUNCTION(BlueprintCallable, Category = "Trace Function Library")
	static bool GetCameraTraceStartEnd(
		APlayerController* Controller,
		ECameraTraceType CameraType,
		float TraceDistance,
		FVector& OutStart,
		FVector& OutEnd,
		FVector& OutDirection
	);

	/**
	 * Debug shape draw - sphere, box, capsule support
	 * 
	 * Note: Debug draw overhead negligible (<0.01ms) but batched draw preferred
	 * Production'da shipping build'de otomatik disabled
	 */
	UFUNCTION(BlueprintCallable, Category = "Trace Function Library", meta = (WorldContext = "WorldContextObject"))
	static void DrawDebugTraceShape(
		const UObject* WorldContextObject,
		ETraceShape Shape,
		FVector Location,
		FRotator Rotation,
		FVector Extent,
		FLinearColor Color,
		float Duration = 2.0f,
		float Thickness = 2.0f
	);

private:
	
	// ==================== INTERNAL HELPERS ====================
	
	/**
	 * Actor'dan collision shape extract eder
	 * Fallback: Sphere shape with bounds radius
	 */
	static bool GetActorCollisionShape(AActor* Actor, FCollisionShape& OutShape, FVector& OutShapeExtent);
	
	/**
	 * Cone pattern için ray directions hesaplar
	 * Spherical coordinate distribution kullanır
	 */
	static void GenerateConeDirections(
		const FVector& CenterDirection,
		float ConeAngleDegrees,
		int32 NumRays,
		TArray<FVector>& OutDirections
	);

	/**
	 * Debug trace query parameters setup
	 */
	static void SetupTraceParams(
		FCollisionQueryParams& OutParams,
		const TArray<AActor*>& ActorsToIgnore,
		bool bTraceComplex,
		const FName& TraceName = NAME_None
	);
};