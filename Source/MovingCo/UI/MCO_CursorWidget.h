#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MCO_CursorWidget.generated.h"

USTRUCT(BlueprintType)
struct FMCO_CursorTextures {
    GENERATED_BODY()

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly) class UPaperSprite* InteractableCursorImage;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly) class UPaperSprite* DefaultCursorImage;
};

UENUM(BlueprintType)
enum class EMCO_CursorMode : uint8 {
    None,
    Interactable
};

UCLASS()
class MOVINGCO_API UMCO_CursorWidget : public UUserWidget
{
	GENERATED_BODY()

public:
    void SetMode(EMCO_CursorMode Mode);

protected:
	virtual void NativeConstruct() override;
    void TrySetImage(UPaperSprite* Sprite);

    UPROPERTY(BlueprintReadWrite, meta=(BindWidget))
    TObjectPtr<class UImage> CursorImage;

    UPROPERTY(EditDefaultsOnly, Category="Cursor")
    FMCO_CursorTextures DefaultCursorTextures;

};
