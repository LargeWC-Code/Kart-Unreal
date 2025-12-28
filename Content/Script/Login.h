#ifndef _LOGIN_
#define _LOGIN_

#define SCREEN_CX	1280
#define SCREEN_CY	720

class UCSimpleTest : public UCControl
{
	UCButton Move;
public:
	UCSimpleTest()
	{
		//Move.Anchor = 2;
		Move.Text = UCString("UCUI - simple test.");
		Move.Size = UCSize(320, 320);
		AddControl(&Move);

		Move.Drag = ucTRUE;

		Move.OnMouseMove += UCEvent(this, This_OnMouseMove);

		Anchor = 15;
		Size = UCSize(SCREEN_CX, SCREEN_CY);
	}
	ucVOID This_OnMouseMove(UCObject*, UCEventArgs*)
	{
	}
};

void main()
{	
	UCDevice3D* Device3D = UIGetDevice3D();

	UCGame Game;
	Game.AutoSize = 1;
	Game.WinSize = UCSize(SCREEN_CX, SCREEN_CY);
	Game.Size = UCSize(SCREEN_CX, SCREEN_CY);
	UCSimpleTest* SimpleTest = new UCSimpleTest;
	SimpleTest->Size = Game.Size;
	Game.Run(SimpleTest);
	delete SimpleTest;
}

#endif