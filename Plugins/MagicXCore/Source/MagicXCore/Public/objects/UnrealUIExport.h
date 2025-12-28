#pragma once

#include "UnrealBaseExport.h"

#include "Components/Widget.h"
#include "Components/PanelSlot.h"
#include "Components/PanelWidget.h"
#include "Components/ContentWidget.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Components/EditableTextBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Components/VerticalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/HorizontalBox.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/CanvasPanel.h"
#include "Components/GridSlot.h"
#include "Components/GridPanel.h"
#include "Components/ScrollBox.h"
#include "Components/Image.h"
#include "Components/ProgressBar.h"
#include "Components/CheckBox.h"
#include "Components/Slider.h"
#include "Components/ComboBoxString.h"
#include "Components/MultiLineEditableTextBox.h"
#include "Components/Border.h"

SCRIPT_DECLARE(, UCE_FString, FString, ucFALSE);
SCRIPT_DECLARE(, UCE_FText, FText, ucFALSE);
SCRIPT_DECLARE(, UCE_FName, FName, ucFALSE);
SCRIPT_DECLARE(, UCE_FColor, FColor, ucFALSE);
SCRIPT_DECLARE(, UCE_FLinearColor, FLinearColor, ucFALSE);
SCRIPT_DECLARE(, UCE_FSlateColor, FSlateColor, ucFALSE);
SCRIPT_DECLARE(, UCE_FVector2D, FVector2D, ucFALSE);
SCRIPT_DECLARE(, UCE_FSlateFontInfo, FSlateFontInfo, ucFALSE);
SCRIPT_DECLARE(, UCE_FAnchors, FAnchors, ucFALSE);
SCRIPT_DECLARE(, UCE_FMargin, FMargin, ucFALSE);


// =================================================================================================

SCRIPT_INHERIT_DECLARE(, UCE_UWidget, UWidget, UCE_UVisual, ucFALSE);
SCRIPT_INHERIT_DECLARE(, UCE_UUserWidget, UUserWidget, UCE_UVisual, ucFALSE);
SCRIPT_INHERIT_DECLARE(, UCE_UPanelSlot, UPanelSlot, UCE_UVisual, ucFALSE);
SCRIPT_INHERIT_DECLARE(, UCE_UPanelWidget, UPanelWidget, UCE_UWidget, ucFALSE);
SCRIPT_INHERIT_DECLARE(, UCE_UContentWidget, UContentWidget, UCE_UPanelWidget, ucFALSE);
SCRIPT_INHERIT_DECLARE(, UCE_UTextLayoutWidget, UTextLayoutWidget, UCE_UWidget, ucFALSE);
SCRIPT_INHERIT_DECLARE(, UCE_UVerticalBoxSlot, UVerticalBoxSlot, UCE_UPanelSlot, ucFALSE);
SCRIPT_INHERIT_DECLARE(, UCE_UVerticalBox, UVerticalBox, UCE_UPanelWidget, ucFALSE);
SCRIPT_INHERIT_DECLARE(, UCE_UHorizontalBoxSlot, UHorizontalBoxSlot, UCE_UPanelSlot, ucFALSE);
SCRIPT_INHERIT_DECLARE(, UCE_UHorizontalBox, UHorizontalBox, UCE_UPanelWidget, ucFALSE);
SCRIPT_INHERIT_DECLARE(, UCE_UCanvasPanelSlot, UCanvasPanelSlot, UCE_UPanelSlot, ucFALSE);
SCRIPT_INHERIT_DECLARE(, UCE_UCanvasPanel, UCanvasPanel, UCE_UPanelWidget, ucFALSE);
SCRIPT_INHERIT_DECLARE(, UCE_UGridSlot, UGridSlot, UCE_UPanelSlot, ucFALSE);
SCRIPT_INHERIT_DECLARE(, UCE_UGridPanel, UGridPanel, UCE_UPanelWidget, ucFALSE);
SCRIPT_INHERIT_DECLARE(, UCE_UScrollBox, UScrollBox, UCE_UPanelWidget, ucFALSE);
SCRIPT_DECLARE(, UCE_FSimpleDelegate, FSimpleDelegate, ucFALSE);

SCRIPT_DECLARE(, UCE_FOnButtonClickedEvent, FOnButtonClickedEvent, ucFALSE);
SCRIPT_INHERIT_DECLARE(, UCE_UButton, UButton, UCE_UContentWidget, ucFALSE);

SCRIPT_INHERIT_DECLARE(, UCE_UBorder, UBorder, UCE_UContentWidget, ucFALSE);
SCRIPT_INHERIT_DECLARE(, UCE_UTextBlock, UTextBlock, UCE_UWidget, ucFALSE);
SCRIPT_INHERIT_DECLARE(, UCE_UEditableTextBox, UEditableTextBox, UCE_UWidget, ucFALSE);
SCRIPT_INHERIT_DECLARE(, UCE_UMultiLineEditableTextBox, UMultiLineEditableTextBox, UCE_UTextLayoutWidget, ucFALSE);
SCRIPT_INHERIT_DECLARE(, UCE_UImage, UImage, UCE_UWidget, ucFALSE);
SCRIPT_INHERIT_DECLARE(, UCE_UProgressBar, UProgressBar, UCE_UWidget, ucFALSE);
SCRIPT_INHERIT_DECLARE(, UCE_UCheckBox, UCheckBox, UCE_UContentWidget, ucFALSE);
SCRIPT_INHERIT_DECLARE(, UCE_USlider, USlider, UCE_UWidget, ucFALSE);
SCRIPT_INHERIT_DECLARE(, UCE_UComboBoxString, UComboBoxString, UCE_UWidget, ucFALSE);
