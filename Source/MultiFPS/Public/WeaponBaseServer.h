// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "WeaponBaseClient.h"
#include "GameFramework/Actor.h"
#include "WeaponBaseServer.generated.h"




UENUM(BlueprintType)
enum class EWeaponType : uint8
{
	AK47 UMETA(DisplayName = "AK47"),
	M4A1 UMETA(DisplayName = "M4A1"),
	DesertEagle UMETA(DisplayerName = "DesertEagle"),
	Sniper UMETA(DisplayName = "Sniper"),
	End UMETA(Hidden)
};

UCLASS()
class MULTIFPS_API AWeaponBaseServer : public AActor
{
	GENERATED_BODY()

	
public:	
	// Sets default values for this actor's properties
	AWeaponBaseServer();

	UPROPERTY(EditAnywhere)
	EWeaponType KindOfWeapon;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	USkeletalMeshComponent* WeaponMesh;

	UPROPERTY(EditAnywhere)
	class USphereComponent* SphereCollision;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<AWeaponBaseClient> ClientWeaponBaseBPClass;

	UPROPERTY(EditAnywhere)
	FName AttachBodySocketName;

	//枪体射击声音
	UPROPERTY(EditAnywhere)
	USoundBase* FireSound;

	//枪体射击火焰效果
	UPROPERTY(EditAnywhere)
	UParticleSystem* MuzzleFlash;

	//枪体子弹数量
	UPROPERTY(EditAnywhere)
	int32 GunCurrentAmmo;

	//弹夹子弹数量
	UPROPERTY(EditAnywhere, Replicated) //Replicated: 1.如果服务器改了，客户端也会自动改 2.有延迟
	int32 ClipCurrentAmmo;

	//弹夹子弹容量
	UPROPERTY(EditAnywhere)
	int32 MaxClipAmmo;

	//身体射击蒙太奇动画
	UPROPERTY(EditAnywhere)
	UAnimMontage* ServerBodyFireMontage;

	//身体换弹蒙太奇动画
	UPROPERTY(EditAnywhere)
	UAnimMontage* ServerBodyReloadMontage;

	//射线检测距离
	UPROPERTY(EditAnywhere)
	float BulletDistance;

	//弹孔贴花
	UPROPERTY(EditAnywhere)
	UMaterialInterface* BulletDecal;

	//武器基础伤害
	UPROPERTY(EditAnywhere)
	float BaseDamage;

	//武器开火模式
	UPROPERTY(EditAnywhere)
	bool IsAutomatic;

	//武器射速
	UPROPERTY(EditAnywhere, meta = (EditCondition = "IsAutomatic"))
	float AutomaticFireRate;
	
	//垂直后坐力曲线
	UPROPERTY(EditAnywhere)
	UCurveFloat* VerticalRecoilCurve;

	//水平后坐力曲线
	UPROPERTY(EditAnywhere)
	UCurveFloat* HorizontalRecoilCurve;

	//移动射击偏移
	UPROPERTY(EditAnywhere)
	float MovingFireRandomRange;

	//手枪后坐力恢复时间
	UPROPERTY(EditAnywhere, Category = "SpreadWeaponData")
	float PistolSpreadCallBackTime = 0.5;

	//手枪后坐力幅度
	UPROPERTY(EditAnywhere, Category = "SpreadWeaponData")
	float SpreadWeaponMin;
	//手枪后坐力幅度
	UPROPERTY(EditAnywhere, Category = "SpreadWeaponData")
	float SpreadWeaponMax;

	
	

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UFUNCTION()
	virtual void OnBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);


public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

public:

	//被拾取时更改物理相关属性
	UFUNCTION()
	void EquipWeapon();

	UFUNCTION(NetMulticast, Reliable, WithValidation)
	void MultiShootingEffect();
	void MultiShootingEffect_Implementation();
	bool MultiShootingEffect_Validate();

	//TODO 第三人称播放换弹动画，暂无合适动画，未启用
	UFUNCTION(BlueprintImplementableEvent, Category = "TPGunAnimation")
	void PlayReloadAnimation();
	

};
