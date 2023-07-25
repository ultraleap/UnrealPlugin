// Fill out your copyright notice in the Description page of Project Settings.


#include "UltraleapInput.h"

#define LOCTEXT_NAMESPACE "UltraleapInput"

const FKey FUltraleapKey::UltraleapHand_Left_Pinch("UltraleapHand_Left_Pinch");
const FKey FUltraleapKey::UltraleapHand_Left_Grab("UltraleapHand_Left_Grab");
const FKey FUltraleapKey::UltraleapHand_Right_Pinch("UltraleapHand_Right_Pinch");
const FKey FUltraleapKey::UltraleapHand_Right_Grab("UltraleapHand_Right_Grab");

const FUltraleapKeyNames::Type FUltraleapKeyNames::UltraleapHand_Left_Pinch("UltraleapHand_Left_Pinch");
const FUltraleapKeyNames::Type FUltraleapKeyNames::UltraleapHand_Left_Grab("UltraleapHand_Left_Grab");
const FUltraleapKeyNames::Type FUltraleapKeyNames::UltraleapHand_Right_Pinch("UltraleapHand_Right_Pinch");
const FUltraleapKeyNames::Type FUltraleapKeyNames::UltraleapHand_Right_Grab("UltraleapHand_Right_Grab");


FUltraleapInput::FUltraleapInput()
{
}

FUltraleapInput::~FUltraleapInput()
{
}

void FUltraleapInput::PreInit()
{
	EKeys::AddMenuCategoryDisplayInfo("UltraleapHand", LOCTEXT("UltraleapHandSubCategory", "Ultraleap Hand"), TEXT("GraphEditor.PadEvent_16x"));

	EKeys::AddKey(FKeyDetails(FUltraleapKey::UltraleapHand_Left_Pinch, LOCTEXT("UltraleapHand_Left_Pinch", "Ultraleap Hand (L) Pinch"), FKeyDetails::GamepadKey | FKeyDetails::NotBlueprintBindableKey, "UltraleapHand"));
	EKeys::AddKey(FKeyDetails(FUltraleapKey::UltraleapHand_Left_Grab, LOCTEXT("UltraleapHand_Left_Grab", "Ultraleap Hand (L) Grab"), FKeyDetails::GamepadKey | FKeyDetails::NotBlueprintBindableKey, "UltraleapHand"));
	EKeys::AddKey(FKeyDetails(FUltraleapKey::UltraleapHand_Right_Pinch, LOCTEXT("UltraleapHand_Right_Pinch", "Ultraleap Hand (R) Pinch"), FKeyDetails::GamepadKey | FKeyDetails::NotBlueprintBindableKey, "UltraleapHand"));
	EKeys::AddKey(FKeyDetails(FUltraleapKey::UltraleapHand_Right_Grab, LOCTEXT("UltraleapHand_Right_Grab", "Ultraleap Hand (R) Grab"), FKeyDetails::GamepadKey | FKeyDetails::NotBlueprintBindableKey, "UltraleapHand"));
	
}
