// Copyright Ashen Ossuary. All Rights Reserved.

#include "GKAnimMontageTypes.h"
#include "Animation/AnimMontage.h"

namespace GKAnimMontageTableDefaults
{
	static void AssignPlaceholderMontage(FGKAnimMontageRow& Row, const TCHAR* AssetPath)
	{
		Row.Montage = TSoftObjectPtr<UAnimMontage>(FSoftObjectPath(AssetPath));
	}

	static FGKAnimMontageRow MakePCRow(EGKAnimAction Action, int32 Variant, const FString& Notes)
	{
		FGKAnimMontageRow Row;
		Row.OwnerType = EGKAnimOwner::PC;
		Row.EnemyType = EGKEnemyType::None;
		Row.ActionTag = Action;
		Row.ActionVariant = Variant;
		Row.Notes = Notes;
		return Row;
	}

	static FGKAnimMontageRow MakeEnemyRow(
		EGKEnemyType EnemyType, EGKAnimAction Action, int32 Variant, const FString& Notes)
	{
		FGKAnimMontageRow Row;
		Row.OwnerType = EGKAnimOwner::Enemy;
		Row.EnemyType = EnemyType;
		Row.ActionTag = Action;
		Row.ActionVariant = Variant;
		Row.Notes = Notes;
		return Row;
	}
}

bool FGKAnimMontageTableHelpers::AnimActionRequiresVariant(EGKAnimAction Action)
{
	return Action == EGKAnimAction::Attack_Combo;
}

FName FGKAnimMontageTableHelpers::GetPCActionKeyName(EGKAnimAction Action)
{
	switch (Action)
	{
	case EGKAnimAction::Idle: return TEXT("Idle");
	case EGKAnimAction::Run: return TEXT("Run");
	case EGKAnimAction::Walk: return TEXT("Walk");
	case EGKAnimAction::HitStun: return TEXT("HitStun");
	case EGKAnimAction::Death: return TEXT("Death");
	case EGKAnimAction::Sprint: return TEXT("Sprint");
	case EGKAnimAction::Jump: return TEXT("Jump");
	case EGKAnimAction::JumpAttack: return TEXT("JumpAttack");
	case EGKAnimAction::Attack_Combo: return TEXT("Attack_Combo");
	case EGKAnimAction::Evade: return TEXT("Evade");
	case EGKAnimAction::Heal: return TEXT("Heal");
	case EGKAnimAction::HeavyAttack: return TEXT("HeavyAttack");
	case EGKAnimAction::Parry_Active: return TEXT("Parry_Active");
	case EGKAnimAction::Parry_Recovery: return TEXT("Parry_Recovery");
	default: return NAME_None;
	}
}

FName FGKAnimMontageTableHelpers::GetEnemyTypeKeyName(EGKEnemyType EnemyType)
{
	switch (EnemyType)
	{
	case EGKEnemyType::FirstEnemy: return TEXT("FirstEnemy");
	default: return NAME_None;
	}
}

FName FGKAnimMontageTableHelpers::GetEnemyActionKeyName(EGKAnimAction Action)
{
	switch (Action)
	{
	case EGKAnimAction::Idle: return TEXT("Idle");
	case EGKAnimAction::Walk: return TEXT("Walk");
	case EGKAnimAction::Death: return TEXT("Death");
	case EGKAnimAction::Enemy_Attack: return TEXT("Attack");
	case EGKAnimAction::Enemy_Sway: return TEXT("Sway");
	case EGKAnimAction::Enemy_Down: return TEXT("Down");
	default: return NAME_None;
	}
}

FName FGKAnimMontageTableHelpers::BuildPCAnimRowName(EGKAnimAction Action, int32 Variant)
{
	const FName ActionKey = GetPCActionKeyName(Action);
	if (ActionKey.IsNone())
	{
		return NAME_None;
	}

	if (AnimActionRequiresVariant(Action))
	{
		return FName(*FString::Printf(TEXT("PC.%s.%d"), *ActionKey.ToString(), Variant));
	}

	return FName(*FString::Printf(TEXT("PC.%s"), *ActionKey.ToString()));
}

