#pragma once

#include <concepts>
#include <functional>

/** TObj가 UObject 파생인지 제약 */
template<typename T>
concept DerivedFromUObject = std::is_base_of_v<UObject, T>;

/** Func가 TObj*로 호출 가능하고 TResult로 변환 가능한지 제약 */
template<typename Func, typename TObj, typename TResult>
concept ValidGetter = std::is_invocable_v<Func, TObj*>
	&& std::is_convertible_v<std::invoke_result_t<Func, TObj*>, TResult>;

/**
 * Null-safe UObject 값 추출 템플릿
 * - 함수 포인터, 람다, 멤버 포인터 모두 수용
 * - 동적 할당 없음, FORCEINLINE 인라인 보장
 */
template<typename TResult, typename Func, DerivedFromUObject TObj = UObject>
	requires ValidGetter<Func, TObj, TResult>
FORCEINLINE TResult GetFrom(TObj* Obj, Func&& Getter)
{
	return Obj ? static_cast<TResult>(std::invoke(std::forward<Func>(Getter), Obj)) : TResult{};
}
