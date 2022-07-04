/******************************************************************************
 * Copyright (C) Ultraleap, Inc. 2011-2021.                                   *
 *                                                                            *
 * Use subject to the terms of the Apache License 2.0 available at            *
 * http://www.apache.org/licenses/LICENSE-2.0, or another agreement           *
 * between Ultraleap and you, your company or other organization.             *
 ******************************************************************************/

#include "FUltraleapLeapCustomDetailsPanel.h"
#include "DetailCategoryBuilder.h"
#include "DetailLayoutBuilder.h"
#include "DetailWidgetRow.h"
#include "IDetailsView.h"
#include "UObject/Class.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/SNullWidget.h"
#include "Widgets/Text/STextBlock.h"
#include "LeapComponent.h"


FUltraleapLeapCustomDetailsPanel::FUltraleapLeapCustomDetailsPanel()
{
}

FUltraleapLeapCustomDetailsPanel::~FUltraleapLeapCustomDetailsPanel()
{

}

TSharedRef<IDetailCustomization> FUltraleapLeapCustomDetailsPanel::MakeInstance()
{
	return MakeShareable(new FUltraleapLeapCustomDetailsPanel);
}

void FUltraleapLeapCustomDetailsPanel::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)
{
	PassDetailsToLeapComponent(DetailBuilder);
}

bool FUltraleapLeapCustomDetailsPanel::PassDetailsToLeapComponent(IDetailLayoutBuilder& DetailBuilder)
{
	bool IsValid = false;
	/* Contains references to all selected objects inside in the viewport */
	TArray<TWeakObjectPtr<UObject>> SelectedObjects;
	DetailBuilder.GetObjectsBeingCustomized(SelectedObjects);
	for (const TWeakObjectPtr<UObject>& Object : SelectedObjects)
	{
		// there's a mirror instance of components in the editor
		// so this instance is not the actual live in editor but a copy
		ULeapComponent* LeapComponentGen = Cast<ULeapComponent>(Object.Get());
		LeapComponentGen->SetCustomDetailsPanel(&DetailBuilder);
		ComponentsReferenced.AddUnique(LeapComponentGen);
		IsValid = true;
	}
	return IsValid;
}
void FUltraleapLeapCustomDetailsPanel::PendingDelete()
{
	for(auto Component : ComponentsReferenced)
	{
		Component->SetCustomDetailsPanel(nullptr);
	}
}