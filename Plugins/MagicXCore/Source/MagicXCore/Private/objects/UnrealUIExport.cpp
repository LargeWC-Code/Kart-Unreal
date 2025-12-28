#include "objects/UnrealUIExport.h"
#include "objects/ScriptDelegateBridge.h"

SCRIPT_IMPLEMENT_BEGIN(UCE_FString, FString);
	SCRIPT_NCONSTRUCT_1(c1, UCE_TCHAR*);
	SCRIPT_NCONSTRUCT_1(c2, ucCONST UCE_FString&);
	SCRIPT_NCONSTRUCT_1(c3, ucCONST UCE_UCString&);
	SCRIPT_RRMOPERATOR_1(UCE_FString&, dy2, =, ucCONST UCE_TCHAR*);
	SCRIPT_RRMOPERATOR_1(UCE_FString&, dy3, =, ucCONST UCE_FString&);
	SCRIPT_RMOPERATOR_1(UCE_FString, aeq, +=, ucCONST UCE_FString&);
	SCRIPT_RMFUNCTION_1(UCE_FString, Append, ucCONST UCE_FString&);
	SCRIPT_RMOPERATOR_C_0(UCE_TCHAR*, zz, *);
	SCRIPT_RMOPERATOR_C_1(UCE_TCHAR, zkh, [], UCE_ucINT);
	SCRIPT_RMOPERATOR_C_1(UCE_ucINT, eeq, == , ucCONST UCE_FString&);
	SCRIPT_RMOPERATOR_C_1(UCE_ucINT, eeq1, == , ucCONST UCE_TCHAR*);
	SCRIPT_RMOPERATOR_C_1(UCE_ucINT, uneq, != , ucCONST UCE_FString&);
	SCRIPT_RMOPERATOR_C_1(UCE_ucINT, uneq1, != , ucCONST UCE_TCHAR*);
SCRIPT_IMPLEMENT_END(UCE_FString);

SCRIPT_IMPLEMENT_BEGIN(UCE_FName, FName);
	SCRIPT_NCONSTRUCT_1(c1, UCE_TCHAR*);
	SCRIPT_NCONSTRUCT_1(c2, ucCONST UCE_FString&);
	SCRIPT_NCONSTRUCT_1(c3, ucCONST UCE_FName&);
	SCRIPT_RRMOPERATOR_1(UCE_FName&, dy1, =, ucCONST UCE_TCHAR*);
	SCRIPT_RRMOPERATOR_1(UCE_FName&, dy3, =, ucCONST UCE_FName&);
	SCRIPT_RMOPERATOR_C_1(UCE_ucINT, eeq, == , ucCONST UCE_FName&);
	SCRIPT_RMOPERATOR_C_1(UCE_ucINT, uneq, != , ucCONST UCE_FName&);
	SCRIPT_RMFUNCTION_0(UCE_FString, ToString);
	SCRIPT_RMFUNCTION_0(UCE_ucINT, GetNumber);
	SCRIPT_RMFUNCTION_0(UCE_BOOL, IsValid);
	SCRIPT_RMFUNCTION_0(UCE_BOOL, IsNone);
SCRIPT_IMPLEMENT_END(UCE_FName);

SCRIPT_IMPLEMENT_BEGIN(UCE_FText, FText);
	SCRIPT_CONSTRUCT_0();
	// �����ı�
 	SCRIPT_RSMNFUNCTION_1(UCE_FText, _1, FromString, UCE_FString&);
	SCRIPT_RSMNFUNCTION_1(UCE_FText, _2, FromString, ucCONST UCE_TCHAR*);

	SCRIPT_RMFUNCTION_0(UCE_FString, ToString);

	// �Ƚ�
