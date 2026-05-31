// Copyright Ashen Ossuary. All Rights Reserved.
// GKModularArmorComponent.h
// [에이전트 E → A] 아머 비주얼 전담 컴포넌트.
// AGKCharacter(및 하위 BP)에 AddComponent로 추가, 각 갑옷 파트를 SkeletalMeshComponent로 관리.
// 실제 애니메이션·전투 로직과 완전 분리. 컴포넌트 교체만으로 시각 변경 가능.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GKModularArmorComponent.generated.h"

class USkeletalMeshComponent;
class UStaticMeshComponent;

// ─────────────────────────────────────────────────────────────
// 갑옷 파트 열거형 — Blueprint Enum으로 노출
// ─────────────────────────────────────────────────────────────
UENUM(BlueprintType)
enum class EGKArmorSlot : uint8
{
    Helmet      UMETA(DisplayName = "Helmet"),
    Torso       UMETA(DisplayName = "Torso / Chest"),
    PauldronL   UMETA(DisplayName = "Pauldron Left"),
    PauldronR   UMETA(DisplayName = "Pauldron Right"),
    GauntletL   UMETA(DisplayName = "Gauntlet Left"),
    GauntletR   UMETA(DisplayName = "Gauntlet Right"),
    GreaveL     UMETA(DisplayName = "Greave Left"),
    GreaveR     UMETA(DisplayName = "Greave Right"),
    Cloak       UMETA(DisplayName = "Cloak"),
    COUNT       UMETA(Hidden),
};

// ─────────────────────────────────────────────────────────────
// 단일 갑옷 파트 데이터
// ─────────────────────────────────────────────────────────────
USTRUCT(BlueprintType)
struct FGKArmorSlotData
{
    GENERATED_BODY()

    // 이 슬롯에 장착할 스켈레탈 메시
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Armor")
    TSoftObjectPtr<USkeletalMesh> SkeletalMesh;

    // 마스터 스켈레탈 메시에 어태치할 소켓 이름 (예: "helmet_socket", "spine_03_socket")
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Armor")
    FName AttachSocketName = NAME_None;

    // 슬롯별 오프셋 (에디터에서 미세 조정)
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Armor")
    FTransform RelativeTransform = FTransform::Identity;

    // 머티리얼 인스턴스 오버라이드 목록 (슬롯 인덱스 → MI)
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Armor")
    TArray<TSoftObjectPtr<UMaterialInterface>> MaterialOverrides;
};

// ─────────────────────────────────────────────────────────────
// UGKModularArmorComponent
// ─────────────────────────────────────────────────────────────
UCLASS(ClassGroup = (GK), meta = (BlueprintSpawnableComponent),
    DisplayName = "GK Modular Armor Component")
class GK_API UGKModularArmorComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UGKModularArmorComponent();

    // ── 공개 API ────────────────────────────────────────────
    /**
     * 특정 슬롯의 스켈레탈 메시를 런타임에 교체.
     * @param Slot      변경할 갑옷 슬롯
     * @param NewMesh   교체할 메시 (nullptr 허용 — 해당 파트 숨김)
     */
    UFUNCTION(BlueprintCallable, Category = "GK|Armor")
    void SwapArmorPiece(EGKArmorSlot Slot, USkeletalMesh* NewMesh);

    /** 슬롯에 해당하는 SkeletalMeshComponent 반환 (nullptr 가능). */
    UFUNCTION(BlueprintPure, Category = "GK|Armor")
    USkeletalMeshComponent* GetSlotComponent(EGKArmorSlot Slot) const;

    /** 망토 가시성 전환 (HitReaction / 컷씬용). */
    UFUNCTION(BlueprintCallable, Category = "GK|Armor")
    void SetCloakVisible(bool bVisible);

protected:
    virtual void BeginPlay() override;

    // ── 에디터 노출 — 각 슬롯 초기 데이터 ─────────────────
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GK|Armor|Slots",
        meta = (TitleProperty = "AttachSocketName"))
    TMap<EGKArmorSlot, FGKArmorSlotData> ArmorSlots;

private:
    // 런타임 생성 컴포넌트 맵 (Slot → SMC)
    TMap<EGKArmorSlot, TObjectPtr<USkeletalMeshComponent>> RuntimeComponents;

    /** 부모 액터의 기준 SkeletalMeshComponent (캐릭터 메인 메시) 조회. */
    USkeletalMeshComponent* GetOwnerMainMesh() const;

    /** ArmorSlots 테이블을 순회해 SkeletalMeshComponent 스폰. BeginPlay에서 호출. */
    void SpawnArmorComponents();

    /** 메시 설정 + 머티리얼 오버라이드 적용. */
    void ApplyMeshAndMaterials(USkeletalMeshComponent* Component, const FGKArmorSlotData& SlotData) const;
};
