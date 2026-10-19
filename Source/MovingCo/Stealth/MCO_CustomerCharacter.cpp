#include "MCO_CustomerCharacter.h"
#include "Animation/AnimInstance.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/SkeletalMesh.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "MCO_VisionConeComponent.h"
#include "MCO_WatcherAIController.h"
#include "UObject/ConstructorHelpers.h"

AMCO_CustomerCharacter::AMCO_CustomerCharacter()
{
    constexpr float HalfHeight = 88.f;
    GetCapsuleComponent()->InitCapsuleSize(34.f, HalfHeight);

    bUseControllerRotationYaw = false;
    GetCharacterMovement()->bUseControllerDesiredRotation = true;
    GetCharacterMovement()->RotationRate = FRotator(0.f, 300.f, 0.f);
    GetCharacterMovement()->MaxWalkSpeed = 180.f;

    // Placeholder look until the customer gets its own art
    static ConstructorHelpers::FObjectFinder<USkeletalMesh> MeshAsset(
        TEXT("/Game/Characters/Mannequins/Meshes/SKM_Quinn_Simple.SKM_Quinn_Simple"));
    static ConstructorHelpers::FClassFinder<UAnimInstance> AnimClass(
        TEXT("/Game/Characters/Mannequins/Anims/Unarmed/ABP_Unarmed"));
    if (MeshAsset.Succeeded())
        GetMesh()->SetSkeletalMesh(MeshAsset.Object);
    if (AnimClass.Succeeded())
        GetMesh()->SetAnimInstanceClass(AnimClass.Class);
    GetMesh()->SetRelativeLocationAndRotation(FVector(0.f, 0.f, -HalfHeight), FRotator(0.f, -90.f, 0.f));

    VisionCone = CreateDefaultSubobject<UMCO_VisionConeComponent>(TEXT("Vision Cone"));
    VisionCone->SetupAttachment(GetCapsuleComponent());
    VisionCone->SetRelativeLocation(FVector(0.f, 0.f, -HalfHeight + 2.f));

    AIControllerClass = AMCO_WatcherAIController::StaticClass();
    AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;
}

FVector AMCO_CustomerCharacter::GetPatrolPoint(int32 Index) const
{
    if (!PatrolOrigin.IsSet())
        PatrolOrigin = GetActorTransform();

    return PatrolOrigin->TransformPosition(PatrolPoints[Index]);
}
