
class UCKartGameInstance
{
public:
	UCKartGameInstance()
	{
		RContainer_NET = new UCRContainer_NET;
	}
	~UCKartGameInstance()
	{
		delete RContainer_NET;
		RContainer_NET = 0;
	}
	ucVOID Run()
	{
		if (UCGetApp()->Name.IsEmpty())
			UCGetApp()->Name = UCString("KartGame");

		ucTrace(UCGetApp()->Name + ITOS(UCGetApp()->Index, UCString(":%d\r\n")));

		UCRunNewFiber(UCEvent(this, UCKartGameInstance::OnFiberRun), 0);

		RContainer_NET->SetAdminKey(0xABCDABCDABCDABCDU);
		RContainer_NET->TCP_SetMaxTime(300);
		RContainer_NET->TCP_Run(6);
		RContainer_NET->TCP_Add(6064);
		//核心服务启动
		RContainer_NET->Run(UCGetApp()->Name, UCGetApp()->Index, ucTRUE, ucFALSE, 10);
	}
	ucVOID OnFiberRun(UCObject* Sender, UCEventArgs* e)
	{
		UCTimeFiberData* FiberData = (UCTimeFiberData*)Sender;
		FiberData->Delay();

// 		RContainer_NET->AppendSource(UCString("local//UCTest/1.obj"),
// 			typeof(UCTest), typeof(UCTestServer), &TestServer, ucTRUE, ucFALSE);
	}

	ucVOID InitDispatcher()
	{
		UCUnrealEventDispatcher& Dispatcher = UCUnrealEventDispatcher::GetStatic();

		Dispatcher.Register(UCString("AKartGamePlayerController::BeginPlay"), UCEvent(this, PlayerController_BeginPlay));
	}
	ucVOID PlayerController_BeginPlay(UCObject*, UCEventArgs*)
	{
		UCTObjectPtr<UClass> WidgetClass = UCLoadClass<UUserWidget>(FString("/Game/VehicleTemplate/Blueprints/UI/BP_Login"));

		APlayerController* PC = UCGetUWorld()->GetFirstPlayerController();

		LoginUI = UCCreateWidget<UUserWidget>(PC, WidgetClass.Object);
		if (LoginUI.Object)
		{
			LoginUI->AddToViewport(0);
			// 通过名称查找BT_Login按钮
			UCTObjectPtr<UButton> LoginButton = (UButton*)(LoginUI.Object->GetWidgetFromName(FName("BT_Login")));
			if (LoginButton.Object)
			{
				LoginButton.Object->OnClicked.AddScript(this, OnButtonClick);
				//LoginButton->OnClicked.AddDynamic(Bridge, &UScriptDelegateBridge::Invoke);
			}
		}
	}
	ucVOID OnButtonClick()
	{
		MBox("OnButtonClick");
	}
private:
	UCRContainer_NET*			RContainer_NET;
	UCTObjectPtr<UUserWidget>	LoginUI;
};

void main()
{
// 	UCKartGameInstance Instance;
// 
// 	Instance.InitDispatcher();
// 	Instance.Run();
}
