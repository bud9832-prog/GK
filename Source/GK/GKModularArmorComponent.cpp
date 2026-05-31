// Copyright Ashen Ossuary. All Rights Reserved.
// GKModularArmorComponent.cpp

#include "GKModularArmorComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/Character.h"
#include "Engine/SkeletalMesh.h"
#include "Materials/MaterialInterface.h"

UGKModularArmorComponent::UGKModularArmorComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
    bAutoActivate = true;
}

void UGKModularArmorComponent::BeginPlay()
{
    Super::BeginPlay();
    SpawnArmorComponents();
}

// ─────────────────────────────────────────────────────────────
// 내부: 부모 캐릭터 기준 메쉬 조회
// ─────────────────────────────────────────────────────────────
USkeletalMeshComponent* UGKModularArmorComponent::GetOwnerMainMesh() const
{
    const ACharacter* Character = Cast<ACharacter>(GetOwner());
    if (!Character)
    {
        UE_LOG(LogTemp, Warning, TEXT("[GKArmor] Owner가 ACharacter가 아님: %s"), *GetNameSafe(GetOwner()));
        return nullptr;
    }
    return Character->GetMesh();
}

// ─────────────────────────────────────────────────────────────
// 내부: ArmorSlots 테이블 순회 → SkeletalMeshComponent 스폰
// ─────────────────────────────────────────────────────────────
void UGKModularArmorComponent::SpawnArmorComponents()
{
    USkeletalMeshComponent* BaseMesh = GetOwnerMainMesh();
    if (!BaseMesh)
    {
        UE_LOG(LogTemp, Error, TEXT("[GKArmor] BaseMesh 없음 — SpawnArmorComponents 중단"));
        return;
    }

    AActor* Owner = GetOwner();
    for (auto& [Slot, SlotData] : ArmorSlots)
    {
        if (RuntimeComponents.Contains(Slot))
        {
            continue; // 중복 스폰 방지
        }

        // 컴포넌트 이름 = Slot 열거형 이름 + "_SMC"
        const FString SlotEnumName = UEnum::GetValueAsString(Slot);
        const FName   CompName     = *FString::Printf(TEXT("%s_SMC"), *SlotEnumName);

        USkeletalMeshComponent* ArmorSMC = NewObject<USkeletalMeshComponent>(Owner, CompName);
        if (!ArmorSMC)
        {
            UE_LOG(LogTemp, Warning, TEXT("[GKArmor] SMC 생성 실패: %s"), *CompName.ToString());
            continue;
        }

        // 등록 + 부모에 어태치
        ArmorSMC->RegisterComponent();

        const FName SocketName = SlotData.AttachSocketName.IsNone()
                                 ? USkeletalMeshComponent::SocketName
                                 : SlotData.AttachSocketName;

        ArmorSMC->AttachToComponent(BaseMesh, FAttachmentTransformRules::SnapToTargetNotIncludingScale, SocketName);
        ArmorSMC->SetRelativeTransform(SlotData.RelativeTransform);

        // 메인 메시 애니메이션과 동기화 (Leader Pose)
        ArmorSMC->SetLeaderPoseComponent(BaseMesh);

        // 메시 + 머티리얼 적용
        ApplyMeshAndMaterials(ArmorSMC, SlotData);

        RuntimeComponents.Add(Slot, ArmorSMC);

        UE_LOG(LogTemp, Log, TEXT("[GKArmor] 슬롯 스폰: %s  소켓: %s"),
            *CompName.ToString(), *SocketName.ToString());
    }
}

// ─────────────────────────────────────────────────────────────
// 내부: 메시 로드 + 머티리얼 오버라이드 적용
// ─────────────────────────────────────────────────────────────
void UGKModularArmorComponent::ApplyMeshAndMaterials(
    USkeletalMeshComponent* Component,
    const FGKArmorSlotData& SlotData) const
{
    if (!Component)
    {
        return;
    }

    // Soft 레퍼런스 동기 로드 (에디터 환경 / PIE)
    if (!SlotData.SkeletalMesh.IsNull())
    {
        USkeletalMesh* Mesh = SlotData.SkeletalMesh.LoadSynchronous();
        Component->SetSkeletalMesh(Mesh);
    }

    // 머티리얼 인스턴스 오버라이드
    for (int32 MatIdx = 0; MatIdx < SlotData.MaterialOverrides.Num(); ++MatIdx)
    {
        if (!SlotData.MaterialOverrides[MatIdx].IsNull())
        {
            UMaterialInterface* MI = SlotData.MaterialOverrides[MatIdx].LoadSynchronous();
            Component->SetMaterial(MatIdx, MI);
        }
    }
}

// ─────────────────────────────────────────────────────────────
// 공개: 런타임 갑옷 파트 교체
// ─────────────────────────────────────────────────────────────
void UGKModularArmorComponent::SwapArmorPiece(EGKArmorSlot Slot, USkeletalMesh* NewMesh)
{
    TObjectPtr<USkeletalMeshComponent>* ComponentPtr = RuntimeComponents.Find(Slot);
    if (!ComponentPtr || !(*ComponentPtr))
    {
        UE_LOG(LogTemp, Warning, TEXT("[GKArmor] 슬롯 컴포넌트 없음: %s"), *UEnum::GetValueAsString(Slot));
        return;
    }

    USkeletalMeshComponent* Component = ComponentPtr->Get();
    Component->SetSkeletalMesh(NewMesh);

    if (!NewMesh)
    {
        Component->SetVisibility(false);
    }
    else
    {
        Component->SetVisibility(true);
        // LeaderPose 재연결 (메시 교체 시 풀릴 수 있음)
        if (USkeletalMeshComponent* BaseMesh = GetOwnerMainMesh())
        {
            Component->SetLeaderPoseComponent(BaseMesh);
        }
    }

    UE_LOG(LogTemp, Log, TEXT("[GKArmor] 슬롯 교체: %s → %s"),
        *UEnum::GetValueAsString(Slot), *GetNameSafe(NewMesh));
}

USkeletalMeshComponent* UGKModularArmorComponent::GetSlotComponent(EGKArmorSlot Slot) const
{
    if (const TObjectPtr<USkeletalMeshComponent>* Ptr = RuntimeComponents.Find(Slot))
    {
        return Ptr->Get();
    }
    return nullptr;
}

void UGKModularArmorComponent::SetCloakVisible(bool bVisible)
{
    if (USkeletalMeshComponent* CloakSMC = GetSlotComponent(EGKArmorSlot::Cloak))
    {
        CloakSMC->SetVisibility(bVisible);
    }
}
