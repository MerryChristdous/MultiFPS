// Fill out your copyright notice in the Description page of Project Settings.


#include "WeaponBaseClient.h"
#include "Kismet/GameplayStatics.h"



// Sets default values
AWeaponBaseClient::AWeaponBaseClient()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	WeaponMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("WeaponMesh"));
	WeaponMesh->SetOnlyOwnerSee(true);
	WeaponMesh->SetCastShadow(false);
	RootComponent = WeaponMesh;

	AttachArmSocketName = TEXT("WeaponSocket");
	
}

// Called when the game starts or when spawned
void AWeaponBaseClient::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AWeaponBaseClient::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AWeaponBaseClient::DisplayWeaponEffect()
{
	//播放枪体声音
	if (FireSound)
	{
		UGameplayStatics::PlaySound2D(GetWorld(), FireSound);
	}
	//显示枪口火焰
	if (MuzzleFlash)
	{
		UGameplayStatics::SpawnEmitterAttached(MuzzleFlash, WeaponMesh, FName("Fire_FX_Slot"));
	}
	
}

