#pragma once

#if WITH_EDITOR
#include "CoreMinimal.h"
#include "Editor/UnrealEdEngine.h"

class UUPEdEngine : public UUnrealEdEngine
{
public:
	virtual void Init(IEngineLoop* InEngineLoop) override;
};
#endif
