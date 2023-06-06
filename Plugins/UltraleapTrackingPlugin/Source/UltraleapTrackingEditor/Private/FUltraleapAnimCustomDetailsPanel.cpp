/******************************************************************************
 * Copyright (C) Ultraleap, Inc. 2011-2021.                                   *
 *                                                                            *
 * Use subject to the terms of the Apache License 2.0 available at            *
 * http://www.apache.org/licenses/LICENSE-2.0, or another agreement           *
 * between Ultraleap and you, your company or other organization.             *
 ******************************************************************************/

#include "FUltraleapAnimCustomDetailsPanel.h"

#include "BodyStateAnimInstance.h"
#include "DetailCategoryBuilder.h"
#include "DetailLayoutBuilder.h"
#include "DetailWidgetRow.h"
#include "IDetailsView.h"
#include "UObject/Class.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/SNullWidget.h"
#include "Widgets/Text/STextBlock.h"

FUltraleapAnimCustomDetailsPanel::FUltraleapAnimCustomDetailsPanel()
{
}

FUltraleapAnimCustomDetailsPanel::~FUltraleapAnimCustomDetailsPanel()
{
}

TSharedRef<IDetailCustomization> FUltraleapAnimCustomDetailsPanel::MakeInstance()
{
	return MakeShareable(new FUltraleapAnimCustomDetailsPanel);
}

void FUltraleapAnimCustomDetailsPanel::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)
{
	// Edits a category. If it doesn't exist it creates a new one
	IDetailCategoryBuilder& CustomCategory = DetailBuilder.EditCategory("Ultraleap auto bone mapping");

	// Store the currently selected objects from the viewport to the SelectedObjects array.
	DetailBuilder.GetObjectsBeingCustomized(SelectedObjects);

	if (!HasValidAnimInstance())
	{
		return;
	}
	// Adding a custom row
	CustomCategory.AddCustomRow(FText::FromString("Auto bone mapping category"))
		.ValueContent()
		.VAlign(VAlign_Center)	  // set vertical alignment to center
		.MaxDesiredWidth(250)[	  // With this operator we declare a new slate object inside our widget row
								  // In this case the slate object is a button
			SNew(SButton)
				.ToolTipText(FText::FromString("Automatically maps tracked bones to the skeleton bones and optionally "
											   "automatically corrects the model orientation"))
				.VAlign(VAlign_Center)
				.OnClicked(this, &FUltraleapAnimCustomDetailsPanel::ClickedOnButton)	// Binding the OnClick function we want to
																						// execute when
																						// this object is clicked
				.Content()[	   // We create a new slate object inside our button.
					SNew(STextBlock).Text(FText::FromString("Auto map!"))]];
}

bool FUltraleapAnimCustomDetailsPanel::HasValidAnimInstance()
{
	bool IsValid = false;
	for (const TWeakObjectPtr<UObject>& Object : SelectedObjects)
	{
		UBodyStateAnimInstance* AnimInstance = Cast<UBodyStateAnimInstance>(Object.Get());

		if (AnimInstance != nullptr)
		{
			USkeletalMeshComponent* OwningComponent = Cast<USkeletalMeshComponent>(AnimInstance->GetOuter());

			IsValid = OwningComponent != nullptr;
		}
	}
	return IsValid;
}
FReply FUltraleapAnimCustomDetailsPanel::ClickedOnButton()
{
	if (GEngine)
	{
		for (const TWeakObjectPtr<UObject>& Object : SelectedObjects)
		{
			UBodyStateAnimInstance* AnimInstance = Cast<UBodyStateAnimInstance>(Object.Get());

			if (AnimInstance != nullptr)
			{
				AnimInstance->ExecuteAutoMapping();
			}
		}
	}
	return FReply::Handled();
}