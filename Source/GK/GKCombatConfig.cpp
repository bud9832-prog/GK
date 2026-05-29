// Copyright Ashen Ossuary. All Rights Reserved.

#include "GKCombatConfig.h"

namespace GKCombatConfigDefaults
{
	static FGKComboAttackRow MakeComboRow(
		int32 ComboIndex,
		float Damage,
		float StaminaCost,
		float MotionDuration,
		float HitWindowStart,
		float HitWindowEnd,
		float ComboInputWindowStart,
		float ComboInputWindowEnd)
	{
		FGKComboAttackRow Row;
		Row.ComboIndex = ComboIndex;
		Row.Damage = Damage;
		Row.StaminaCost = StaminaCost;
		Row.MotionDuration = MotionDuration;
		Row.HitWindowStart = HitWindowStart;
		Row.HitWindowEnd = HitWindowEnd;
		Row.ComboInputWindowStart = ComboInputWindowStart;
		Row.ComboInputWindowEnd = ComboInputWindowEnd;
		return Row;
	}
}

UDataTable* UGKCombatConfig::CreateDefaultComboAttackTable(UObject* Outer)
{
	UDataTable* Table = NewObject<UDataTable>(Outer, NAME_None, RF_Transient);
	if (!Table)
	{
		return nullptr;
	}

	Table->RowStruct = FGKComboAttackRow::StaticStruct();

	using namespace GKCombatConfigDefaults;
	Table->AddRow(FName(TEXT("Combo_01")), MakeComboRow(0, 25.f, 22.f, 0.70f, 0.25f, 0.45f, 0.30f, 0.70f));
	Table->AddRow(FName(TEXT("Combo_02")), MakeComboRow(1, 32.f, 24.f, 0.85f, 0.35f, 0.55f, 0.40f, 0.85f));
	Table->AddRow(FName(TEXT("Combo_03")), MakeComboRow(2, 45.f, 28.f, 1.10f, 0.50f, 0.75f, 0.f, 0.f));

	return Table;
}

const FGKComboAttackRow* UGKCombatConfig::GetComboRowByIndex(int32 ComboIndex) const
{
	if (!ComboAttackTable)
	{
		return nullptr;
	}

	for (const TPair<FName, uint8*>& RowPair : ComboAttackTable->GetRowMap())
	{
		const FGKComboAttackRow* Row = reinterpret_cast<const FGKComboAttackRow*>(RowPair.Value);
		if (Row && Row->ComboIndex == ComboIndex)
		{
			return Row;
		}
	}

	return nullptr;
}