// 	SCRIPT_RMOPERATOR_C_1(UCE_ucINT, eeq, == , ucCONST UCE_FText&);
// 	SCRIPT_RMOPERATOR_C_1(UCE_ucINT, uneq, != , ucCONST UCE_FText&);

	// ���
	SCRIPT_RMFUNCTION_0(UCE_BOOL, IsEmpty);

	// ����ת�ı�
	SCRIPT_RMFUNCTION_1(UCE_FText, AsNumber, UCE_FLOAT);
	SCRIPT_RMFUNCTION_1(UCE_FText, AsPercent, UCE_FLOAT);

	// �ı��Ƚ�
	SCRIPT_RMFUNCTION_1(UCE_BOOL, EqualTo, ucCONST UCE_FText&);
	SCRIPT_RMFUNCTION_1(UCE_BOOL, IdenticalTo, ucCONST UCE_FText&);

	// �ı�����
	SCRIPT_RMFUNCTION_0(UCE_FText, ToUpper);
	SCRIPT_RMFUNCTION_0(UCE_FText, ToLower);
	SCRIPT_RMFUNCTION_0(UCE_FString, BuildSourceString);

SCRIPT_IMPLEMENT_END(UCE_FText);

SCRIPT_IMPLEMENT_BEGIN(UCE_FColor, FColor);
    // ���캯�� (R,G,B,A)
	SCRIPT_CONSTRUCT_4(UCE_BYTE, UCE_BYTE, UCE_BYTE, UCE_BYTE);
    
    // ��̬��ɫ����
    SCRIPT_PROPERTY(UCE_BYTE, R);
    SCRIPT_PROPERTY(UCE_BYTE, G);
    SCRIPT_PROPERTY(UCE_BYTE, B);
    SCRIPT_PROPERTY(UCE_BYTE, A);

	SCRIPT_PROPERTY(UCE_DWORD, Bits);

SCRIPT_IMPLEMENT_END(UCE_FColor);

SCRIPT_IMPLEMENT_BEGIN(UCE_FLinearColor, FLinearColor);
    // ���캯�� (R,G,B,A)
	SCRIPT_CONSTRUCT_4(UCE_FLOAT, UCE_FLOAT, UCE_FLOAT, UCE_FLOAT);
    
    // ��̬��ɫ����
    SCRIPT_PROPERTY(UCE_FLOAT, R);
    SCRIPT_PROPERTY(UCE_FLOAT, G);
    SCRIPT_PROPERTY(UCE_FLOAT, B);
    SCRIPT_PROPERTY(UCE_FLOAT, A);
    
SCRIPT_IMPLEMENT_END(UCE_FLinearColor);

SCRIPT_IMPLEMENT_BEGIN(UCE_FSlateColor, FSlateColor);
    // ���캯�� (R,G,B,A)
	SCRIPT_CONSTRUCT_1(UCE_FLinearColor);
    
SCRIPT_IMPLEMENT_END(UCE_FSlateColor);

SCRIPT_IMPLEMENT_BEGIN(UCE_FVector2D, FVector2D);
	SCRIPT_PROPERTY(UCE_FLOAT, X);
	SCRIPT_PROPERTY(UCE_FLOAT, Y);
SCRIPT_IMPLEMENT_END(UCE_FVector2D);

SCRIPT_IMPLEMENT_BEGIN(UCE_FSlateFontInfo, FSlateFontInfo);
SCRIPT_IMPLEMENT_END(UCE_FSlateFontInfo);

SCRIPT_IMPLEMENT_BEGIN(UCE_FAnchors, FAnchors);
SCRIPT_IMPLEMENT_END(UCE_FAnchors);

SCRIPT_IMPLEMENT_BEGIN(UCE_FMargin, FMargin);
SCRIPT_IMPLEMENT_END(UCE_FMargin);

// =========================================================================================

SCRIPT_IMPLEMENT_BEGIN(UCE_UObject, UObject);
SCRIPT_IMPLEMENT_END(UCE_UObject);

SCRIPT_IMPLEMENT_BEGIN(UCE_UClass, UClass);
SCRIPT_IMPLEMENT_END(UCE_UClass);

