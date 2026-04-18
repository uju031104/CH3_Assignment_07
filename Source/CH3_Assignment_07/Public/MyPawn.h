#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "MyPawn.generated.h"

class USpringArmComponent;
class UCameraComponent;
class UCapsuleComponent;
class UInputAction;
class UInputMappingContext;
struct FInputActionValue;

UCLASS()
class CH3_ASSIGNMENT_07_API AMyPawn : public APawn
{
	GENERATED_BODY()

public:
	AMyPawn();

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Components")
	USkeletalMeshComponent* SkeletalMeshComp;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Components")
	USpringArmComponent* SpringArmComp;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Components")
	UCameraComponent* CameraComp;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Components")
	UCapsuleComponent* CapsuleComp;

	// 함수로 Input Value를 받아서 변수에 저장
	FVector MovementInput;
	FVector LookInput;
	void Move(const FInputActionValue& Value);
	void Look(const FInputActionValue& Value);

	// 속도값 2가지
	//UPROPERTY(EditAnywhere, Category = "Movement")
	//float MoveSpeed = 500.f;
	UPROPERTY(EditAnywhere, Category = "Movement")
	float RotationSpeed = 50.f;

	// 중력 설정 변수
	UPROPERTY(EditAnywhere, Category = "Movement")
	float GravityStrength = 980.f;
	float VerticalVelocity = 0.f;

	// 카메라 전환 속도(값이 클수록 빠름)
	float CameraRotationInterpSpeed = 10.0f;
	// 현재 카메라가 유지해야 할 회전값
	FRotator TargetCameraRotation;
	
	// 바닥 체크 함수
	bool IsOnGround();
	// 바닥 상태 저장 변수
	bool bWasOnGround = false;
	// 화면 전환 체크 변수
	bool bIsNotRotate = false;

	// 에디터에서 IA_Move 할당
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* MoveAction;

	// 에디터에서 IA_Look 할당
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* LookAction;

	// 에디터에서 IMC_MyPawn 할당
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputMappingContext* MyPawnMappingContext;
};
