// Fill out your copyright notice in the Description page of Project Settings.


#include "FPSBaseCharacter.h"
#include "MultiFPSPlayerController.h"
#include "Blueprint/UserWidget.h"
#include "Camera/CameraComponent.h"
#include "Components/DecalComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Net/UnrealNetwork.h"
#include "PhysicalMaterials/PhysicalMaterial.h"


// Sets default values
AFPSBaseCharacter::AFPSBaseCharacter()
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

#pragma region component


	//玩家摄像头
	PlayerCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("PlayerCamera"));
	PlayerCamera->SetRelativeLocation(FVector(0.f, 0.f, 70.f));
	PlayerCamera->SetupAttachment(RootComponent);
	PlayerCamera->bUsePawnControlRotation = true;

	//第一人称手臂模型
	FPArmsMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("FPArmsMesh"));
	FPArmsMesh->SetupAttachment(PlayerCamera);
	FPArmsMesh->SetOnlyOwnerSee(true);
	FPArmsMesh->SetCastShadow(false);
	FPArmsMesh->SetRelativeLocation(FVector(-6.f, 0.f, -170.f));
	FPArmsMesh->SetRelativeRotation(FRotator(0.f, -90.f, 0.f));

	//第三人称模型
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	GetMesh()->SetCollisionObjectType(ECC_Pawn);
	GetMesh()->SetRelativeLocation(FVector(0.f, 0.f, -88.f));
	GetMesh()->SetRelativeRotation(FRotator(0.f, -90.f, 0.f));
	GetMesh()->SetOwnerNoSee(true);

	InitializeStatesAndValues();

#pragma endregion
}



#pragma region engine
// Called when the game starts or when spawned
void AFPSBaseCharacter:: BeginPlay()
{
	Super::BeginPlay();

	//伤害回调
	OnTakePointDamage.AddDynamic(this, &AFPSBaseCharacter::OnHit);
	

	// 获得手臂动画蓝图
	ClientArmAnimBP = FPArmsMesh->GetAnimInstance();

	//获得身体动画蓝图
	ServerBodyAnimBP = GetMesh()->GetAnimInstance();

	/** 得到 pawn 的 contorller
	 * 
	 *  TODO！注：在服务器上 Pawn比 Controller先生成，所以一开始得到 controller 为空
	 */
	FPSPlayerController = Cast<AMultiFPSPlayerController> (GetController());
	if (FPSPlayerController)
	{
		UKismetSystemLibrary::PrintString(this, FString::Printf(TEXT("1 %s"),*FPSPlayerController->GetName()));

		//创建用户UI面板
		FPSPlayerController->CreatePlayerUI();

		//初始化生命值UI
		FPSPlayerController->UpdateHealthUI(CurrentHealth, HealthPercent, 0.f);
	}

	/**
	 *TODO! 1.当打包standalone类型时，服务器与客户端一体，而服务器上的controller比pawn生成的晚，因此无法即时获得controller并创建PlayerUI。
	 *TODO！2.延迟获取服务器controller可以解决这个问题
	 *TODO！3.当standalone加入服务器后会更改为Client模式，实际运行情况正常。
	*/
	else
	{
		FTimerHandle TimerHandle;
		GetWorldTimerManager().SetTimer(TimerHandle, FTimerDelegate::CreateLambda([this]()
		{
			
			FPSPlayerController = Cast<AMultiFPSPlayerController> (GetController());
		if (FPSPlayerController)
		{
			UKismetSystemLibrary::PrintString(this, FString::Printf(TEXT("2 %s"),*FPSPlayerController->GetName()));
	
			//创建用户UI面板
			FPSPlayerController->CreatePlayerUI();
	
			//初始化生命值UI
			FPSPlayerController->UpdateHealthUI(CurrentHealth, HealthPercent, 0.f);
		}
		
		}),0.5, false);
	}


	

	//开始时获取武器
	StartWithKindOfWeapon();

}


// Called every frame
void AFPSBaseCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	
}

// Called to bind functionality to input
void AFPSBaseCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	check(PlayerInputComponent);
	//Axis bind
	PlayerInputComponent->BindAxis("MoveForward", this, &AFPSBaseCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &AFPSBaseCharacter::MoveRight);
	PlayerInputComponent->BindAxis("Turn", this, &AFPSBaseCharacter::AddControllerYawInput);
	PlayerInputComponent->BindAxis("LookUp", this, &AFPSBaseCharacter::AddControllerPitchInput);

	//Action bind
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &AFPSBaseCharacter::Jump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &AFPSBaseCharacter::StopJumping);
	PlayerInputComponent->BindAction("LowSpeedWalk", IE_Pressed, this, &AFPSBaseCharacter::LowSpeedWalkAction);
	PlayerInputComponent->BindAction("LowSpeedWalk", IE_Released, this, &AFPSBaseCharacter::NormalSpeedWalkAction);

	PlayerInputComponent->BindAction("Fire", IE_Pressed, this, &AFPSBaseCharacter::InputFirePressed);
	PlayerInputComponent->BindAction("Fire", IE_Released, this, &AFPSBaseCharacter::InputFireReleased);

	PlayerInputComponent->BindAction("Aiming", IE_Pressed, this, &AFPSBaseCharacter::InputAimingPressed);
	PlayerInputComponent->BindAction("Aiming", IE_Released, this, &AFPSBaseCharacter::InputAimingReleased);

	PlayerInputComponent->BindAction("Reload", IE_Pressed, this, &AFPSBaseCharacter::InputReloadPressed);
}

#pragma endregion


#pragma region InputEvents

void AFPSBaseCharacter::MoveForward(float AxisValue)
{
	AddMovementInput(GetActorForwardVector(), AxisValue);
}

void AFPSBaseCharacter::MoveRight(float AxisValue)
{
	AddMovementInput(GetActorRightVector(), AxisValue);
}

void AFPSBaseCharacter::LowSpeedWalkAction()
{
	GetCharacterMovement()->MaxWalkSpeed = 300.f;;
	ServerLowSpeedWalkAction();
}

void AFPSBaseCharacter::NormalSpeedWalkAction()
{
	GetCharacterMovement()->MaxWalkSpeed = 600.f;;
	ServerNormalSpeedWalkAction();
}

void AFPSBaseCharacter::InputFirePressed()
{
	switch (ActiveWeapon)
	{
	case EWeaponType::AK47:
		{
			FireWeaponPrimary();
		}
		break;
	case EWeaponType::M4A1:
		{
			FireWeaponPrimary();
		}
		break;
	case EWeaponType::DesertEagle:
		{
			FireWeaponSecondary();
		}
		break;
	case EWeaponType::Sniper:
		{
			FireWeaponSniper();
		}
		break;
	default:
		break;
	}
}

void AFPSBaseCharacter::InputFireReleased()
{
	switch (ActiveWeapon)
	{
	case EWeaponType::AK47:
		{
			StopFirePrimary();
		}
		break;
	case EWeaponType::M4A1:
		{
			StopFirePrimary();
		}
		break;
	case EWeaponType::DesertEagle:
		{
			StopFireSecondary();
		}
		break;
	case EWeaponType::Sniper:
		{
			StopFireSniper();
		}
		break;
	default:
		break;
	}
}