SCRIPT_IMPLEMENT_BEGIN(UCE_UVisual, UVisual);
SCRIPT_IMPLEMENT_END(UCE_UVisual);

// =========================================================================================

// === �����ؼ��� ===
SCRIPT_IMPLEMENT_BEGIN(UCE_UWidget, UWidget);
	// ��ǿ��
	SCRIPT_MFUNCTION_BASE_1(UCE_void, 0, SetVisibility, 0, , SetVisibility, ESlateVisibility, UCE_INT);
	SCRIPT_RMFUNCTION_0(UCE_INT, GetVisibility);
	
    SCRIPT_RMFUNCTION_0(UCE_BOOL, IsVisible);

    // ����״̬
	SCRIPT_MFUNCTION_BASE_1(UCE_void, 0, SetIsEnabled, 0, , SetIsEnabled, bool, UCE_INT);
    SCRIPT_RMFUNCTION_0(UCE_BOOL, GetIsEnabled);

    // ��Ⱦ�任
    SCRIPT_MFUNCTION_1(SetRenderTranslation, UCE_FVector2D&);
    SCRIPT_MFUNCTION_1(SetRenderScale, UCE_FVector2D&);
    SCRIPT_MFUNCTION_1(SetRenderOpacity, UCE_FLOAT);
    SCRIPT_RMFUNCTION_0(UCE_FLOAT, GetRenderOpacity);
	
	// ���ӹ�ϵ
    SCRIPT_MFUNCTION_0(RemoveFromParent);
    SCRIPT_RMFUNCTION_0(UCE_UPanelWidget*, GetParent);

    // ����
    SCRIPT_MFUNCTION_0(SetFocus);
    SCRIPT_RMFUNCTION_0(UCE_BOOL, HasKeyboardFocus);

SCRIPT_IMPLEMENT_END(UCE_UWidget);

SCRIPT_IMPLEMENT_BEGIN(UCE_UUserWidget, UUserWidget);
	SCRIPT_MFUNCTION_1(AddToViewport, UCE_INT);

	SCRIPT_RMFUNCTION_1(UCE_UWidget*, GetWidgetFromName, UCE_FName);

	SCRIPT_RMFUNCTION_0(UCE_UWidget*, GetRootWidget);       // ��ȡ���ؼ�
SCRIPT_IMPLEMENT_END(UCE_UUserWidget);

SCRIPT_IMPLEMENT_BEGIN(UCE_UPanelSlot, UPanelSlot);
SCRIPT_IMPLEMENT_END(UCE_UPanelSlot);

// === �������� ===
SCRIPT_IMPLEMENT_BEGIN(UCE_UPanelWidget, UPanelWidget);
	// �ӿؼ����� - ��Щ����ֻ�� PanelWidget ���У�
	SCRIPT_MFUNCTION_1(AddChild, UCE_UWidget*);
	SCRIPT_MFUNCTION_1(RemoveChild, UCE_UWidget*);
	SCRIPT_MFUNCTION_0(ClearChildren);
	SCRIPT_RMFUNCTION_0(UCE_INT, GetChildrenCount);
	SCRIPT_RMFUNCTION_1(UCE_UWidget*, GetChildAt, UCE_INT);
SCRIPT_IMPLEMENT_END(UCE_UPanelWidget);

SCRIPT_IMPLEMENT_BEGIN(UCE_UContentWidget, UContentWidget);
	// ���ݹ��� - ֻ����һ�������ӿؼ�
	SCRIPT_MFUNCTION_1(SetContent, UCE_UWidget*);
	SCRIPT_RMFUNCTION_0(UCE_UWidget*, GetContent);
SCRIPT_IMPLEMENT_END(UCE_UContentWidget);

SCRIPT_IMPLEMENT_BEGIN(UCE_UTextLayoutWidget, UTextLayoutWidget);

SCRIPT_IMPLEMENT_END(UCE_UTextLayoutWidget);

