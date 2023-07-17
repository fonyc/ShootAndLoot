// Fill out your copyright notice in the Description page of Project Settings.

#include "Item.h"

#include "Components/BoxComponent.h"
#include "Components/WidgetComponent.h"

AItem::AItem()
{
	PrimaryActorTick.bCanEverTick = true;

	ItemMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("ItemMesh"));
	SetRootComponent(ItemMesh);

	CollisionBox = CreateDefaultSubobject<UBoxComponent>(TEXT("CollisionBox"));
	CollisionBox->SetupAttachment(ItemMesh);
	CollisionBox->SetCollisionResponseToAllChannels(ECR_Ignore);
	CollisionBox->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);

	PickupWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("PickupWidget"));
	PickupWidget->SetupAttachment(ItemMesh);
}

void AItem::BeginPlay()
{
	Super::BeginPlay();

	//Hide item at start
	PickupWidget->SetVisibility(false);
	
}

void AItem::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