void AFPSBaseCharacter::InputAimingPressed()
{
	//显示瞄准镜UI, 关闭枪体可见, 摄像头可见距离拉远
	//更改bIsAiming 服务器RPC
	if (ActiveWeapon == EWeaponType::Sniper)
	{
		ServerSetAiming(true);
		ClientStartAiming();
	}
}

void AFPSBaseCharacter::InputAimingReleased()
{
	//关闭瞄准镜UI, 打开枪体可见, 摄像头可见距离恢复
	if (ActiveWeapon == EWeaponType::Sniper)
	{
		ServerSetAiming(false);
		ClientEndAiming();
	}
	
}

void AFPSBaseCharacter::InputReloadPressed()
{
	if (!bIsReloading && !bIsFiring)
	{
		switch (ActiveWeapon)
		{
		case EWeaponType::AK47:
			{
				ServerReloadPrimary();
			}
			break;
		case EWeaponType::M4A1:
			{
				ServerReloadPrimary();
			}
			break;
		case EWeaponType::DesertEagle:
			{
				ServerReloadSecondary();
			}
			break;
		case EWeaponType::Sniper:
			{
				ServerReloadPrimary();
			}
			break;
		default:
			break;
		}
	}
}


#pragma endregion


#pragma region NetWorking
void AFPSBaseCharacter::ServerLowSpeedWalkAction_Implementation()
{
	GetCharacterMovement()->MaxWalkSpeed = 300.f;
}

bool AFPSBaseCharacter::ServerLowSpeedWalkAction_Validate()
{
	return true;
}

void AFPSBaseCharacter::ServerNormalSpeedWalkAction_Implementation()
{
	GetCharacterMovement()->MaxWalkSpeed = 600.f;
}

bool AFPSBaseCharacter::ServerNormalSpeedWalkAction_Validate()
{
	return true;
}

void AFPSBaseCharacter::ServerFireRifleWeapon_Implementation(FVector CameraLocation, FRotator CameraRotation,
	bool IsMoving)
{
	if (ServerPrimaryWeapon)
	{
		//多播(1.必须在服务器调用才能生效 2.谁调谁多播）
		ServerPrimaryWeapon->MultiShootingEffect();
		ServerPrimaryWeapon->ClipCurrentAmmo -= 1;
		
		//播放第三人称蒙太奇动画
		MultiShooting();
		
		//客户端更新UI
		ClientUpdateAmmoUI(ServerPrimaryWeapon->ClipCurrentAmmo, ServerPrimaryWeapon->GunCurrentAmmo);
		
	}

	//射线检测
	RifleLineTrace(CameraLocation, CameraRotation, IsMoving);
	

	//UKismetSystemLibrary::PrintString(this, FString::Printf(TEXT("ServerPrimaryWeapon->ClipCurrentAmmo: %d"),ServerPrimaryWeapon->ClipCurrentAmmo));

}

bool AFPSBaseCharacter::ServerFireRifleWeapon_Validate(FVector CameraLocation, FRotator CameraRotation, bool IsMoving)
{
	return true;
}

void AFPSBaseCharacter::ServerFirePistolWeapon_Implementation(FVector CameraLocation, FRotator CameraRotation,
	bool IsMoving)
{
	if (ServerSecondaryWeapon)
	{
		//多播(1.必须在服务器调用才能生效 2.谁调谁多播）
		ServerSecondaryWeapon->MultiShootingEffect();
		ServerSecondaryWeapon->ClipCurrentAmmo -= 1;
		
		//播放第三人称蒙太奇动画
		MultiShooting();
		
		//客户端更新UI
		ClientUpdateAmmoUI(ServerSecondaryWeapon->ClipCurrentAmmo, ServerSecondaryWeapon->GunCurrentAmmo);

		

		
	}

	//射线检测
	PistolLineTrace(CameraLocation, CameraRotation, IsMoving);
	
	UKismetSystemLibrary::RetriggerableDelay(this, ServerSecondaryWeapon->PistolSpreadCallBackTime, FLatentActionInfo(0, 1, TEXT("DelaySpreadWeaponShootCallBack"), this));
	

	//UKismetSystemLibrary::PrintString(this, FString::Printf(TEXT("ServerSecondaryWeapon->ClipCurrentAmmo: %d"),ServerSecondaryWeapon->ClipCurrentAmmo));

	
}

bool AFPSBaseCharacter::ServerFirePistolWeapon_Validate(FVector CameraLocation, FRotator CameraRotation, bool IsMoving)
{
	return true;
}

void AFPSBaseCharacter::ServerFireSniperWeapon_Implementation(FVector CameraLocation, FRotator CameraRotation,
	bool IsMoving)
{
	if (ServerPrimaryWeapon)
	{
		//多播(1.必须在服务器调用才能生效 2.谁调谁多播）
		ServerPrimaryWeapon->MultiShootingEffect();
		ServerPrimaryWeapon->ClipCurrentAmmo -= 1;
		
		//播放第三人称蒙太奇动画
		MultiShooting();
		
		//客户端更新UI
		ClientUpdateAmmoUI(ServerPrimaryWeapon->ClipCurrentAmmo, ServerPrimaryWeapon->GunCurrentAmmo);
		
	}
	
	if (ClientPrimaryWeapon)
	{
		UKismetSystemLibrary::Delay(this, ClientPrimaryWeapon->ClientArmFireMontage->GetPlayLength(), FLatentActionInfo(0, FMath::Rand(), TEXT("DelaySniperShootCallBack"), this));

	}
	
	bIsFiring = true;
	//射线检测
	SniperLineTrace(CameraLocation, CameraRotation, IsMoving);
	

	//UKismetSystemLibrary::PrintString(this, FString::Printf(TEXT("ServerPrimaryWeapon->ClipCurrentAmmo: %d"),ServerPrimaryWeapon->ClipCurrentAmmo));

}

bool AFPSBaseCharacter::ServerFireSniperWeapon_Validate(FVector CameraLocation, FRotator CameraRotation, bool IsMoving)
{
	return true;
}

void AFPSBaseCharacter::ServerStopFire_Implementation()
{
	bIsFiring = false;
}

bool AFPSBaseCharacter::ServerStopFire_Validate()
{
	return true;
}

void AFPSBaseCharacter::ServerSetAiming_Implementation(bool NewAimingState)
{
	bIsAiming = NewAimingState;
}

bool AFPSBaseCharacter::ServerSetAiming_Validate(bool NewAimingState)
{
	return true;
}