SCRIPT_IMPLEMENT_BEGIN(UCE_UVerticalBoxSlot, UVerticalBoxSlot);
SCRIPT_IMPLEMENT_END(UCE_UVerticalBoxSlot);
// === ���������� ===
SCRIPT_IMPLEMENT_BEGIN(UCE_UVerticalBox, UVerticalBox);
	// �����ӿؼ�����ֱ����
	SCRIPT_RMFUNCTION_1(UCE_UVerticalBoxSlot*, AddChildToVerticalBox, UCE_UWidget*);

	// ��ָ��λ�ò����ӿؼ�
	SCRIPT_RMFUNCTION_2(UCE_UVerticalBoxSlot*, InsertChildAt, UCE_INT, UCE_UWidget*);
SCRIPT_IMPLEMENT_END(UCE_UVerticalBox);

SCRIPT_IMPLEMENT_BEGIN(UCE_UHorizontalBoxSlot, UHorizontalBoxSlot);
SCRIPT_IMPLEMENT_END(UCE_UHorizontalBoxSlot);

SCRIPT_IMPLEMENT_BEGIN(UCE_UHorizontalBox, UHorizontalBox);
	// �����ӿؼ���ˮƽ����
	SCRIPT_RMFUNCTION_1(UCE_UHorizontalBoxSlot*, AddChildToHorizontalBox, UCE_UWidget*);

	// ��ָ��λ�ò����ӿؼ�
	SCRIPT_RMFUNCTION_2(UCE_UHorizontalBoxSlot*, InsertChildAt, UCE_INT, UCE_UWidget*);
SCRIPT_IMPLEMENT_END(UCE_UHorizontalBox);

SCRIPT_IMPLEMENT_BEGIN(UCE_UCanvasPanelSlot, UCanvasPanelSlot);
	// ����λ��
	SCRIPT_MFUNCTION_1(SetPosition, UCE_FVector2D&);
	SCRIPT_RMFUNCTION_0(UCE_FVector2D, GetPosition);

	// ���ô�С
	SCRIPT_MFUNCTION_1(SetSize, UCE_FVector2D&);
	SCRIPT_RMFUNCTION_0(UCE_FVector2D, GetSize);

	// ����ê�㣨���λ�ã�
	SCRIPT_MFUNCTION_1(SetAnchors, UCE_FAnchors&);
	SCRIPT_RMFUNCTION_0(UCE_FAnchors, GetAnchors);

	// ����ƫ�������߾ࣩ
	SCRIPT_MFUNCTION_1(SetOffsets, UCE_FMargin&);
	SCRIPT_RMFUNCTION_0(UCE_FMargin, GetOffsets);

	// ���ö��뷽ʽ
	SCRIPT_MFUNCTION_1(SetAlignment, UCE_FVector2D&);
	SCRIPT_RMFUNCTION_0(UCE_FVector2D, GetAlignment);

	// ���� Z �㼶
	SCRIPT_MFUNCTION_1(SetZOrder, UCE_INT);
	SCRIPT_RMFUNCTION_0(UCE_INT, GetZOrder);
SCRIPT_IMPLEMENT_END(UCE_UCanvasPanelSlot);

SCRIPT_IMPLEMENT_BEGIN(UCE_UCanvasPanel, UCanvasPanel);
	SCRIPT_RMFUNCTION_1(UCE_UCanvasPanelSlot*, AddChildToCanvas, UCE_UWidget*);
SCRIPT_IMPLEMENT_END(UCE_UCanvasPanel);

