// Fill out your copyright notice in the Description page of Project Settings.


#include "MultiFPSLibrary.h"

void UMultiFPSLibrary::SortValues(TArray<FDeathMatchPlayerData>& InValues)
{
	if (InValues.Num())
	{
		QuickSort(InValues, 0, InValues.Num()-1);
	}

}

TArray<FDeathMatchPlayerData>& UMultiFPSLibrary::QuickSort(TArray<FDeathMatchPlayerData>& InValues, int Start, int End)
{
	if (Start >= End)
	{
		return InValues;
	}
	int i = Start, j = End;
	const FDeathMatchPlayerData Temp = InValues[i];
	while (i != j)
	{
		while (j > i && InValues[j].PlayerScore <= Temp.PlayerScore)
		{
			j--;
		}
		InValues[i] = InValues[j];
		while (j > i && InValues[i].PlayerScore >= Temp.PlayerScore)
		{
			i++;
		}
		InValues[j] = InValues[j];
	}
	InValues[i] = Temp;
	QuickSort(InValues, Start, i-1);
	QuickSort(InValues, i+1, End);
	return InValues;
	
}