void AFPSBaseCharacter::ServerReloadPrimary_Implementation()
{
	//UKismetSystemLibrary::PrintString(this, FString::Printf(TEXT(__FUNCTION__)));
	if (ServerPrimaryWeapon)
	{
		if (ServerPrimaryWeapon->GunCurrentAmmo > 0 && ServerPrimaryWeapon->ClipCurrentAmmo < ServerPrimaryWeapon->MaxClipAmmo)
		{
			bIsReloading = true;
			//服务器身体多播动画，子弹数据更新
			MultiReload();
			//客户端手臂播放动画，子弹UI更改
			ClientReload();
			if (ClientPrimaryWeapon)
			{
				const FLatentActionInfo LatentInfo(0, FMath::Rand(), TEXT("DelayPlayArmReloadCallBack"), this);
				UKismetSystemLibrary::Delay(this, ClientPrimaryWeapon->ClientArmReloadMontage->GetPlayLength(), LatentInfo);
			}
			
			
		}
	}
	
	
}

bool AFPSBaseCharacter::ServerReloadPrimary_Validate()
{
	return true; 
}

void AFPSBaseCharacter::ServerReloadSecondary_Implementation()
{
	if (ServerSecondaryWeapon)
	{
		if (ServerSecondaryWeapon->GunCurrentAmmo > 0 && ServerSecondaryWeapon->ClipCurrentAmmo < ServerSecondaryWeapon->MaxClipAmmo)
		{
			bIsReloading = true;
			//服务器身体多播动画，子弹数据更新
			MultiReload();
			//客户端手臂播放动画，子弹UI更改
			ClientReload();
			if (ClientSecondaryWeapon)
			{
				const FLatentActionInfo LatentInfo(0, FMath::Rand(), TEXT("DelayPlayArmReloadCallBack"), this);
				UKismetSystemLibrary::Delay(this, ClientSecondaryWeapon->ClientArmReloadMontage->GetPlayLength(), LatentInfo);
			}
			
			
		}
	}
}

bool AFPSBaseCharacter::ServerReloadSecondary_Validate()
{
	return true; 
}


void AFPSBaseCharacter::ClientEquipFPArmsPrimary_Implementation()
{
	if (ServerPrimaryWeapon)
	{
		if (ClientPrimaryWeapon)
		{
		}
		else
		{
			{
				FActorSpawnParameters SpawnInfo;
				SpawnInfo.Owner = this;
				SpawnInfo.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
				
				//生成客户端第一人称武器
				ClientPrimaryWeapon = GetWorld()->SpawnActor<AWeaponBaseClient>(
					ServerPrimaryWeapon->ClientWeaponBaseBPClass,
					GetActorTransform(), SpawnInfo);
				if (ClientPrimaryWeapon)
				{
					//将武器绑定到手臂上
					ClientPrimaryWeapon->AttachToComponent(FPArmsMesh,
						FAttachmentTransformRules::SnapToTargetNotIncludingScale,ClientPrimaryWeapon->AttachArmSocketName);
					
					//根据武器类型更新手臂混合动画
					UpdateFPArmsBlendPose(ClientPrimaryWeapon->FPArmsBlendPose);
				}
				
				//捡起枪时更新一次子弹的UI
				ClientUpdateAmmoUI(ServerPrimaryWeapon->ClipCurrentAmmo, ServerPrimaryWeapon->GunCurrentAmmo);

				
			}
		}
	}
}


void AFPSBaseCharacter::ClientEquipFPArmsSecondary_Implementation()
{
	if (ServerSecondaryWeapon)
	{
		if (ClientSecondaryWeapon)
		{
		}
		else
		{
			{
				FActorSpawnParameters SpawnInfo;
				SpawnInfo.Owner = this;
				SpawnInfo.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
				
				//生成客户端第一人称武器
				ClientSecondaryWeapon = GetWorld()->SpawnActor<AWeaponBaseClient>(
					ServerSecondaryWeapon->ClientWeaponBaseBPClass,
					GetActorTransform(), SpawnInfo);
				if (ClientSecondaryWeapon)
				{
					//将武器绑定到手臂上
					ClientSecondaryWeapon->AttachToComponent(FPArmsMesh,
						FAttachmentTransformRules::SnapToTargetNotIncludingScale,ClientSecondaryWeapon->AttachArmSocketName);
					
					//根据武器类型更新手臂混合动画
					UpdateFPArmsBlendPose(ClientSecondaryWeapon->FPArmsBlendPose);
				}
				
				//捡起枪时更新一次子弹的UI
				ClientUpdateAmmoUI(ServerSecondaryWeapon->ClipCurrentAmmo, ServerSecondaryWeapon->GunCurrentAmmo);

				
			}
		}
	}
	
}

void AFPSBaseCharacter::ClientFire_Implementation()
{
	AWeaponBaseClient* CurrentClientWeapon = GetCurrentClientFPArmsWeapon();
	if (CurrentClientWeapon)
	{
		//枪体播放动画
		CurrentClientWeapon->PlayShootAnimation();

		//手臂播放动画
		UAnimMontage* ClientArmFireMontage = CurrentClientWeapon->ClientArmFireMontage;
		if (ClientArmFireMontage && ClientArmAnimBP)
		{
			ClientArmAnimBP->Montage_Play(ClientArmFireMontage);
		}
		//枪体效果
		CurrentClientWeapon->DisplayWeaponEffect();
		
		if (FPSPlayerController)
		{
			//播放摄像机抖动
			FPSPlayerController->PlayerCameraShake(CurrentClientWeapon->FireCameraShake);

			//播放准星扩散动画
			FPSPlayerController->DoCrossHairRecoil();
		}
	
	}
}
void AFPSBaseCharacter::ClientUpdateAmmoUI_Implementation(int32 ClipCurrentAmmo, int32 GunCurrentAmmo)
{
	if (FPSPlayerController)
	{
		FPSPlayerController->UpdateAmmoUI(ClipCurrentAmmo, GunCurrentAmmo);
	}
	
}

void AFPSBaseCharacter::MultiShooting_Implementation()
{
	const AWeaponBaseServer* CurrentServerWeapon = GetCurrentServerTPBodyWeapon();
	if (CurrentServerWeapon)
	{
		UAnimMontage* ServerBodyFireMontage = CurrentServerWeapon->ServerBodyFireMontage;
		if (ServerBodyAnimBP && ServerBodyFireMontage)
		{
			ServerBodyAnimBP->Montage_Play(ServerBodyFireMontage);
		}
		
	}
}

bool AFPSBaseCharacter::MultiShooting_Validate()
{
	return true;
}

void AFPSBaseCharacter::MultiSpawnBulletDecal_Implementation(FVector Location, FRotator Rotation)
{
	const AWeaponBaseServer* CurrentServerWeapon = GetCurrentServerTPBodyWeapon();
	if (CurrentServerWeapon)
	{
		UDecalComponent* Decal = UGameplayStatics::SpawnDecalAtLocation(GetWorld(), CurrentServerWeapon->BulletDecal,
	FVector(8), Location, Rotation, 10.f);
		if (Decal)
		{
			Decal->SetFadeScreenSize(0.001);
		}
	
	}

	
}

bool AFPSBaseCharacter::MultiSpawnBulletDecal_Validate(FVector Location, FRotator Rotation)
{
	return true;
}