SCRIPT_IMPLEMENT_BEGIN(UCE_UGridSlot, UGridSlot);
	// ��������λ��
	SCRIPT_MFUNCTION_1(SetRow, UCE_INT);
	SCRIPT_RMFUNCTION_0(UCE_INT, GetRow);
	SCRIPT_MFUNCTION_1(SetColumn, UCE_INT);
	SCRIPT_RMFUNCTION_0(UCE_INT, GetColumn);

	// ���ÿ��п���
	SCRIPT_MFUNCTION_1(SetRowSpan, UCE_INT);
	SCRIPT_RMFUNCTION_0(UCE_INT, GetRowSpan);
	SCRIPT_MFUNCTION_1(SetColumnSpan, UCE_INT);
	SCRIPT_RMFUNCTION_0(UCE_INT, GetColumnSpan);

	// �������
	SCRIPT_MFUNCTION_1(SetPadding, UCE_FMargin&);
	SCRIPT_RMFUNCTION_0(UCE_FMargin, GetPadding);

	// ���ö��뷽ʽ
	SCRIPT_MFUNCTION_BASE_1(UCE_void, 0, SetHorizontalAlignment, 0, , SetHorizontalAlignment, EHorizontalAlignment, UCE_INT);
	SCRIPT_MFUNCTION_BASE_1(UCE_void, 0, SetVerticalAlignment, 0, , SetVerticalAlignment, EVerticalAlignment, UCE_INT);

	// ���ô�С����
	//SCRIPT_MFUNCTION_BASE_1(UCE_void, SetSize, 0, , SetSize, FSlateChildSize, UCE_FSlateChildSize&);
SCRIPT_IMPLEMENT_END(UCE_UGridSlot);

SCRIPT_IMPLEMENT_BEGIN(UCE_UGridPanel, UGridPanel);
	// ֻ�������������ȫɾ��
	SCRIPT_RMFUNCTION_3(UCE_UGridSlot*, AddChildToGrid, UCE_UWidget*, UCE_INT, UCE_INT);
SCRIPT_IMPLEMENT_END(UCE_UGridPanel);

SCRIPT_IMPLEMENT_BEGIN(UCE_UScrollBox, UScrollBox);
	// ��������
	SCRIPT_MFUNCTION_0(ScrollToStart);
	SCRIPT_MFUNCTION_0(ScrollToEnd);
	SCRIPT_MFUNCTION_1(SetScrollOffset, UCE_FLOAT);
	SCRIPT_RMFUNCTION_0(UCE_FLOAT, GetScrollOffset);
	SCRIPT_RMFUNCTION_0(UCE_FLOAT, GetScrollOffsetOfEnd);

	// ������ָ���ؼ�
	SCRIPT_MFUNCTION_1(ScrollWidgetIntoView, UCE_UWidget*);

	// ����������
	SCRIPT_MFUNCTION_BASE_1(UCE_void, 0, SetScrollBarVisibility, 0, , SetScrollBarVisibility, ESlateVisibility, UCE_INT);
	SCRIPT_MFUNCTION_BASE_1(UCE_void, 0, SetOrientation, 0, , SetOrientation, EOrientation, UCE_INT);
SCRIPT_IMPLEMENT_END(UCE_UScrollBox);

// 使用新的委托事件宏简化导出
SCRIPT_UNREAL_FEVENT_0(UCE_FOnButtonClickedEvent, FOnButtonClickedEvent, void);

// 示例：其他常见的委托类型
// SCRIPT_FEVENT_1(UCE_FOnTextChanged, FOnTextChanged, void, FText);  // 文本改变事件
// SCRIPT_FEVENT_1(UCE_FOnValueChanged, FOnValueChanged, void, float);  // 值改变事件

// 示例：多参数委托类型
// SCRIPT_FEVENT_2(UCE_FOnMultiParamEvent, FOnMultiParamEvent, void, int32, FString);  // 双参数事件
// SCRIPT_FEVENT_3(UCE_FOnTripleParamEvent, FOnTripleParamEvent, void, int32, FString, float);  // 三参数事件
// SCRIPT_FEVENT_4(UCE_FOnQuadParamEvent, FOnQuadParamEvent, void, int32, FString, float, bool);  // 四参数事件
// SCRIPT_FEVENT_5(UCE_FOnPentaParamEvent, FOnPentaParamEvent, void, int32, FString, float, bool, FVector);  // 五参数事件

