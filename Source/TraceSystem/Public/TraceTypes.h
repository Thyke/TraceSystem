#pragma once

#include "CoreMinimal.h"
#include "TraceTypes.generated.h"

/**
 * Camera trace tipi - farklı oyun türleri için trace başlangıç noktası belirleme
 * FPS: Camera viewpoint'ten trace
 * ThirdPerson: Pawn location'dan trace
 * RTS: Mouse cursor deprojection'dan trace
 */
UENUM(BlueprintType)
enum class ECameraTraceType : uint8
{
	FPS           UMETA(DisplayName = "FPS"),
	ThirdPerson   UMETA(DisplayName = "Third Person"),
	RTS           UMETA(DisplayName = "RTS")
};

/**
 * Sweep shape tipi - collision shape seçimi
 */
UENUM(BlueprintType)
enum class ETraceShape : uint8
{
	Sphere        UMETA(DisplayName = "Sphere"),
	Box           UMETA(DisplayName = "Box"),
	Capsule       UMETA(DisplayName = "Capsule")
};