void AFPSBaseCharacter::MultiReload_Implementation()
{
	const AWeaponBaseServer* CurrentServerWeapon = GetCurrentServerTPBodyWeapon();
	if (CurrentServerWeapon)
	{
		//播放枪体换弹动画, 没有合适动画
		//CurrentServerWeapon->PlayReloadAnimation();
		UAnimMontage* BodyReloadMontage = CurrentServerWeapon->ServerBodyReloadMontage;
		if (ServerBodyAnimBP && BodyReloadMontage)
		{
			//播放身体换弹动画
			ServerBodyAnimBP->Montage_Play(BodyReloadMontage);
		}
	
	}
		
}

bool AFPSBaseCharacter::MultiReload_Validate()
{
	return true;
}

void AFPSBaseCharacter::MultiDeathMatchDeath_Implementation()
{
	AWeaponBaseClient* CurrentClientWeapon = GetCurrentClientFPArmsWeapon();
	if (CurrentClientWeapon)
	{
		CurrentClientWeapon->Destroy();
	}
}

bool AFPSBaseCharacter::MultiDeathMatchDeath_Validate()
{
	return true;
}


void AFPSBaseCharacter::ClientUpdateHealthUI_Implementation(float InHealth, float InHealthPercent, float InDamage)
{
	if (FPSPlayerController)
	{
		FPSPlayerController->UpdateHealthUI(InHealth, InHealthPercent, InDamage);
	}
}


void AFPSBaseCharacter::ClientRecoil_Implementation()
{
	if (ServerPrimaryWeapon)
	{
		//计算垂直后坐力
		const UCurveFloat* VerticalRecoilCurve = ServerPrimaryWeapon->VerticalRecoilCurve;
		if (VerticalRecoilCurve)
		{
			OldVerticalRecoilAmount = VerticalRecoilCurve->GetFloatValue(RecoilCurveCoordX);
			RecoilCurveCoordX += 0.1;
			DeltaVerticalRecoilAmount = VerticalRecoilCurve->GetFloatValue(RecoilCurveCoordX) - OldVerticalRecoilAmount;
		}
		//计算水平后坐力
		const UCurveFloat* HorizontalRecoilCurve = ServerPrimaryWeapon->HorizontalRecoilCurve;
		if (HorizontalRecoilCurve)
		{
			OldHorizontalRecoilAmount = HorizontalRecoilCurve->GetFloatValue(RecoilCurveCoordX);
			RecoilCurveCoordX += 0.1;
			DeltaHorizontalRecoilAmount = HorizontalRecoilCurve->GetFloatValue(RecoilCurveCoordX) - OldHorizontalRecoilAmount;
		}
		//用后坐力曲线驱动摄像机旋转
		if (FPSPlayerController)
		{
			const FRotator ControllerRotation = FPSPlayerController->GetControlRotation();
			FPSPlayerController->SetControlRotation(FRotator(
				ControllerRotation.Pitch + DeltaVerticalRecoilAmount,
				ControllerRotation.Yaw + DeltaHorizontalRecoilAmount,
				ControllerRotation.Roll));
		}
	}

}


void AFPSBaseCharacter::ClientReload_Implementation()
{
	AWeaponBaseClient* CurrentClientWeapon = GetCurrentClientFPArmsWeapon();
	if (CurrentClientWeapon)
	{
		UAnimMontage* ClientArmReloadMontage = CurrentClientWeapon->ClientArmReloadMontage;
		if (ClientArmAnimBP && ClientArmReloadMontage)
		{
			ClientArmAnimBP->Montage_Play(ClientArmReloadMontage);
			CurrentClientWeapon->PlayReloadAnimation();
		}
	}

}


void AFPSBaseCharacter::ClientStartAiming_Implementation()
{

	if (FPArmsMesh)
	{
		//隐藏手臂
		FPArmsMesh->SetHiddenInGame(true);
	}
	
	if (ClientPrimaryWeapon)
	{
		//隐藏武器
		ClientPrimaryWeapon->SetActorHiddenInGame(true);
		if (PlayerCamera)
		{
			//设置FOV大小
			PlayerCamera->SetFieldOfView(ClientPrimaryWeapon->FieldOfAimingView);
		}
	}
	//添加瞄准镜UI
	if (SniperScopeBPClass)
	{
		ScopeWidget = CreateWidget<UUserWidget>(GetWorld(), SniperScopeBPClass);
		if (ScopeWidget)
		{
			ScopeWidget->AddToViewport();
		}
	
	}
	
	
}

void AFPSBaseCharacter::ClientEndAiming_Implementation()
{
	if (FPArmsMesh)
	{
		//隐藏手臂
		FPArmsMesh->SetHiddenInGame(false);
	}
	
	if (ClientPrimaryWeapon)
	{
		//隐藏武器
		ClientPrimaryWeapon->SetActorHiddenInGame(false);
		if (PlayerCamera)
		{
			//设置FOV大小
			PlayerCamera->SetFieldOfView(90);
		}
	}
	//添加瞄准镜UI
	if (SniperScopeBPClass)
	{
	
		if (ScopeWidget)
		{
			ScopeWidget->RemoveFromViewport();
		}
	
	}
	
	
}


#pragma endregion


#pragma region Weapon


void AFPSBaseCharacter::DelayPlayArmReloadCallBack()
{
	AWeaponBaseServer* CurrentServerWeapon = GetCurrentServerTPBodyWeapon();
	if (CurrentServerWeapon)
	{
		int32 GunCurrentAmmo = CurrentServerWeapon->GunCurrentAmmo;
		int32 ClipCurrentAmmo = CurrentServerWeapon->ClipCurrentAmmo;
		const int32 MaxClipAmmo = CurrentServerWeapon->MaxClipAmmo;

		//判断枪体剩余子弹是否足够
		if (MaxClipAmmo- ClipCurrentAmmo >= GunCurrentAmmo)
		{
			//填装所有剩余子弹
			ClipCurrentAmmo += GunCurrentAmmo;
			GunCurrentAmmo = 0;
		}
		else
		{
			//填装弹夹所需子弹
			GunCurrentAmmo -= MaxClipAmmo - ClipCurrentAmmo;
			ClipCurrentAmmo = MaxClipAmmo;
		}
		CurrentServerWeapon->GunCurrentAmmo = GunCurrentAmmo;
		CurrentServerWeapon->ClipCurrentAmmo = ClipCurrentAmmo;

		ClientUpdateAmmoUI(ClipCurrentAmmo, GunCurrentAmmo);
		bIsReloading = false;
	}
	
	
}

void AFPSBaseCharacter::EquipPrimary(AWeaponBaseServer* WeaponBaseServer)
{
	if (ServerPrimaryWeapon)
	{
	}
	else
	{
		//Count++;
		//UE_LOG(LogTemp, Warning, TEXT("Has authority? %d. Count: %d"), HasAuthority(),Count)
		ServerPrimaryWeapon = WeaponBaseServer;
		ServerPrimaryWeapon->SetOwner(this);
		ServerPrimaryWeapon->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale,
		ServerPrimaryWeapon->AttachBodySocketName);
		                                 
		//	if (HasAuthority())
		{
			ClientEquipFPArmsPrimary();
		}
	}
}