// === ���ݿؼ��� ===
SCRIPT_IMPLEMENT_BEGIN(UCE_UButton, UButton);
	// ����¼���
	SCRIPT_PROPERTY(UCE_FOnButtonClickedEvent, OnClicked);  // ��Ҫί������
	SCRIPT_PROPERTY(UCE_FOnButtonClickedEvent, OnPressed);  // ��Ҫί������

	// ��ť��ʽ
	SCRIPT_MFUNCTION_1(SetBackgroundColor, UCE_FLinearColor&);
SCRIPT_IMPLEMENT_END(UCE_UButton);

SCRIPT_IMPLEMENT_BEGIN(UCE_UBorder, UBorder);
	// ��������
	SCRIPT_MFUNCTION_1(SetBrushColor, UCE_FLinearColor&);
	SCRIPT_RMFUNCTION_0(UCE_FLinearColor, GetBrushColor);

	// ���ݱ߾�
	SCRIPT_MFUNCTION_1(SetPadding, UCE_FMargin&);
	SCRIPT_RMFUNCTION_0(UCE_FMargin, GetPadding);

	// ˮƽ����
	SCRIPT_MFUNCTION_BASE_1(UCE_void, 0, SetHorizontalAlignment, 0, , SetHorizontalAlignment, EHorizontalAlignment, UCE_INT);

	// ��ֱ����
	SCRIPT_MFUNCTION_BASE_1(UCE_void, 0, SetVerticalAlignment, 0, , SetVerticalAlignment, EVerticalAlignment, UCE_INT);
SCRIPT_IMPLEMENT_END(UCE_UBorder);

// === ֱ�Ӽ̳��� Widget �Ŀؼ��� ===
SCRIPT_IMPLEMENT_BEGIN(UCE_UTextBlock, UTextBlock);
	// �ı�����
	SCRIPT_MFUNCTION_1(SetText, UCE_FText&);
	SCRIPT_RMFUNCTION_0(UCE_FText, GetText);

	// ��������
	SCRIPT_MFUNCTION_1(SetFont, UCE_FSlateFontInfo&);
	SCRIPT_RMFUNCTION_0(UCE_FSlateFontInfo, GetFont);

	// ��ɫ����
	SCRIPT_MFUNCTION_1(SetColorAndOpacity, UCE_FSlateColor&);

	// ���뷽ʽ
	SCRIPT_MFUNCTION_BASE_1(UCE_void, 0, SetJustification, 0, , SetJustification, ETextJustify::Type, UCE_INT);
SCRIPT_IMPLEMENT_END(UCE_UTextBlock);

SCRIPT_IMPLEMENT_BEGIN(UCE_UEditableTextBox, UEditableTextBox);
	// �ı�����
	SCRIPT_MFUNCTION_1(SetText, UCE_FText&);
	SCRIPT_RMFUNCTION_0(UCE_FText, GetText);

	// ��ʾ�ı�
	SCRIPT_MFUNCTION_1(SetHintText, UCE_FText&);
	SCRIPT_RMFUNCTION_0(UCE_FText, GetHintText);

	// ֻ��ģʽ
	SCRIPT_MFUNCTION_BASE_1(UCE_void, 0, SetIsReadOnly, 0, , SetIsReadOnly, bool, UCE_INT);
    SCRIPT_RMFUNCTION_0(UCE_BOOL, GetIsReadOnly);

	// ����ģʽ
	SCRIPT_MFUNCTION_BASE_1(UCE_void, 0, SetIsPassword, 0, , SetIsPassword, bool, UCE_INT);
	SCRIPT_RMFUNCTION_0(UCE_BOOL, GetIsPassword);

	// �ı��仯�¼�
// 	SCRIPT_PROPERTY(UCE_FOnEditableTextChangedEvent, OnTextChanged);
// 	SCRIPT_PROPERTY(UCE_FOnEditableTextCommittedEvent, OnTextCommitted);
SCRIPT_IMPLEMENT_END(UCE_UEditableTextBox);

