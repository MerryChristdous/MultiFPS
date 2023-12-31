// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "WeaponBaseServer.h"
#include "FPSBaseCharacter.generated.h"


class AWeaponBaseClient;




UCLASS()
class MULTIFPS_API AFPSBaseCharacter : public ACharacter
{
	GENERATED_BODY()

#pragma region component

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Character, meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* PlayerCamera;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Character, meta = (AllowPrivateAccess = "true"))
	USkeletalMeshComponent* FPArmsMesh;

	//仅自己观看的第一人称手臂动画蓝图
	UPROPERTY(BlueprintReadOnly, Category = Character, meta = (AllowPrivateAccess = "true"))
	UAnimInstance* ClientArmAnimBP;

	//其他客户端观看的第三人称身体动画蓝图
	UPROPERTY(BlueprintReadOnly, Category = Character, meta = (AllowPrivateAccess = "true"))
	UAnimInstance* ServerBodyAnimBP;

	//PlayerController
	UPROPERTY(BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	class AMultiFPSPlayerController* FPSPlayerController;
	

	//玩家最大生命值
	UPROPERTY(EditAnywhere)
	float MaxHealth;

	//玩家生命值
	UPROPERTY(EditAnywhere)
	float CurrentHealth;

	//生命值百分比
	UPROPERTY(VisibleAnywhere, Transient)
	float HealthPercent;


	


#pragma endregion 
	
public:
	// Sets default values for this character's properties
	AFPSBaseCharacter();

	

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;


#pragma region InputEvents
	

	//MoveForward
	void MoveForward(float AxisValue);

	//MoveRight
	void MoveRight(float AxisValue);

	//慢速度行走
	void LowSpeedWalkAction();

	//正常速度行走
	void NormalSpeedWalkAction();

	//按下开火
	void InputFirePressed();

	//松开开火
	void InputFireReleased();

	//按下瞄准
	void InputAimingPressed();

	//松开瞄准
	void InputAimingReleased();

	//按下换弹
	void InputReloadPressed();


#pragma endregion



public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;



	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	

public:

#pragma region NetWorking

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerLowSpeedWalkAction();
	void ServerLowSpeedWalkAction_Implementation();
	bool ServerLowSpeedWalkAction_Validate();

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerNormalSpeedWalkAction();
	void ServerNormalSpeedWalkAction_Implementation();
	bool ServerNormalSpeedWalkAction_Validate();

	//服务端 步枪射击
	UFUNCTION(Server, Reliable, WithValidation)
	void ServerFireRifleWeapon(FVector CameraLocation, FRotator CameraRotation, bool IsMoving);
	void ServerFireRifleWeapon_Implementation(FVector CameraLocation, FRotator CameraRotation, bool IsMoving);
	bool ServerFireRifleWeapon_Validate(FVector CameraLocation, FRotator CameraRotation, bool IsMoving);

	//服务端 手枪射击
	UFUNCTION(Server, Reliable, WithValidation)
	void ServerFirePistolWeapon(FVector CameraLocation, FRotator CameraRotation, bool IsMoving);
	void ServerFirePistolWeapon_Implementation(FVector CameraLocation, FRotator CameraRotation, bool IsMoving);
	bool ServerFirePistolWeapon_Validate(FVector CameraLocation, FRotator CameraRotation, bool IsMoving);

	//服务端 狙击枪射击
	UFUNCTION(Server, Reliable, WithValidation)
	void ServerFireSniperWeapon(FVector CameraLocation, FRotator CameraRotation, bool IsMoving);
	void ServerFireSniperWeapon_Implementation(FVector CameraLocation, FRotator CameraRotation, bool IsMoving);
	bool ServerFireSniperWeapon_Validate(FVector CameraLocation, FRotator CameraRotation, bool IsMoving);

	//服务器停止射击
	UFUNCTION(Server, Reliable, WithValidation)
	void ServerStopFire();
	void ServerStopFire_Implementation();
	bool ServerStopFire_Validate();

	//服务器更改瞄准状态
	UFUNCTION(Server, Reliable, WithValidation)
	void ServerSetAiming(bool NewAimingState);
	void ServerSetAiming_Implementation(bool NewAimingState);
	bool ServerSetAiming_Validate(bool NewAimingState);

	//服务器主武器换弹
	UFUNCTION(Server, Reliable, WithValidation)
	void ServerReloadPrimary();
	void ServerReloadPrimary_Implementation();
	bool ServerReloadPrimary_Validate();

	//服务器副武器换弹
	UFUNCTION(Server, Reliable, WithValidation)
	void ServerReloadSecondary();
	void ServerReloadSecondary_Implementation();
	bool ServerReloadSecondary_Validate();
	

	//客户端装备主武器
	UFUNCTION(Client, Reliable)
	void ClientEquipFPArmsPrimary();

	//客户端装备副武器
	UFUNCTION(Client, Reliable)
	void ClientEquipFPArmsSecondary();

	//客户端射击逻辑（包含动画、声音等众多效果）
	UFUNCTION(Client, Reliable)
	void ClientFire();

	//客户端更新弹药UI
	UFUNCTION(Client, Reliable)
	void ClientUpdateAmmoUI(int32 ClipCurrentAmmo, int32 GunCurrentAmmo);

	//多播(射击效果)
	UFUNCTION(NetMulticast, Reliable, WithValidation)
	void MultiShooting();
	void MultiShooting_Implementation();
	bool MultiShooting_Validate();

	//多播（生成弹孔贴花）
	UFUNCTION(NetMulticast, Reliable, WithValidation)
	void MultiSpawnBulletDecal(FVector Location, FRotator Rotation);
	void MultiSpawnBulletDecal_Implementation(FVector Location, FRotator Rotation);
	bool MultiSpawnBulletDecal_Validate(FVector Location, FRotator Rotation);

	//客户端更新生命值UI
	UFUNCTION(Client, Reliable)
	void ClientUpdateHealthUI(float InHealth, float InHealthPercent, float InDamage);

	//客户端后坐力
	UFUNCTION(Client, Reliable)
	void ClientRecoil();

	//客户端播放换弹动画
	UFUNCTION(Client, Reliable)
	void ClientReload();

	//客户端瞄准
	UFUNCTION(Client, Reliable)
	void ClientStartAiming();

	//客户端停止瞄准
	UFUNCTION(Client, Reliable)
	void ClientEndAiming();

	//多播,换弹
	UFUNCTION(NetMulticast, Reliable, WithValidation)
	void MultiReload();
	void MultiReload_Implementation();
	bool MultiReload_Validate();

	//多播，死亡时删除第一人称武器
	UFUNCTION(NetMulticast, Reliable, WithValidation)
	void MultiDeathMatchDeath();
	void MultiDeathMatchDeath_Implementation();
	bool MultiDeathMatchDeath_Validate();;
	
	//Reload
	//是否在开火中
	UPROPERTY(Replicated)
	bool bIsFiring;
	//是否在换弹中
	UPROPERTY(Replicated)
	bool bIsReloading;
	//是否在瞄准中
	UPROPERTY(Replicated)
	bool bIsAiming;

	UPROPERTY()
	UUserWidget* ScopeWidget;

	UPROPERTY(EditAnywhere, Category = "SniperUI")
	TSubclassOf<UUserWidget> SniperScopeBPClass;
	

	//换弹延迟回调
	UFUNCTION()
	void DelayPlayArmReloadCallBack();



#pragma endregion



#pragma region Weapon

public:

	//装备主武器
	void EquipPrimary(AWeaponBaseServer* WeaponBaseServer);
	//装备副武器
	void EquipSecondary(AWeaponBaseServer* WeaponBaseServer);

	UFUNCTION(BlueprintImplementableEvent)
	void UpdateFPArmsBlendPose(int32 NewIndex);

private:

	//当前武器类型
	UPROPERTY(Replicated);
	EWeaponType ActiveWeapon;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"));
	EWeaponType TestStartWeapon;
	
	UPROPERTY(meta = (AllowPrivateAccess = "true"))
	AWeaponBaseServer* ServerPrimaryWeapon;//用于服务器的第三人称主武器

	UPROPERTY(meta = (AllowPrivateAccess = "true"))
	AWeaponBaseClient* ClientPrimaryWeapon;//用户客户端自己的第一人称主武器

	UPROPERTY(meta = (AllowPrivateAccess = "true"))
	AWeaponBaseServer* ServerSecondaryWeapon;//用于服务器的第三人称副武器

	UPROPERTY(meta = (AllowPrivateAccess = "true"))
	AWeaponBaseClient* ClientSecondaryWeapon;//用户客户端自己的第一人称副武器
	

	//开始时拥有武器
	void StartWithKindOfWeapon();

	//购买武器
	void PurchaseWeapon(EWeaponType WeaponType);

	//得到客户端的第一人称手臂武器
	AWeaponBaseClient* GetCurrentClientFPArmsWeapon();

	//得到服务端的的第三人称武器
	AWeaponBaseServer* GetCurrentServerTPBodyWeapon();
	





#pragma endregion

private:

#pragma region Fire

	#pragma region Rifle
	
	//主武器射击逻辑
	void FireWeaponPrimary();
	//主武器停止射击
	void StopFirePrimary();
	//步枪射线检测
	void RifleLineTrace(FVector CameraLocation, FRotator CameraRotation, bool bIsMoving);

	#pragma endregion

	#pragma region Pistol

	//副武器射击逻辑
	void FireWeaponSecondary();
	//副武器停止射击
	void StopFireSecondary();
	//手强射线检测
	void PistolLineTrace(FVector CameraLocation, FRotator CameraRotation, bool bIsMoving);

	//手枪射击后的回调方法
	UFUNCTION()
	void DelaySpreadWeaponShootCallBack();

	#pragma endregion

	#pragma region Sniper
	
	//副武器射击逻辑
	void FireWeaponSniper();
	//副武器停止射击
	void StopFireSniper();
	//手强射线检测
	void SniperLineTrace(FVector CameraLocation, FRotator CameraRotation, bool bIsMoving);
	
	//狙击枪射击回调
	UFUNCTION()
	void DelaySniperShootCallBack();

	//UPROPERTY(EditAnywhere, Category = "Debug")
	//EDrawDebugTrace DrawDebug;

	#pragma endregion
	
	//自动射击计时器
	FTimerHandle AutomaticFireTimerHandle;
	
	//主射击内部逻辑 TODO 方法名称不合适
	void FirePrimaryInternal();

	//副武器内部逻辑 TODO 方法名称不合适
	void FireSecondaryInternal();

	//狙击枪内部逻辑 TODO 方法名称不合适
	void FireSniperInternal();
	
	
	//造成伤害
	void DamagePlayer(AActor* DamageActor, const FVector& HitFromDirection, const FHitResult& HitResult);

	//步枪垂直后坐力
	float OldVerticalRecoilAmount;
	float DeltaVerticalRecoilAmount;
	//步枪水平后坐力
	float OldHorizontalRecoilAmount;
	float DeltaHorizontalRecoilAmount;
	//步枪后坐力曲线X轴
	float RecoilCurveCoordX;

	//手强后坐力
	float PistolRecoilMin = 0;
	float PistolRecoilMax = 0;
	
	
	//步枪重置后坐力
	void ResetRecoil();


	/** 受到伤害
	 *
	 * @param DamagedActor 受到伤害的物体
	 * @param Damage 伤害值
	 * @param InstigatedBy 此方法的发起者param
	 * @param HitLocation 检测点位置
	 * @param FHitComponent 检测到的组件
	 * @param BoneName 骨骼名称
	 * @param ShotFromDirection 伤害来源方向
	 * @param DamageType 伤害类型
	 * @param DamageCauser 伤害来源
	 *
	 */
	UFUNCTION()
	void OnHit(AActor* DamagedActor, float Damage, class AController* InstigatedBy, FVector HitLocation,class UPrimitiveComponent* FHitComponent,
		FName BoneName, FVector ShotFromDirection, const class UDamageType* DamageType, AActor* DamageCauser);


	//玩家死亡
	void DeathMatchDeath(AController* InstigatedBy);

#pragma endregion


	void InitializeStatesAndValues();

	void CalculateHealthPercent();
	
	virtual void PostInitProperties() override;

#if WITH_EDITOR

	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;

#endif
	
	
};