void AFPSBaseCharacter::EquipSecondary(AWeaponBaseServer* WeaponBaseServer)
{
	if (ServerSecondaryWeapon)
	{
	}
	else
	{
		//Count++;
		//UE_LOG(LogTemp, Warning, TEXT("Has authority? %d. Count: %d"), HasAuthority(),Count)
		ServerSecondaryWeapon = WeaponBaseServer;
		ServerSecondaryWeapon->SetOwner(this);
		ServerSecondaryWeapon->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale,
		ServerSecondaryWeapon->AttachBodySocketName);
		                                 
		//	if (HasAuthority())
		{
			ClientEquipFPArmsSecondary();
		}
	}
}

/**	 
 *	玩家生成武器逻辑：
 *	
 *	1.服务器先生成玩家，调用 StartWithKindOfWeapon >> 购买武器（使用HasAuthority判断，只在服务器上执行） >>
 *	调用EquipPrimary生成武器并且装备 >> 完成服务器武器与服务器玩家绑定
 *	
 *	2.生成服务器武器时，同时调用ClientEquipFPArmsPrimary 在客户端上生成客户端武器 >> 完成客户端武器与客户端玩家绑定，
 *	
 */



void AFPSBaseCharacter::StartWithKindOfWeapon()
{
	//只在服务器上执行
	if (HasAuthority())
	{
		//将枚举转变为随机的int值 用以 生成随机武器
		PurchaseWeapon(static_cast<EWeaponType>(UKismetMathLibrary::RandomIntegerInRange(0, static_cast<int8>(EWeaponType::End) -1 ))); 
	}
}

void AFPSBaseCharacter::PurchaseWeapon(EWeaponType WeaponType)
{
	FActorSpawnParameters SpawnParameters;
	SpawnParameters.Owner = this;
	SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	FString BlueprintClassPath;
	switch (WeaponType)
	{
	case EWeaponType::AK47:
		{
			BlueprintClassPath = TEXT("/Game/Blueprint/Weapon/AK47/ServerBP_AK47.ServerBP_AK47_C");
			ActiveWeapon = EWeaponType::AK47;
			UClass* BlueprintVar = StaticLoadClass(AWeaponBaseServer::StaticClass(), nullptr,*BlueprintClassPath);
			if (BlueprintVar)
			{
				AWeaponBaseServer* ServerWeapon = GetWorld()->SpawnActor<AWeaponBaseServer>(BlueprintVar,
				GetActorTransform(), SpawnParameters);
				ServerWeapon->EquipWeapon();
				EquipPrimary(ServerWeapon);
		
			}
		}
		break;
	case EWeaponType::M4A1:
		{
			BlueprintClassPath = TEXT("/Game/Blueprint/Weapon/M4A1/ServerBP_M4A1.ServerBP_M4A1_C");
			ActiveWeapon = EWeaponType::M4A1;
			UClass* BlueprintVar = StaticLoadClass(AWeaponBaseServer::StaticClass(), nullptr,*BlueprintClassPath);
			if (BlueprintVar)
			{
				AWeaponBaseServer* ServerWeapon = GetWorld()->SpawnActor<AWeaponBaseServer>(BlueprintVar,
				GetActorTransform(), SpawnParameters);
				ServerWeapon->EquipWeapon();
				EquipPrimary(ServerWeapon);
			
			}
		}
		break;
	case EWeaponType::DesertEagle:
		{
			BlueprintClassPath = TEXT("/Game/Blueprint/Weapon/DesertEagle/ServerBP_DesertEagle.ServerBP_DesertEagle_C");
			ActiveWeapon = EWeaponType::DesertEagle;
			UClass* BlueprintVar = StaticLoadClass(AWeaponBaseServer::StaticClass(), nullptr,*BlueprintClassPath);
			if (BlueprintVar)
			{
				AWeaponBaseServer* ServerWeapon = GetWorld()->SpawnActor<AWeaponBaseServer>(BlueprintVar,
				GetActorTransform(), SpawnParameters);
				ServerWeapon->EquipWeapon();
				EquipSecondary(ServerWeapon);
			}
		}
		break;
	case EWeaponType::Sniper:
		{
			BlueprintClassPath = TEXT("/Game/Blueprint/Weapon/Sniper/ServerBP_Sniper.ServerBP_Sniper_C");
			ActiveWeapon = EWeaponType::Sniper;
			UClass* BlueprintVar = StaticLoadClass(AWeaponBaseServer::StaticClass(), nullptr,*BlueprintClassPath);
			if (BlueprintVar)
			{
				AWeaponBaseServer* ServerWeapon = GetWorld()->SpawnActor<AWeaponBaseServer>(BlueprintVar,
				GetActorTransform(), SpawnParameters);
				ServerWeapon->EquipWeapon();
				EquipPrimary(ServerWeapon);
			}
		}
		break;
	default:
		{
		}
		break;
	}
	
}

AWeaponBaseClient* AFPSBaseCharacter::GetCurrentClientFPArmsWeapon()
{
	switch (ActiveWeapon)
	{
	case EWeaponType::AK47:
		{
			return ClientPrimaryWeapon;
		}
		
	case EWeaponType::M4A1:
		{
			return ClientPrimaryWeapon;
		}
	case EWeaponType::DesertEagle:
		{
			return ClientSecondaryWeapon;
		}
	case EWeaponType::Sniper:
		{
			return ClientPrimaryWeapon;
		}
	default:
		break;
	}

	return nullptr;
}

AWeaponBaseServer* AFPSBaseCharacter::GetCurrentServerTPBodyWeapon()
{
	switch (ActiveWeapon)
	{
	case EWeaponType::AK47:
		{
			return ServerPrimaryWeapon;
		}
	case EWeaponType::M4A1:
		{
			return ServerPrimaryWeapon;
		}
	case EWeaponType::DesertEagle:
		{
			return ServerSecondaryWeapon;
		}
	case EWeaponType::Sniper:
		{
			return ServerPrimaryWeapon;
		}
	default:
		break;
	}

	return nullptr;
}


#pragma endregion



#pragma region Fire


void AFPSBaseCharacter::FireWeaponPrimary()
{
	if (ServerPrimaryWeapon)
	{
		//判断当前子弹是否足够
		if (ServerPrimaryWeapon->ClipCurrentAmmo > 0)
		{
			if (!bIsReloading)
			{
				if (ServerPrimaryWeapon->IsAutomatic)
				{
					//开启计时器
					GetWorldTimerManager().SetTimer(AutomaticFireTimerHandle, this, &AFPSBaseCharacter::FirePrimaryInternal,
						ServerPrimaryWeapon->AutomaticFireRate, true, 0);
				}
				//半自动
				else
				{
					FirePrimaryInternal();
				}
				bIsFiring = true;
				//自动 or 半自动 
			}
			//全自动

			else
			{
				UKismetSystemLibrary::PrintString(this, FString::Printf(TEXT("正在换弹中")));
			}
		}
		else
		{
			UKismetSystemLibrary::PrintString(this, FString::Printf(TEXT("弹夹子弹为空")));
		}
	
	
	}
	
}

