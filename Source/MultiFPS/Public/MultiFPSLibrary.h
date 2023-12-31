// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "MultiFPSLibrary.generated.h"

/**
 * 
 */

USTRUCT(BlueprintType)
struct FDeathMatchPlayerData
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite)
	FName PlayerName;

	UPROPERTY(BlueprintReadWrite)
	int32 PlayerScore;

	FDeathMatchPlayerData()
	{
		PlayerName = TEXT(" ");
		PlayerScore = 0;
	}
	
};


UCLASS()
class MULTIFPS_API UMultiFPSLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()


	//玩家分数排序
	UFUNCTION(BlueprintCallable, Category = "Sort")
	static void SortValues(UPARAM(ref)TArray<FDeathMatchPlayerData>& InValues);

	//手写快排
	static TArray<FDeathMatchPlayerData>& QuickSort(TArray<FDeathMatchPlayerData>& InValues, int Start, int End);
	
};
