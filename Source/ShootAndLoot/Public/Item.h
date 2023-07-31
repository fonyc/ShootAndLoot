// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Item.generated.h"

UENUM(BlueprintType)
enum class EItemRarity : uint8
{
	E_Common = 0 UMETA(DisplayName="Common"),
	E_Uncommon = 1 UMETA(DisplayName="Uncommon"),
	E_Rare = 2 UMETA(DisplayName="Rare"),
	E_Epic = 3 UMETA(DisplayName="Epic"),
	E_Legendary = 4 UMETA(DisplayName="Legendary"),
	E_Max = 5 UMETA(Hidden)
};
ENUM_RANGE_BY_COUNT(EItemRarity, EItemRarity::E_Max);

UCLASS()
class SHOOTANDLOOT_API AItem : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AItem();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UFUNCTION()
	void OnSphereOverlap(UPrimitiveComponent* OverlappedComponent,
	                     AActor* OtherActor,
	                     UPrimitiveComponent* OtherComp,
	                     int32 OtherBodyIndex,
	                     bool bFromSweep,
	                     const FHitResult& SweepResult);

	UFUNCTION()
	void OnSphereEndOverlap(UPrimitiveComponent* OverlappedComponent,
	                        AActor* OtherActor,
	                        UPrimitiveComponent* OtherComp,
	                        int32 OtherBodyIndex);

	void SetActiveStars(const EItemRarity& Rarity);

public:
	virtual void Tick(float DeltaTime) override;

private:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Item Properties", meta = (AllowPrivateAccess = "true"))
	USkeletalMeshComponent* ItemMesh;

	//Collision box that triggers the HUD with its properties
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Item Properties", meta = (AllowPrivateAccess = "true"))
	class UBoxComponent* CollisionBox;

	//Popup widget for when the player looks at the item
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Item Properties", meta = (AllowPrivateAccess = "true"))
	class UWidgetComponent* PickupWidget;

	//Used to enable item tracing when pointed at
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Item Properties", meta = (AllowPrivateAccess = "true"))
	class USphereComponent* SphereComponent;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Item Properties", meta = (AllowPrivateAccess = "true"))
	bool CanBeTraced;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Item Properties", meta = (AllowPrivateAccess = "true"))
	FString ItemName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Item Properties", meta = (AllowPrivateAccess = "true"))
	int32 ItemCount;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Item Properties", meta = (AllowPrivateAccess = "true"))
	EItemRarity ItemRarity;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Item Properties", meta = (AllowPrivateAccess = "true"))
	TArray<bool> ActiveStars;
	
public:
	FORCEINLINE UWidgetComponent* GetPickupWidget() const { return PickupWidget; }
	FORCEINLINE void SetItemTraceability(const bool Traceability) { CanBeTraced = Traceability; }
	FORCEINLINE bool GetItemTraceability() const { return CanBeTraced; }
	TArray<bool> GetActiveStars() const { return ActiveStars; } 
};