void AFPSBaseCharacter::StopFirePrimary()
{
	// 清除计时器
	if (AutomaticFireTimerHandle.IsValid())
	{
		GetWorldTimerManager().ClearTimer(AutomaticFireTimerHandle);
	}
	ResetRecoil();

	//更改开火状态
	bIsFiring = false;
}

void AFPSBaseCharacter::FireWeaponSecondary()
{
	
	if (ServerSecondaryWeapon)
	{
		//判断当前子弹是否足够
		if (ServerSecondaryWeapon->ClipCurrentAmmo > 0)
		{
			if (!bIsReloading)
			{
				if (ServerSecondaryWeapon->IsAutomatic)
				{
					//开启计时器
					GetWorldTimerManager().SetTimer(AutomaticFireTimerHandle, this, &AFPSBaseCharacter::FireSecondaryInternal,
						ServerSecondaryWeapon->AutomaticFireRate, true, 0);
				}
				//半自动
				else
				{
					FireSecondaryInternal();
				}
				bIsFiring = true;
				//自动 or 半自动 
			}
			//全自动

			else
			{
				UKismetSystemLibrary::PrintString(this, FString::Printf(TEXT("正在换弹中")));
			}
		}
		else
		{
			UKismetSystemLibrary::PrintString(this, FString::Printf(TEXT("弹夹子弹为空")));
		}
	}
	
	
}

void AFPSBaseCharacter::StopFireSecondary()
{
	// 清除计时器
	if (AutomaticFireTimerHandle.IsValid())
	{
		GetWorldTimerManager().ClearTimer(AutomaticFireTimerHandle);
	}
	//ResetRecoil();

	//更改开火状态
	bIsFiring = false;
}

void AFPSBaseCharacter::FirePrimaryInternal()
{
	if (ServerPrimaryWeapon->ClipCurrentAmmo > 0)
	{
		//检测是否在移动中
		const bool bIsMoving = GetVelocity().Size() > 0.1f;
		
		//服务端 (枪口的闪光效果, 播放射击声音, 减少弹药， 射线检测， 伤害应用， 弹孔印花）
		ServerFireRifleWeapon(PlayerCamera->GetComponentLocation(), PlayerCamera->GetComponentRotation(), bIsMoving);
	
		//客户端 (枪体播放动画，手臂播放动画，播放射击声音，应用屏幕抖动，应用后坐力，枪口的闪光效果）
		//客户端 （十字线的瞄准UI，初始化UI， 射击时准星扩散）
		ClientFire();
	}
	else
	{
		StopFirePrimary();
	}

	ClientRecoil();

}

void AFPSBaseCharacter::FireSecondaryInternal()
{
	if (ServerSecondaryWeapon->ClipCurrentAmmo > 0)
	{
		//检测是否在移动中
		const bool bIsMoving = GetVelocity().Size() > 0.1f;
		
		//服务端 (枪口的闪光效果, 播放射击声音, 减少弹药， 射线检测， 伤害应用， 弹孔印花）
		ServerFirePistolWeapon(PlayerCamera->GetComponentLocation(), PlayerCamera->GetComponentRotation(), bIsMoving);
	
		//客户端 (枪体播放动画，手臂播放动画，播放射击声音，应用屏幕抖动，应用后坐力，枪口的闪光效果）
		//客户端 （十字线的瞄准UI，初始化UI， 射击时准星扩散）
		ClientFire();
		
		
	}
	else
	{
		StopFireSecondary();
	}

	//换新的后坐力方式 ClientRecoil();
}

void AFPSBaseCharacter::FireSniperInternal()
{
	if (ServerPrimaryWeapon->ClipCurrentAmmo > 0)
	{
		//检测是否在移动中
		const bool bIsMoving = GetVelocity().Size() > 0.1f;
		
		//服务端 (枪口的闪光效果, 播放射击声音, 减少弹药， 射线检测， 伤害应用， 弹孔印花）
		ServerFireSniperWeapon(PlayerCamera->GetComponentLocation(), PlayerCamera->GetComponentRotation(), bIsMoving);
	
		//客户端 (枪体播放动画，手臂播放动画，播放射击声音，应用屏幕抖动，应用后坐力，枪口的闪光效果）
		//客户端 （十字线的瞄准UI，初始化UI， 射击时准星扩散）
		ClientFire();
	}
	else
	{
		StopFirePrimary();
	}
	
	
	ClientRecoil();
}

void AFPSBaseCharacter::FireWeaponSniper()
{
	if (ServerPrimaryWeapon)
	{
		//判断当前子弹是否足够
		if (ServerPrimaryWeapon->ClipCurrentAmmo > 0)
		{
			
			if (!bIsReloading && !bIsFiring)
			{
				if (ServerPrimaryWeapon->IsAutomatic)
				{
					//开启计时器
					GetWorldTimerManager().SetTimer(AutomaticFireTimerHandle, this, &AFPSBaseCharacter::FireSniperInternal,
						ServerPrimaryWeapon->AutomaticFireRate, true, 0);
				}
				//半自动
				else
				{
					FireSniperInternal();
				}
	
				//自动 or 半自动 
			}
			//全自动

			else
			{
				UKismetSystemLibrary::PrintString(this, FString::Printf(TEXT("正在换弹中")));
			}
		}
		else
		{
			UKismetSystemLibrary::PrintString(this, FString::Printf(TEXT("弹夹子弹为空")));
		}
	
	
	}
}

void AFPSBaseCharacter::StopFireSniper()
{
	// 清除计时器
	if (AutomaticFireTimerHandle.IsValid())
	{
		GetWorldTimerManager().ClearTimer(AutomaticFireTimerHandle);
	}
	ResetRecoil();


}


void AFPSBaseCharacter::DelaySpreadWeaponShootCallBack()
{
	PistolRecoilMin = 0;
	PistolRecoilMax = 0;

}

void AFPSBaseCharacter::ResetRecoil()
{
	
	OldVerticalRecoilAmount = 0;
	DeltaVerticalRecoilAmount = 0;
	OldHorizontalRecoilAmount = 0;
	DeltaHorizontalRecoilAmount = 0;
	RecoilCurveCoordX = 0;
}


