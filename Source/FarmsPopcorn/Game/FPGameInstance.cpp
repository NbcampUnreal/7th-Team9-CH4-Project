// Fill out your copyright notice in the Description page of Project Settings.


#include "FPGameInstance.h"

#include "UI/FPUIManagerSubsystem.h"

UFPGameInstance::UFPGameInstance()
{
	
}

void UFPGameInstance::Init()
{
	Super::Init();
	UFPUIManagerSubsystem* Storage = GetSubsystem<UFPUIManagerSubsystem>();
	if (Storage)
	{
		SaveNickName = Storage->SavedNickName;
	}
}
