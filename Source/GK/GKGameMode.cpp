// Copyright Ashen Ossuary. All Rights Reserved.

#include "GKGameMode.h"
#include "GKCharacter.h"

AGKGameMode::AGKGameMode()
{
	DefaultPawnClass = AGKCharacter::StaticClass();
}