void AFPSBaseCharacter::RifleLineTrace(FVector CameraLocation, FRotator CameraRotation, bool bIsMoving)
{
	FVector EndLocation;
	TArray<AActor*> IgnoreArray;
	IgnoreArray.Add(this);
	FHitResult HitResult;
	
	if (ServerPrimaryWeapon)
	{
		//移动会影响射线检测计算
		if (bIsMoving)
		{
			const FVector Vector = CameraLocation + UKismetMathLibrary::GetForwardVector(CameraRotation) * ServerPrimaryWeapon->BulletDistance;
			const float RandomX = UKismetMathLibrary::RandomFloatInRange(-ServerPrimaryWeapon->MovingFireRandomRange, ServerPrimaryWeapon->MovingFireRandomRange);
			const float RandomY = UKismetMathLibrary::RandomFloatInRange(-ServerPrimaryWeapon->MovingFireRandomRange, ServerPrimaryWeapon->MovingFireRandomRange);
			const float RandomZ = UKismetMathLibrary::RandomFloatInRange(-ServerPrimaryWeapon->MovingFireRandomRange, ServerPrimaryWeapon->MovingFireRandomRange);
	
			EndLocation = FVector(Vector.X + RandomX, Vector.Y + RandomY, Vector.Z + RandomZ);
		
		

		}
		else
		{
			EndLocation = CameraLocation + UKismetMathLibrary::GetForwardVector(CameraRotation) * ServerPrimaryWeapon->BulletDistance;
		}
	}


	const bool bHitSuccess = UKismetSystemLibrary::LineTraceSingle(
		GetWorld(),
		CameraLocation,
		EndLocation,
		ETraceTypeQuery::TraceTypeQuery1,
		false,
		IgnoreArray,
		EDrawDebugTrace::None,
		HitResult,
		true);

	if (bHitSuccess)
	{
		const AFPSBaseCharacter* BaseCharacter = Cast<AFPSBaseCharacter>(HitResult.Actor);
		if (BaseCharacter)
		{
			//打到玩家，应用伤害
			DamagePlayer(HitResult.Actor.Get(), CameraLocation, HitResult);
		}
		else
		{
			//打到墙壁，生成弹孔
			const FVector Location = HitResult.Location;
			const FRotator Rotation = UKismetMathLibrary::MakeRotFromX(HitResult.Normal);
			MultiSpawnBulletDecal(Location, Rotation);
			
		}
		//UKismetSystemLibrary::PrintString(GetWorld(), FString::Printf(TEXT("Hit Actor Name is : %s"), *HitResult.Actor->GetName()));
		
		
	}
}

void AFPSBaseCharacter::PistolLineTrace(FVector CameraLocation, FRotator CameraRotation, bool bIsMoving)
{
	FVector EndLocation;
	TArray<AActor*> IgnoreArray;
	IgnoreArray.Add(this);
	FHitResult HitResult;
	
	if (ServerSecondaryWeapon)
	{
		//移动会影响射线检测计算
		if (bIsMoving)
		{
			FRotator Rotator;
			Rotator.Roll = CameraRotation.Roll;
			Rotator.Pitch = CameraRotation.Pitch + UKismetMathLibrary::RandomFloatInRange(PistolRecoilMin, PistolRecoilMax);
			Rotator.Yaw = CameraRotation.Yaw + UKismetMathLibrary::RandomFloatInRange(PistolRecoilMin, PistolRecoilMax);
			
			const FVector Vector = CameraLocation + UKismetMathLibrary::GetForwardVector(Rotator) * ServerSecondaryWeapon->BulletDistance;
			const float RandomX = UKismetMathLibrary::RandomFloatInRange(-ServerSecondaryWeapon->MovingFireRandomRange, ServerSecondaryWeapon->MovingFireRandomRange);
			const float RandomY = UKismetMathLibrary::RandomFloatInRange(-ServerSecondaryWeapon->MovingFireRandomRange, ServerSecondaryWeapon->MovingFireRandomRange);
			const float RandomZ = UKismetMathLibrary::RandomFloatInRange(-ServerSecondaryWeapon->MovingFireRandomRange, ServerSecondaryWeapon->MovingFireRandomRange);
	
			EndLocation = FVector(Vector.X + RandomX, Vector.Y + RandomY, Vector.Z + RandomZ);
		
		

		}
		else
		{
			FRotator Rotator;
			Rotator.Roll = CameraRotation.Roll;
			Rotator.Pitch = CameraRotation.Pitch + UKismetMathLibrary::RandomFloatInRange(PistolRecoilMin, PistolRecoilMax);
			Rotator.Yaw = CameraRotation.Yaw + UKismetMathLibrary::RandomFloatInRange(PistolRecoilMin, PistolRecoilMax);
			UKismetSystemLibrary::PrintString(this, FString::Printf(TEXT("%f, %f"), PistolRecoilMin, PistolRecoilMax));
			EndLocation = CameraLocation + UKismetMathLibrary::GetForwardVector(Rotator) * ServerSecondaryWeapon->BulletDistance;
		}
	}


	const bool bHitSuccess = UKismetSystemLibrary::LineTraceSingle(
		GetWorld(),
		CameraLocation,
		EndLocation,
		ETraceTypeQuery::TraceTypeQuery1,
		false,
		IgnoreArray,
		EDrawDebugTrace::None,
		HitResult,
		true);

	PistolRecoilMin -= ServerSecondaryWeapon->SpreadWeaponMin;
	PistolRecoilMax += ServerSecondaryWeapon->SpreadWeaponMax;
	
	if (bHitSuccess)
	{
		const AFPSBaseCharacter* BaseCharacter = Cast<AFPSBaseCharacter>(HitResult.Actor);
		if (BaseCharacter)
		{
			//打到玩家，应用伤害
			DamagePlayer(HitResult.Actor.Get(), CameraLocation, HitResult);
		}
		else
		{
			//打到墙壁，生成弹孔
			const FVector Location = HitResult.Location;
			const FRotator Rotation = UKismetMathLibrary::MakeRotFromX(HitResult.Normal);
			MultiSpawnBulletDecal(Location, Rotation);
			
		}
		//UKismetSystemLibrary::PrintString(GetWorld(),FString::Printf(TEXT("Hit Actor Name is : %s"), *HitResult.Actor->GetName()));
		
		
	}
}