SCRIPT_IMPLEMENT_BEGIN(UCE_UMultiLineEditableTextBox, UMultiLineEditableTextBox);
	// �ı�����
	SCRIPT_MFUNCTION_1(SetText, UCE_FText&);
	SCRIPT_RMFUNCTION_0(UCE_FText, GetText);

	// ��ʾ�ı�
	SCRIPT_MFUNCTION_1(SetHintText, UCE_FText&);
	SCRIPT_RMFUNCTION_0(UCE_FText, GetHintText);

	// ֻ��ģʽ
	SCRIPT_MFUNCTION_BASE_1(UCE_void, 0, SetIsReadOnly, 0, , SetIsReadOnly, bool, UCE_INT);
	SCRIPT_RMFUNCTION_0(UCE_BOOL, GetIsReadOnly);

	// �Զ�����
	SCRIPT_MFUNCTION_BASE_1(UCE_void, 0, SetAutoWrapText, 0, , SetAutoWrapText, bool, UCE_INT);
	SCRIPT_RMFUNCTION_0(UCE_BOOL, GetAutoWrapText);

	// �ı��仯�¼�
	//  SCRIPT_PROPERTY(UCE_FOnMultiLineEditableTextChangedEvent, OnTextChanged);
	//  SCRIPT_PROPERTY(UCE_FOnMultiLineEditableTextCommittedEvent, OnTextCommitted);
SCRIPT_IMPLEMENT_END(UCE_UMultiLineEditableTextBox);

SCRIPT_IMPLEMENT_BEGIN(UCE_UImage, UImage);
	// ͼƬ��Դ
	// 	SCRIPT_MFUNCTION_1(SetBrushFromTexture, UCE_UTexture2D*);
	// 	SCRIPT_MFUNCTION_1(SetBrushFromMaterial, UCE_UMaterialInterface*);

		// ��ɫ��͸����
	SCRIPT_MFUNCTION_1(SetColorAndOpacity, UCE_FLinearColor&);
	SCRIPT_RMFUNCTION_0(UCE_FLinearColor, GetColorAndOpacity);

	// ��͸����
	SCRIPT_MFUNCTION_1(SetOpacity, UCE_FLOAT);

	// ͼƬ��Сģʽ
	SCRIPT_MFUNCTION_BASE_1(UCE_void, 0, SetDesiredSizeOverride, 0, , SetDesiredSizeOverride, FVector2D, UCE_FVector2D&);
SCRIPT_IMPLEMENT_END(UCE_UImage);

SCRIPT_IMPLEMENT_BEGIN(UCE_UProgressBar, UProgressBar);
	// ����ֵ (0.0 - 1.0)
	SCRIPT_MFUNCTION_1(SetPercent, UCE_FLOAT);
	SCRIPT_RMFUNCTION_0(UCE_FLOAT, GetPercent);

	// �����ɫ
	SCRIPT_MFUNCTION_1(SetFillColorAndOpacity, UCE_FLinearColor&);
	SCRIPT_RMFUNCTION_0(UCE_FLinearColor, GetFillColorAndOpacity);

	// ����������
	SCRIPT_MFUNCTION_BASE_1(UCE_void, 0, SetBarFillType, 0, , SetBarFillType, EProgressBarFillType::Type, UCE_INT);

	// �Ƿ���ʾ�ٷֱ��ı�
	SCRIPT_MFUNCTION_BASE_1(UCE_void, 0, SetIsMarquee, 0, , SetIsMarquee, bool, UCE_INT);
SCRIPT_IMPLEMENT_END(UCE_UProgressBar);

