#include "MyPawn.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "InputActionValue.h"
#include "EnhancedInputComponent.h"  // 바인딩을 위해 필요
#include "EnhancedInputSubsystems.h" // IMC 등록을 위해 필요

AMyPawn::AMyPawn()
{
	PrimaryActorTick.bCanEverTick = true;

	CapsuleComp = CreateDefaultSubobject<UCapsuleComponent>(TEXT("CapsuleComp"));
	SetRootComponent(CapsuleComp);

	SkeletalMeshComp = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("SkeletalMeshComp"));
	SkeletalMeshComp->SetupAttachment(CapsuleComp);

	SpringArmComp = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArmComp"));
	SpringArmComp->SetupAttachment(CapsuleComp);
	SpringArmComp->TargetArmLength = 300.f;
	// 컨트롤러 회전 대신 폰의 회전을 그대로 따르도록 설정
	SpringArmComp->bUsePawnControlRotation = false;
	SpringArmComp->bInheritPitch = true;
	SpringArmComp->bInheritYaw = true;
	SpringArmComp->bInheritRoll = true;
	// 카메라가 너무 흔들리는 게 싫다면 '지연(Lag)' 기능을 켠다
	SpringArmComp->bEnableCameraLag = true;
	SpringArmComp->CameraLagSpeed = 5.0f; // 숫자가 낮을수록 부드럽게 따라옴

	CameraComp = CreateDefaultSubobject<UCameraComponent>(TEXT("CameraComp"));
	CameraComp->SetupAttachment(SpringArmComp);
	CameraComp->bUsePawnControlRotation = false;
	
}

void AMyPawn::BeginPlay()
{
	Super::BeginPlay();
	
	// PlayerController를 통해 Enhanced Input Local Player Subsystem을 가져옴
	if (APlayerController* PlayerController = Cast<APlayerController>(GetController()))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(MyPawnMappingContext, 0);
		}
	}
}

void AMyPawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// 중력 적용
	FHitResult GravityHit;
	// 중력은 World 기준으로 아래로 작용하기 때문에 사용한다.
	AddActorWorldOffset(FVector(0.f, 0.f, VerticalVelocity * DeltaTime), true, &GravityHit);

	// 캡슐이 바닥에 닿아있는지 확인
	bool bGrounded = IsOnGround();

	APlayerController* PC = Cast<APlayerController>(GetController());
	
	// 바닥 -> 공중 or 공중 -> 바닥 즉, 상태가 변했을 때만 실행
	if (bGrounded != bWasOnGround)
	{
		if (bGrounded)
		{
			// 카메라를 컨트롤러가 제어
			SpringArmComp->bUsePawnControlRotation = true;
			// 착륙 시 폰의 회전값(공중에선 폰에 회전과 함께 카메라가 움직임)을 카메라 컨트롤러와 일치시킴
			if (PC)
			{
				PC->SetControlRotation(GetActorRotation());
			}				
			// 플레이어가 카메라 제어
			bIsNotRotate = false;

			SpringArmComp->bUsePawnControlRotation = true;
		}
		else
		{
			// 플레이어가 제어하지 못함
			bIsNotRotate = true;
		}
		bWasOnGround = bGrounded;
	}

	// 회전 불가능한 상태일때(이륙 직후) 부드러운 화면 전환
	if (bIsNotRotate && PC)
	{

		FRotator CurrentRotation = PC->GetControlRotation();
		FRotator TargetRotation = GetActorRotation();

		FRotator DelayRotation = FMath::RInterpTo(
			CurrentRotation,
			TargetRotation,
			DeltaTime,
			CameraRotationInterpSpeed
		);

		PC->SetControlRotation(DelayRotation);

		if (CurrentRotation.Equals(TargetRotation, 0.1f))
		{
			bIsNotRotate = false;
			SpringArmComp->bUsePawnControlRotation = false;
		}
	}

	// 바닥에 닿아있고 추락중이면 속도 초기화
	if (bGrounded && VerticalVelocity < 0.f)
	{
		VerticalVelocity = 0.f;
	}
	else
	{
		// 공중에 떠 있으면 중력 적용
		VerticalVelocity -= GravityStrength * DeltaTime;
	}
	
	// 바닥에서 폰이 오뚜기처럼 서있게 + 카메라도 같이 수정
	if (bGrounded && VerticalVelocity == 0.f)
	{
		
		FRotator CurrentRotation = GetActorRotation();
		FRotator DesiredRotation = FRotator(0.f, CurrentRotation.Yaw, 0.f);

		if (FMath::IsNearlyEqual(CurrentRotation.Pitch, 0.f, 0.01f) && FMath::IsNearlyEqual(CurrentRotation.Roll, 0.f, 0.01f))
		{
			SetActorRotation(DesiredRotation);
		}
		else
		{
			FRotator DelayRotation = FMath::RInterpTo(
				CurrentRotation,
				DesiredRotation,
				DeltaTime,
				5.f
			);

			SetActorRotation(DelayRotation);
		}
		
		if (PC)
		{
			FRotator CurrentCameraRotation = PC->GetControlRotation();
			FRotator DesiredCameraRotation = FRotator(CurrentCameraRotation.Pitch, CurrentCameraRotation.Yaw, 0.f);

			if (FMath::IsNearlyEqual(CurrentCameraRotation.Roll, 0.f, 0.01f))
			{
				PC->SetControlRotation(DesiredCameraRotation);
			}
			else
			{
				FRotator DelayCameraRotation = FMath::RInterpTo(
					CurrentCameraRotation,
					DesiredCameraRotation,
					DeltaTime,
					5.f
				);
				PC->SetControlRotation(DelayCameraRotation);
			}
		}
	}


	if (!MovementInput.IsNearlyZero())
	{
		FVector DesiredOffset;
		if (bGrounded)
		{
			DesiredOffset = FVector(MovementInput.X * 500.f, MovementInput.Y * 500.f, MovementInput.Z * 200.f) * DeltaTime;
		}
		else
		{
			DesiredOffset = FVector(MovementInput.X * 250.f, MovementInput.Y * 250.f, MovementInput.Z * 200.f) * DeltaTime;
		}

		FHitResult Hit;
		AddActorLocalOffset(DesiredOffset, true, &Hit);

		// 상승 중 중력 조절
		if (MovementInput.Z > 0.f)
		{
			VerticalVelocity = 0.f; // 상승할땐 중력 무시
		}
	}

	if (!LookInput.IsNearlyZero() && !bGrounded && !bIsNotRotate)
	{
		FRotator NewRotation = FRotator(-LookInput.Y * 100.f * DeltaTime, LookInput.X * 100.f * DeltaTime, LookInput.Z * 100.f * DeltaTime);
		AddActorLocalRotation(NewRotation);
	}

	// 매 프레임 입력 값을 초기화하여 "계속 이동" 방지
	MovementInput = FVector::ZeroVector;
	LookInput = FVector::ZeroVector;
}

void AMyPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		// IA_Move 에셋과 Move 함수 연결
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AMyPawn::Move);

		// IA_Look 에셋과 Look 함수 연결
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &AMyPawn::Look);
	}
}

void AMyPawn::Move(const FInputActionValue& Value)
{
	MovementInput = Value.Get<FVector>();
}

void AMyPawn::Look(const FInputActionValue& Value)
{
	FVector LookValue = Value.Get<FVector>();

	// 바닥에 있을 땐 카메라만 회전, 공중에 뜨면 폰이 회전(공중에서 카메라는 폰 회전에 맞게 움직임)
	if (IsOnGround())
	{
		SpringArmComp->CameraLagSpeed = 10.0f; // 바닥에선 카메라 지연 줄이기
		AddControllerYawInput(LookValue.X * RotationSpeed * GetWorld()->GetDeltaSeconds());
		AddControllerPitchInput(LookValue.Y * RotationSpeed * GetWorld()->GetDeltaSeconds());
	}
	else
	{
		SpringArmComp->CameraLagSpeed = 5.0f; // 공중에서 카메라 지연 원래대로
		LookInput = LookValue;
	}
}

bool AMyPawn::IsOnGround()
{
	FHitResult HitResult;

	// 시작~ 끝점 설정 (현재 위치에서 아주 살짝 아래까지)
	FVector Start = GetActorLocation();
	FVector End = Start + FVector(0.f, 0.f, -2.f);

	// 에디터의 캡슐 정보 가져오기
	FCollisionShape CapsuleShape = FCollisionShape::MakeCapsule(
		CapsuleComp->GetScaledCapsuleRadius(),
		CapsuleComp->GetScaledCapsuleHalfHeight()
	);

	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this); // 나 자신은 제외

	bool bHit = GetWorld()->SweepSingleByChannel(
		HitResult,
		Start,
		End,
		FQuat::Identity,   // 회전 기본값
		ECC_Visibility,    // Visibility 채널 사용 (바닥이 막는)
		CapsuleShape,
		Params
	);

	return bHit;
}