void AFPSBaseCharacter::SniperLineTrace(FVector CameraLocation, FRotator CameraRotation, bool bIsMoving)
{
	FVector EndLocation;
	TArray<AActor*> IgnoreArray;
	IgnoreArray.Add(this);
	FHitResult HitResult;
	
	if (ServerPrimaryWeapon)
	{
		//瞄准会影响射线检测
		if (bIsAiming)
		{
			//移动会影响射线检测计算
			if (bIsMoving)
			{
				const FVector Vector = CameraLocation + UKismetMathLibrary::GetForwardVector(CameraRotation) * ServerPrimaryWeapon->BulletDistance;
				const float RandomX = UKismetMathLibrary::RandomFloatInRange(-ServerPrimaryWeapon->MovingFireRandomRange, ServerPrimaryWeapon->MovingFireRandomRange);
				const float RandomY = UKismetMathLibrary::RandomFloatInRange(-ServerPrimaryWeapon->MovingFireRandomRange, ServerPrimaryWeapon->MovingFireRandomRange);
				const float RandomZ = UKismetMathLibrary::RandomFloatInRange(-ServerPrimaryWeapon->MovingFireRandomRange, ServerPrimaryWeapon->MovingFireRandomRange);
	
				EndLocation = FVector(Vector.X + RandomX, Vector.Y + RandomY, Vector.Z + RandomZ);
		
		

			}
			else
			{
				EndLocation = CameraLocation + UKismetMathLibrary::GetForwardVector(CameraRotation) * ServerPrimaryWeapon->BulletDistance;
			}

			ClientEndAiming();
		}
		
		else
		{
			if (bIsMoving)
			{
				const FVector Vector = CameraLocation + UKismetMathLibrary::GetForwardVector(CameraRotation) * ServerPrimaryWeapon->BulletDistance;
				const float RandomX = UKismetMathLibrary::RandomFloatInRange(-ServerPrimaryWeapon->MovingFireRandomRange, ServerPrimaryWeapon->MovingFireRandomRange);
				const float RandomY = UKismetMathLibrary::RandomFloatInRange(-ServerPrimaryWeapon->MovingFireRandomRange, ServerPrimaryWeapon->MovingFireRandomRange);
				const float RandomZ = UKismetMathLibrary::RandomFloatInRange(-ServerPrimaryWeapon->MovingFireRandomRange, ServerPrimaryWeapon->MovingFireRandomRange);
	
				EndLocation = FVector(Vector.X + RandomX, Vector.Y + RandomY, Vector.Z + RandomZ);
				
			}
			else
			{
				const FVector Vector = CameraLocation + UKismetMathLibrary::GetForwardVector(CameraRotation) * ServerPrimaryWeapon->BulletDistance;
				const float RandomX = UKismetMathLibrary::RandomFloatInRange(-ServerPrimaryWeapon->MovingFireRandomRange / 2, ServerPrimaryWeapon->MovingFireRandomRange / 2);
				const float RandomY = UKismetMathLibrary::RandomFloatInRange(-ServerPrimaryWeapon->MovingFireRandomRange / 2, ServerPrimaryWeapon->MovingFireRandomRange / 2);
				const float RandomZ = UKismetMathLibrary::RandomFloatInRange(-ServerPrimaryWeapon->MovingFireRandomRange / 2, ServerPrimaryWeapon->MovingFireRandomRange / 2);
	
				EndLocation = FVector(Vector.X + RandomX, Vector.Y + RandomY, Vector.Z + RandomZ);
			}
		}
	}


	const bool bHitSuccess = UKismetSystemLibrary::LineTraceSingle(
		GetWorld(),
		CameraLocation,
		EndLocation,
		ETraceTypeQuery::TraceTypeQuery1,
		false,
		IgnoreArray,
		EDrawDebugTrace::Persistent,
		HitResult,
		true);

	if (bHitSuccess)
	{
		const AFPSBaseCharacter* BaseCharacter = Cast<AFPSBaseCharacter>(HitResult.Actor);
		if (BaseCharacter)
		{
			//打到玩家，应用伤害
			DamagePlayer(HitResult.Actor.Get(), CameraLocation, HitResult);
		}
		else
		{
			//打到墙壁，生成弹孔
			const FVector Location = HitResult.Location;
			const FRotator Rotation = UKismetMathLibrary::MakeRotFromX(HitResult.Normal);
			MultiSpawnBulletDecal(Location, Rotation);
			
		}
		//UKismetSystemLibrary::PrintString(GetWorld(), FString::Printf(TEXT("Hit Actor Name is : %s"), *HitResult.Actor->GetName()));
	}	
}

void AFPSBaseCharacter::DelaySniperShootCallBack()
{
	//更改开火状态
	//ServerStopFire();
	bIsFiring = false;
	//UKismetSystemLibrary::PrintString(this, FString::Printf(TEXT("正在换弹中 %d" ),bIsFiring));
}


void AFPSBaseCharacter::DamagePlayer(AActor* DamageActor, const FVector& HitFromDirection, const FHitResult& HitResult)
{
	const AWeaponBaseServer* CurrentServerWeapon = GetCurrentServerTPBodyWeapon();
	if (CurrentServerWeapon)
	{
		float ActualDamage = CurrentServerWeapon->BaseDamage;
		{
			switch (HitResult.PhysMaterial->SurfaceType)
			{
			case EPhysicalSurface::SurfaceType1:
				{
					ActualDamage *= 4;
				}
				break;

			case EPhysicalSurface::SurfaceType2:
				{
					ActualDamage *= 1;
				}
				break;

			case EPhysicalSurface::SurfaceType3:
				{
					ActualDamage *= 0.8;
				}
				break;

			case EPhysicalSurface::SurfaceType4:
				{
					ActualDamage *= 0.7;
				}
				break;
				
			default:
				break;
			}
		}

		UGameplayStatics::ApplyPointDamage(DamageActor, ActualDamage, HitFromDirection, HitResult, GetController(),
		this, UDamageType::StaticClass());
	}
	
}


void AFPSBaseCharacter::OnHit(AActor* DamagedActor, float Damage, AController* InstigatedBy, FVector HitLocation,
                              UPrimitiveComponent* FHitComponent, FName BoneName, FVector ShotFromDirection,
                              const UDamageType* DamageType, AActor* DamageCauser)
{
	
	CurrentHealth = FMath::Clamp(CurrentHealth - Damage, 0.f, 100.f);
	UKismetSystemLibrary::PrintString(GetWorld(), FString::Printf(TEXT("Health = %f"), CurrentHealth));
	CalculateHealthPercent();
	ClientUpdateHealthUI(CurrentHealth, HealthPercent, Damage);
	if (CurrentHealth <= 0)
	{
		//死亡逻辑
		DeathMatchDeath(InstigatedBy);
	}
	
	
}

void AFPSBaseCharacter::DeathMatchDeath(AController* InstigatedBy)
{
	//UKismetSystemLibrary::PrintString(GetWorld(), FString::Printf(TEXT(__FUNCTION__)));

	//第三人称武器是复制的，所以可以直接在服务器上删除
	AWeaponBaseServer* CurrentServerWeapon = GetCurrentServerTPBodyWeapon();
	if (CurrentServerWeapon)
	{
		CurrentServerWeapon->Destroy();
	}
	//第一人称服务是不复制的,服务器和客户端的都要删除
	MultiDeathMatchDeath();
	
	// if (FPSPlayerController)
	// {
	// 	FPSPlayerController->DeathMatchDeath(InstigatedBy);
	// }
	//TODO 不能直接使用上面的方式，伤害是从服务器上计算的，一开始获取服务器Controller为空

	AMultiFPSPlayerController* MultiFPSPlayerController = Cast<AMultiFPSPlayerController>(GetController());
	if (MultiFPSPlayerController)
	{
		MultiFPSPlayerController->DeathMatchDeath(InstigatedBy);
	}
	
}


#pragma endregion

void AFPSBaseCharacter::InitializeStatesAndValues()
{
	//生命值初始化
	CurrentHealth = MaxHealth = 100.f;
	//初始化状态
	bIsFiring = false;
	bIsReloading = false;
	bIsAiming = false;
	
}

void AFPSBaseCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AFPSBaseCharacter, bIsFiring);
	DOREPLIFETIME(AFPSBaseCharacter, bIsReloading);
	DOREPLIFETIME(AFPSBaseCharacter, bIsAiming);
	DOREPLIFETIME(AFPSBaseCharacter, ActiveWeapon);
}



void AFPSBaseCharacter::PostInitProperties()
{
	Super::PostInitProperties();
	CalculateHealthPercent();
}

void AFPSBaseCharacter::CalculateHealthPercent()
{
	HealthPercent = CurrentHealth / MaxHealth;
}

#if WITH_EDITOR

void AFPSBaseCharacter::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	CalculateHealthPercent();
	Super::PostEditChangeProperty(PropertyChangedEvent);
	
}

#endif