FName FGKAnimMontageTableHelpers::BuildEnemyAnimRowName(
	EGKEnemyType EnemyType, EGKAnimAction Action, int32 Variant)
{
	const FName EnemyKey = GetEnemyTypeKeyName(EnemyType);
	const FName ActionKey = GetEnemyActionKeyName(Action);
	if (EnemyKey.IsNone() || ActionKey.IsNone())
	{
		return NAME_None;
	}

	if (AnimActionRequiresVariant(Action))
	{
		return FName(*FString::Printf(TEXT("Enemy.%s.%s.%d"), *EnemyKey.ToString(), *ActionKey.ToString(), Variant));
	}

	return FName(*FString::Printf(TEXT("Enemy.%s.%s"), *EnemyKey.ToString(), *ActionKey.ToString()));
}

const FGKAnimMontageRow* FGKAnimMontageTableHelpers::FindPCRow(
	const UDataTable* Table, EGKAnimAction Action, int32 Variant)
{
	if (!Table)
	{
		return nullptr;
	}

	const FName RowName = BuildPCAnimRowName(Action, Variant);
	if (RowName.IsNone())
	{
		return nullptr;
	}

	const FGKAnimMontageRow* Row = Table->FindRow<FGKAnimMontageRow>(RowName, TEXT("FindPCRow"));
	if (!Row || Row->OwnerType != EGKAnimOwner::PC || Row->ActionTag != Action || Row->ActionVariant != Variant)
	{
		return nullptr;
	}

	return Row;
}

const FGKAnimMontageRow* FGKAnimMontageTableHelpers::FindEnemyRow(
	const UDataTable* Table, EGKEnemyType EnemyType, EGKAnimAction Action, int32 Variant)
{
	if (!Table)
	{
		return nullptr;
	}

	const FName RowName = BuildEnemyAnimRowName(EnemyType, Action, Variant);
	if (RowName.IsNone())
	{
		return nullptr;
	}

	const FGKAnimMontageRow* Row = Table->FindRow<FGKAnimMontageRow>(RowName, TEXT("FindEnemyRow"));
	if (!Row || Row->OwnerType != EGKAnimOwner::Enemy || Row->EnemyType != EnemyType
		|| Row->ActionTag != Action || Row->ActionVariant != Variant)
	{
		return nullptr;
	}

	return Row;
}

UAnimMontage* FGKAnimMontageTableHelpers::ResolveMontage(const FGKAnimMontageRow* Row)
{
	if (!Row || Row->Montage.IsNull())
	{
		return nullptr;
	}

	return Row->Montage.LoadSynchronous();
}

