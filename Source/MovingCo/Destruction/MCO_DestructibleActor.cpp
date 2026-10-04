#include "MCO_DestructibleActor.h"
#include "Engine/World.h"
#include "MCO_DebrisActor.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Engine/StaticMesh.h"

AMCO_DestructibleActor::AMCO_DestructibleActor()
{
	PrimaryActorTick.bCanEverTick = false;

    Bricks = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("Bricks"));
    Bricks->SetMobility(EComponentMobility::Movable);
    RootComponent = Bricks;
}

void AMCO_DestructibleActor::OnConstruction(const FTransform &Transform){
    Super::OnConstruction(Transform);

    Bricks->ClearInstances();

    const FVector Scale = BrickSize / 100.f;
    const FVector Step = BrickSize + FVector(BrickGap);

    for (int32 Row = 0; Row < Rows; ++Row){
        const float RowOffset = (Row % 2 == 1) ? Step.X * 0.5f : 0.f;
        for(int32 Col = 0; Col < Columns; ++Col){
            const FVector Location(Col * Step.X + RowOffset, 0.f, Row*Step.Z + BrickSize.Z * 0.5f);
            Bricks->AddInstance(FTransform(FQuat::Identity, Location, Scale));
        }
    }
}

void AMCO_DestructibleActor::BeginPlay()
{
	Super::BeginPlay();

    BuildPieces();
}

void AMCO_DestructibleActor::BuildPieces(){
    Pieces.Reset();

    const UStaticMesh* Mesh = Bricks->GetStaticMesh();
    if(!Mesh)
    {
        return;
    }

    const int32 Count = Bricks->GetInstanceCount();
    const FBox MeshBox = Mesh->GetBoundingBox();

    TArray<FBox> Boxes;
    Boxes.Reserve(Count);
    Pieces.SetNum(Count);

    for(int32 i=0; i<Count; ++i){
        Bricks->GetInstanceTransform(i, Pieces[i].Transform);
        Boxes.Add(MeshBox.TransformBy(Pieces[i].Transform));

        // Actor origin is on the ground so anything resting on it is an anchor
        Pieces[i].bIsAnchored = Boxes[i].Min.Z <= NeighborTolerance;
    }

    for(int32 A = 0; A < Count; ++A){
        const FBox Expanded = Boxes[A].ExpandBy(NeighborTolerance);

        // Become neighbors if intersecting with other boxes (within the tolerance)
        for(int32 B = A + 1; B < Count; ++B) {
            if(Expanded.Intersect(Boxes[B])){
                Pieces[A].Neighbors.Add(B);
                Pieces[B].Neighbors.Add(A);
            }
        }
    }
}

void AMCO_DestructibleActor::ApplyImpact(int32 PieceIndex, FVector Location, FVector Impulse){
    if(!Pieces.IsValidIndex(PieceIndex) or Pieces[PieceIndex].bIsDetached){
        return;
    }

    FDestructiblePiece& Piece = Pieces[PieceIndex];
    Piece.Damage += Impulse.Size();

    if(Piece.Damage < DetachThreshold){
        return;
    }

    if(AMCO_DebrisActor* Debris = DetachPiece(PieceIndex)){
        Debris->Launch(Impulse, Location);
    }
    DetachUnsupportedPieces();

    Bricks->MarkRenderStateDirty();
}
void AMCO_DestructibleActor::ApplyRadialImpact(FVector Location, float Radius, FVector PushDirection, float ImpulseStrength) {
    TArray<int32> HitPieces = Bricks->GetInstancesOverlappingSphere(Location, Radius);  
    if(HitPieces.IsEmpty())
        return;

    for(const int32 Index : HitPieces){
        if(AMCO_DebrisActor* Debris = DetachPiece(Index)){
            const FVector Impulse = PushDirection.GetSafeNormal() * ImpulseStrength;

            Debris->Launch(Location, Impulse);
        }
    }
    DetachUnsupportedPieces();
    Bricks->MarkRenderStateDirty();
}
void AMCO_DestructibleActor::Reset() {
    Super::Reset();

    for(const TWeakObjectPtr<AMCO_DebrisActor>& Debris : SpawnedDebris)
    {
        if(Debris.IsValid()){
            Debris->Destroy();
        }
    }
    SpawnedDebris.Reset();

    for(int32 i=0; i<Pieces.Num(); ++i){
        FDestructiblePiece& Piece = Pieces[i];
        if(Piece.bIsDetached){
            Bricks->UpdateInstanceTransform(i, Piece.Transform, false, false, true);
        }
        Piece.bIsDetached = false;
        Piece.Damage = 0.f;
    }

    Bricks->MarkRenderStateDirty();
}

AMCO_DebrisActor* AMCO_DestructibleActor::DetachPiece(int32 PieceIndex){
    FDestructiblePiece& Piece = Pieces[PieceIndex];
    // Don't detach a piece that is 'hidden'
    // We technically hide the instances to avoid allocations
    if(Piece.bIsDetached) 
        return nullptr;

    Piece.bIsDetached = true;

    const FTransform Hidden(FQuat::Identity, 
                            Piece.Transform.GetLocation(),
                            FVector::ZeroVector);
    Bricks->UpdateInstanceTransform(PieceIndex, Hidden, false, false, true);

    const FTransform WorldTransform = Piece.Transform * Bricks->GetComponentTransform();

    AMCO_DebrisActor* Debris = GetWorld()->SpawnActor<AMCO_DebrisActor>(AMCO_DebrisActor::StaticClass(), WorldTransform);
    if(Debris){
        Debris->Init(Bricks->GetStaticMesh(), Bricks->GetMaterial(0));
        SpawnedDebris.Add(Debris);
    }
    return Debris;
}

void AMCO_DestructibleActor::DetachUnsupportedPieces(){
    TArray<bool> Supported;
    Supported.Init(false, Pieces.Num());

    TArray<int32> Open;
    for(int32 i = 0; i < Pieces.Num(); ++i){
        if(Pieces[i].bIsAnchored and !Pieces[i].bIsDetached){
            Supported[i] = true;
            Open.Add(i);
        }
    }

    while(Open.Num() > 0){
        const int32 Current = Open.Pop();

        for (const int32 Neigbor : Pieces[Current].Neighbors){
            if(!Supported[Neigbor] and !Pieces[Neigbor].bIsDetached){
                Supported[Neigbor] = true;
                Open.Add(Neigbor);
            }
        }
    }

    for(int32 i = 0; i < Pieces.Num(); ++i){
        if(!Supported[i] and !Pieces[i].bIsDetached){
            DetachPiece(i);
        }
    }
}
