// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "WeaponBaseClient.generated.h"


class AWeaponBaseServer;
enum class EWeaponType : uint8;

//enum class EWeaponType;

//enum class EWeaponType : uint8;

UCLASS()
class MULTIFPS_API AWeaponBaseClient : public AActor
{
	GENERATED_BODY()

	
	
public:	
	// Sets default values for this actor's properties
	AWeaponBaseClient();

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	USkeletalMeshComponent* WeaponMesh;

	//手臂射击蒙太奇动画
	UPROPERTY(EditAnywhere)
	UAnimMontage* ClientArmFireMontage;

	//手臂换弹蒙太奇动画
	UPROPERTY(EditAnywhere)
	UAnimMontage* ClientArmReloadMontage;

	//枪体射击声音
	UPROPERTY(EditAnywhere)
	USoundBase* FireSound;

	//枪体射击火焰效果
	UPROPERTY(EditAnywhere)
	UParticleSystem* MuzzleFlash;

	//开枪摄像机抖动
	UPROPERTY(EditAnywhere)
	TSubclassOf<UCameraShakeBase> FireCameraShake;

	//默认绑定插槽
	UPROPERTY(EditAnywhere)
	FName AttachArmSocketName;;

	//手臂混合动画序号
	UPROPERTY(EditAnywhere,BlueprintReadOnly)
	int32 FPArmsBlendPose;

	//开镜时FOV
	UPROPERTY(EditAnywhere)
	float FieldOfAimingView;
	

	EWeaponType WeaponType;
	

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;


public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;


	UFUNCTION(BlueprintImplementableEvent, Category = "FPGunAnimation")
	void PlayShootAnimation();

	UFUNCTION(BlueprintImplementableEvent, Category = "FPGunAnimation")
	void PlayReloadAnimation();

	//显示枪体效果
	void DisplayWeaponEffect();
	
};