UDataTable* FGKAnimMontageTableHelpers::CreateDefaultAnimMontageTable(UObject* Outer)
{
	UDataTable* Table = NewObject<UDataTable>(Outer, NAME_None, RF_Transient);
	if (!Table)
	{
		return nullptr;
	}

	Table->RowStruct = FGKAnimMontageRow::StaticStruct();

	using namespace GKAnimMontageTableDefaults;

	static const TCHAR* MontageDash = TEXT("/Game/Characters/Mannequins/Anims/Unarmed/Jump/MM_Dash.MM_Dash");
	static const TCHAR* MontageJump = TEXT("/Game/Characters/Mannequins/Anims/Unarmed/Jump/MM_Jump.MM_Jump");
	static const TCHAR* MontageIdle = TEXT("/Game/Characters/Mannequins/Anims/Unarmed/MM_Idle.MM_Idle");

	Table->AddRow(TEXT("PC.Idle"), MakePCRow(EGKAnimAction::Idle, 0, TEXT("Pillars idle")));
	Table->AddRow(TEXT("PC.Walk"), MakePCRow(EGKAnimAction::Walk, 0, TEXT("Stage 1 reserved")));
	Table->AddRow(TEXT("PC.Run"), MakePCRow(EGKAnimAction::Run, 0, TEXT("Default locomotion")));
	Table->AddRow(TEXT("PC.Sprint"), MakePCRow(EGKAnimAction::Sprint, 0, TEXT("Sprint locomotion")));
	Table->AddRow(TEXT("PC.Jump"), MakePCRow(EGKAnimAction::Jump, 0, TEXT("Jump set placeholder")));
	Table->AddRow(TEXT("PC.JumpAttack"), MakePCRow(EGKAnimAction::JumpAttack, 0, TEXT("Jump attack")));

	{
		FGKAnimMontageRow Row = MakePCRow(EGKAnimAction::Attack_Combo, 0, TEXT("Combo 1"));
		AssignPlaceholderMontage(Row, MontageDash);
		Table->AddRow(TEXT("PC.Attack_Combo.0"), Row);
	}
	{
		FGKAnimMontageRow Row = MakePCRow(EGKAnimAction::Attack_Combo, 1, TEXT("Combo 2"));
		AssignPlaceholderMontage(Row, MontageJump);
		Table->AddRow(TEXT("PC.Attack_Combo.1"), Row);
	}
	{
		FGKAnimMontageRow Row = MakePCRow(EGKAnimAction::Attack_Combo, 2, TEXT("Combo 3"));
		AssignPlaceholderMontage(Row, MontageDash);
		Table->AddRow(TEXT("PC.Attack_Combo.2"), Row);
	}
	{
		FGKAnimMontageRow Row = MakePCRow(EGKAnimAction::Evade, 0, TEXT("Evade motion"));
		AssignPlaceholderMontage(Row, MontageDash);
		Table->AddRow(TEXT("PC.Evade"), Row);
	}
	{
		FGKAnimMontageRow Row = MakePCRow(EGKAnimAction::Heal, 0, TEXT("Heal item drink"));
		AssignPlaceholderMontage(Row, MontageIdle);
		Table->AddRow(TEXT("PC.Heal"), Row);
	}

	Table->AddRow(TEXT("PC.HitStun"), MakePCRow(EGKAnimAction::HitStun, 0, TEXT("Hit reaction")));
	Table->AddRow(TEXT("PC.Death"), MakePCRow(EGKAnimAction::Death, 0, TEXT("Player death")));

	{
		FGKAnimMontageRow Row = MakePCRow(EGKAnimAction::HeavyAttack, 0, TEXT("Skill 01B heavy"));
		AssignPlaceholderMontage(Row, MontageJump);
		Table->AddRow(TEXT("PC.HeavyAttack"), Row);
	}
	{
		FGKAnimMontageRow Row = MakePCRow(EGKAnimAction::Parry_Active, 0, TEXT("Parry active window"));
		AssignPlaceholderMontage(Row, MontageIdle);
		Table->AddRow(TEXT("PC.Parry_Active"), Row);
	}
	{
		FGKAnimMontageRow Row = MakePCRow(EGKAnimAction::Parry_Recovery, 0, TEXT("Parry recovery"));
		AssignPlaceholderMontage(Row, MontageIdle);
		Table->AddRow(TEXT("PC.Parry_Recovery"), Row);
	}

	{
		FGKAnimMontageRow Row = MakeEnemyRow(EGKEnemyType::FirstEnemy, EGKAnimAction::Idle, 0, TEXT("Enemy idle"));
		AssignPlaceholderMontage(Row, MontageIdle);
		Table->AddRow(TEXT("Enemy.FirstEnemy.Idle"), Row);
	}
	Table->AddRow(TEXT("Enemy.FirstEnemy.Walk"),
		MakeEnemyRow(EGKEnemyType::FirstEnemy, EGKAnimAction::Walk, 0, TEXT("Enemy walk")));
	{
		FGKAnimMontageRow Row = MakeEnemyRow(EGKEnemyType::FirstEnemy, EGKAnimAction::Enemy_Attack, 0, TEXT("Enemy attack"));
		AssignPlaceholderMontage(Row, MontageDash);
		Table->AddRow(TEXT("Enemy.FirstEnemy.Attack"), Row);
	}
	Table->AddRow(TEXT("Enemy.FirstEnemy.Sway"),
		MakeEnemyRow(EGKEnemyType::FirstEnemy, EGKAnimAction::Enemy_Sway, 0, TEXT("Enemy sway")));
	Table->AddRow(TEXT("Enemy.FirstEnemy.Down"),
		MakeEnemyRow(EGKEnemyType::FirstEnemy, EGKAnimAction::Enemy_Down, 0, TEXT("Enemy down")));
	Table->AddRow(TEXT("Enemy.FirstEnemy.Death"),
		MakeEnemyRow(EGKEnemyType::FirstEnemy, EGKAnimAction::Death, 0, TEXT("Enemy death")));

	return Table;
}
