// Fill out your copyright notice in the Description page of Project Settings.

#include "Item.h"

#include "ShooterCharacter.h"
#include "Components/BoxComponent.h"
#include "Components/SphereComponent.h"
#include "Components/WidgetComponent.h"

AItem::AItem():
	ItemName(FString("Default")),
	ItemCount(0),
	ItemRarity(EItemRarity::E_Common)
{
	PrimaryActorTick.bCanEverTick = true;

	ItemMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("ItemMesh"));
	SetRootComponent(ItemMesh);

	CollisionBox = CreateDefaultSubobject<UBoxComponent>(TEXT("CollisionBox"));
	CollisionBox->SetupAttachment(RootComponent);
	CollisionBox->SetCollisionResponseToAllChannels(ECR_Ignore);
	CollisionBox->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);

	PickupWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("PickupWidget"));
	PickupWidget->SetupAttachment(RootComponent);

	SphereComponent = CreateDefaultSubobject<USphereComponent>(TEXT("SphereArea"));
	SphereComponent->SetupAttachment(RootComponent);
}

void AItem::BeginPlay()
{
	Super::BeginPlay();

	//Hide item at start
	if (PickupWidget)
	{
		PickupWidget->SetVisibility(false);
	}

	SetActiveStars(ItemRarity);

	SphereComponent->OnComponentBeginOverlap.AddDynamic(this, &AItem::OnSphereOverlap);
	SphereComponent->OnComponentEndOverlap.AddDynamic(this, &AItem::OnSphereEndOverlap);
}

void AItem::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
                            UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep,
                            const FHitResult& SweepResult)
{
	if (!OtherActor) return;
	if (AShooterCharacter* ShooterCharacter = Cast<AShooterCharacter>(OtherActor))
	{
		ShooterCharacter->IncrementOverlappedItemCount(1);
		SetItemTraceability(true);
	}
}

void AItem::OnSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
                               UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (!OtherActor) return;
	if (AShooterCharacter* ShooterCharacter = Cast<AShooterCharacter>(OtherActor))
	{
		ShooterCharacter->IncrementOverlappedItemCount(-1);
		GetPickupWidget()->SetVisibility(false);
		SetItemTraceability(false);
	}
}

void AItem::SetActiveStars(const EItemRarity& Rarity)
{
	const int32 CastedRarity = static_cast<int32>(Rarity);

	//Calculate max fields inside a Enum
	static int32 EnumMaxFields = 0;
	for (EItemRarity Item : TEnumRange<EItemRarity>())
	{
		++EnumMaxFields;
	}
	
	ActiveStars.SetNum(EnumMaxFields);
	
	for (int32 x = 0; x < EnumMaxFields; ++x)
	{
		ActiveStars[x] = CastedRarity >= x ? true : false; 
	}
}

void AItem::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}
