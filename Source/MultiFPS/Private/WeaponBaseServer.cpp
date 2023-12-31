// Fill out your copyright notice in the Description page of Project Settings.


#include "WeaponBaseServer.h"
#include "Components/SphereComponent.h"
#include "FPSBaseCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"


// Sets default values
AWeaponBaseServer::AWeaponBaseServer()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	WeaponMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("WeaponMesh"));
	RootComponent = WeaponMesh;
	WeaponMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	WeaponMesh->SetCollisionObjectType(ECC_WorldStatic);
	WeaponMesh->SetOwnerNoSee(true);
	WeaponMesh->SetSimulatePhysics(true);


	SphereCollision = CreateDefaultSubobject<USphereComponent>(TEXT("SphereCollision"));
	SphereCollision->SetupAttachment(RootComponent);
	SphereCollision->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	SphereCollision->OnComponentBeginOverlap.AddDynamic(this, &AWeaponBaseServer::OnBeginOverlap);

	bReplicates = true;

	BulletDistance = 10000;

	BaseDamage = 20;

	AttachBodySocketName = TEXT("Weapon_Rifle");
	
}

// Called when the game starts or when spawned
void AWeaponBaseServer::BeginPlay()
{
	Super::BeginPlay();
	
}


// Called every frame
void AWeaponBaseServer::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}


void AWeaponBaseServer::OnBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor)
	{
		AFPSBaseCharacter* FPSBaseCharacter = Cast<AFPSBaseCharacter>(OtherActor);
		if (FPSBaseCharacter)
		{
			EquipWeapon();
			
			if (KindOfWeapon == EWeaponType::DesertEagle)
			{
				FPSBaseCharacter->EquipSecondary(this);
			}
			else
			{
				//玩家逻辑
				FPSBaseCharacter->EquipPrimary(this);
			}
			
			
			
		}
	}

}


void AWeaponBaseServer::EquipWeapon()
{
	WeaponMesh->SetEnableGravity(false);
	WeaponMesh->SetSimulatePhysics(false);
	SphereCollision->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	
	
}

void AWeaponBaseServer::MultiShootingEffect_Implementation()
{
	if (GetOwner() != UGameplayStatics::GetPlayerPawn(GetWorld(), 0))
	{
		//在其它客户端播放开枪特效
		if (MuzzleFlash)
		{
			//TODO 此处插槽名字没有设置为变量，最好是在骨骼里统一插槽名称
			UGameplayStatics::SpawnEmitterAttached(MuzzleFlash, WeaponMesh, FName("Fire_FX_Slot"));
		}
		//在其它客户端播放一个3D的枪声
		if (FireSound)
		{
			UGameplayStatics::PlaySoundAtLocation(GetWorld(), FireSound, GetActorLocation());
		}
	}
}

bool AWeaponBaseServer::MultiShootingEffect_Validate()
{
	return true;
}

//TODO 不需要在头文件声明，GENERATED_BODY宏会识别添加
void AWeaponBaseServer::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME_CONDITION(AWeaponBaseServer, ClipCurrentAmmo, COND_None);
}
