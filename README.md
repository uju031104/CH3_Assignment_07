# CH3_Assignment_07

프로젝트에 대한 간단한 설명   

<br/>

### 사용한 클래스와 컴포넌트
***
<br/>

**Pawn 클래스**   

`CapsulComponent(RootComponent)`   
`SkeletalMeshcomponent(Attach Root)`   
`SpringArmComponent(Attach Root)`   
`CameraComponent(Attach SpringArm)`  


<br/>


**GameMode 클래스** 

`DefaultPawnClass` 설정

<br/>

### 구현한 기능들
***

<br/>

**기본 이동**   
>WASD를 이용한 2d 이동 (X, Y, 0)  
L-Shift와 SpaceBar를 이용하여 Z축 이동   
이동속도는 공중이 지상의 50% 수준

<br/>

**기본 회전**   
>마우스를 이용한 2d 회전 (Pitch, Yaw, 0)   
Q, E를 이용하여 Roll값 조절      
지상에선 PlayerController로 카메라 컨트롤   
공중에선 폰의 LocalRotation으로 카메라 컨트롤      

<br/>

**기본 물리**   
>직접 중력을 설정(WorldOffset 조절)   
바닥에 닿아있거나 상승중엔 중력 0   
캡슐 범위로 바닥과 충돌 설정      

<br/>

**추가 회전 액션**   
> 바닥에서 기울어진 상태면 천천히 원래 상태로 복구됨(오뚜기 처럼)   
> 상승시 PlayerController의 회전값에서 Pawn의 회전값으로 부드럽게 이동(RInterpTo)   
> 위의 두 회전값이 일치할 때 까지만 마우스로 직접 회전하지 못하게 통제   
> 지상과 공중의 카메라 지연값을 다르게 하여 비행하는 느낌을 살림   