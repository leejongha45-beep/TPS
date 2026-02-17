#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "Interpolable.generated.h"

UINTERFACE()
class UInterpolable : public UInterface
{
	GENERATED_BODY()
};

/**
 * 보간 Tick 인터페이스
 * - FInterpolateTickFunction에서 호출
 * - 게임 스레드에서 매 프레임 보간 연산 수행
 * - 카메라 보간 등 부드러운 전환이 필요한 컴포넌트가 구현
 */
class TPS_API IInterpolable
{
	GENERATED_BODY()

public:
	/** 매 프레임 보간 처리 (FInterpolateTickFunction에서 호출) */
	virtual void Interpolate_Tick(float DeltaTime) = 0;
};