SCRIPT_IMPLEMENT_BEGIN(UCE_UCheckBox, UCheckBox);
	// ѡ��״̬
	SCRIPT_MFUNCTION_BASE_1(UCE_void, 0, SetIsChecked, 0, , SetIsChecked, bool, UCE_INT);
	SCRIPT_RMFUNCTION_0(UCE_BOOL, IsChecked);

	// ѡ��״̬ (֧����̬��ѡ��/δѡ��/��ȷ��)
	SCRIPT_MFUNCTION_BASE_1(UCE_void, 0, SetCheckedState, 0, , SetCheckedState, ECheckBoxState, UCE_INT);
	SCRIPT_RMFUNCTION_0(UCE_INT, GetCheckedState);

	// ״̬�ı��¼�
	//  SCRIPT_PROPERTY(UCE_FOnCheckBoxComponentStateChanged, OnCheckStateChanged);
SCRIPT_IMPLEMENT_END(UCE_UCheckBox);

SCRIPT_IMPLEMENT_BEGIN(UCE_USlider, USlider);
	// ����ֵ
	SCRIPT_MFUNCTION_1(SetValue, UCE_FLOAT);
	SCRIPT_RMFUNCTION_0(UCE_FLOAT, GetValue);

	// ֵ��Χ
	SCRIPT_MFUNCTION_1(SetMinValue, UCE_FLOAT);
	SCRIPT_RMFUNCTION_0(UCE_FLOAT, GetMinValue);
	SCRIPT_MFUNCTION_1(SetMaxValue, UCE_FLOAT);
	SCRIPT_RMFUNCTION_0(UCE_FLOAT, GetMaxValue);

	// ����
	SCRIPT_MFUNCTION_1(SetStepSize, UCE_FLOAT);
	SCRIPT_RMFUNCTION_0(UCE_FLOAT, GetStepSize);

	// ������ɫ
	SCRIPT_MFUNCTION_1(SetSliderBarColor, UCE_FLinearColor&);
	SCRIPT_MFUNCTION_1(SetSliderHandleColor, UCE_FLinearColor&);

	// ����
	SCRIPT_MFUNCTION_BASE_1(UCE_void, 0, SetOrientation, 0, , SetOrientation, EOrientation, UCE_INT);
	SCRIPT_RMFUNCTION_0(UCE_INT, GetOrientation);

	// ��������
	SCRIPT_MFUNCTION_BASE_1(UCE_void, 0, SetLocked, 0, , SetLocked, bool, UCE_INT);
	SCRIPT_RMFUNCTION_0(UCE_BOOL, IsLocked);

	// ֵ�ı��¼�
	//  SCRIPT_PROPERTY(UCE_FOnFloatValueChangedEvent, OnValueChanged);
SCRIPT_IMPLEMENT_END(UCE_USlider);

SCRIPT_IMPLEMENT_BEGIN(UCE_UComboBoxString, UComboBoxString);
	// ѡ�����
	SCRIPT_MFUNCTION_1(AddOption, UCE_FString&);
	SCRIPT_MFUNCTION_1(RemoveOption, UCE_FString&);
	SCRIPT_MFUNCTION_0(ClearOptions);
	SCRIPT_RMFUNCTION_0(UCE_INT, GetOptionCount);
	SCRIPT_RMFUNCTION_1(UCE_FString, GetOptionAtIndex, UCE_INT);

	// ��ǰѡ��
	SCRIPT_MFUNCTION_1(SetSelectedOption, UCE_FString&);
	SCRIPT_RMFUNCTION_0(UCE_FString, GetSelectedOption);
	SCRIPT_MFUNCTION_1(SetSelectedIndex, UCE_INT);
	SCRIPT_RMFUNCTION_0(UCE_INT, GetSelectedIndex);

	// ����ѡ��
	SCRIPT_RMFUNCTION_1(UCE_INT, FindOptionIndex, UCE_FString&);
	SCRIPT_RMFUNCTION_0(UCE_BOOL, IsOpen);

	// ���������
	SCRIPT_MFUNCTION_0(ClearSelection);

	// ѡ��ı��¼�
	//  SCRIPT_PROPERTY(UCE_FOnSelectionChangedEvent, OnSelectionChanged);
	//  SCRIPT_PROPERTY(UCE_FOnOpeningEvent, OnOpening);
SCRIPT_IMPLEMENT_END(UCE_UComboBoxString);
