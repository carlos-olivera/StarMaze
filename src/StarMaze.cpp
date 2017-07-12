/*  StarMaze: Maze 2D game for Symbian C++ 
 Copyright (C) 2008 Carlos Olivera Terrazas (carlos.olivera.t@gmail.com)

 License: GPL or LGPL (at your choice)
 WWW: http://www.series60.com.ar/blog/index.php/starmaze/

 This program is free software; you can redistribute it and/or modify        
 it under the terms of the GNU General Public License as published by        
 the Free Software Foundation; either version 2 of the License, or           
 (at your option) any later version.                                         
 
 This program is distributed in the hope that it will be useful,       
 but WITHOUT ANY WARRANTY; without even the implied warranty of              
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the               
 GNU General Public License for more details.                
 
 You should have received a copy of the GNU General Public License           
 along with this program; if not, write to the Free Software                 
 Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA   
 
 */

#include <coecntrl.h>
#include <aknappui.h>
#include <aknapp.h>
#include <akndoc.h>
#include "gp.h"
#include <sdlepocapi.h>
#include <aknnotewrappers.h>
#include <eikstart.h>
#include <RDAccelerometer.h>

const TUid KUidSdlApp=
	{
	0xE730F801
	};

// CONSTANTS for accelerometer usage
const TInt KThresholdDown = 12;
const TInt KThresholdUp = -12;
const TInt KThresholdRight = 12;
const TInt KThresholdLeft = -15;
const TInt KPadding = 2;

/*
 * 
 */
class MExitWait
	{
public:
	virtual void DoExit(TInt aErr) = 0;
	};

class CExitWait : public CActive
	{
public:
	CExitWait(MExitWait& aWait);
	void Start();
	~CExitWait();
private:
	void RunL();
	void DoCancel();
private:
	MExitWait& iWait;
	TRequestStatus* iStatusPtr;
	};

/*
 * 
 */
class CSDLWin : public CCoeControl, public MRDAccelerometerObserver
	{
public:
	void ConstructL(const TRect& aRect);
	/**
	 * ~CMovingBallAppView
	 * Virtual Destructor.
	 */
	virtual ~CSDLWin();
	RWindow& GetWindow() const;
	void SetNoDraw();
	/**
	 * From MRDAccelerometerObserver.
	 * Takes care of raw accelerometer data.
	 * @param aX Acceleration in x axis.
	 * @param aY Acceleration in y axis.
	 * @param aZ Acceleration in z axis.
	 */
	void HandleAccelerationL(TInt aX, TInt aY, TInt aZ);
private:
	TBool iLeft, iRight, iUp, iDown;
	TInt KeyLeft;
	TInt KeyRight;
	TInt KeyUp;
	TInt KeyDown;
	CPeriodic* iPeriodic; // periodic timer for backlight
	void Draw(const TRect& aRect) const;
	static TInt Tick(TAny* aObject); // directly called (backlight)
	void DoTick(); // indirectly called (backlight)
private:
	CRDAccelerometer* iAccelerometer;
	TReal iX;
	TReal iY;
	TReal iX1;
	TReal iY1;
	TReal iZ0;
	TReal iZ1;
	TReal iZ2;
	TReal iZmax;
	};

/*
 * 
 */
class CSdlApplication : public CAknApplication
	{
private:
	// from CApaApplication
	CApaDocument* CreateDocumentL();
	TUid AppDllUid() const;
	};

class CSdlAppDocument : public CAknDocument
	{
public:
	CSdlAppDocument(CEikApplication& aApp) :
		CAknDocument(aApp)
		{
		}
private:
	CEikAppUi* CreateAppUiL();
	};

/*
 * 
 */
class CSdlAppUi : public CAknAppUi, public MExitWait
	{
public:
	void ConstructL();
	~CSdlAppUi();
private:
	void HandleCommandL(TInt aCommand);
	void StartTestL(TInt aCmd);
	void DoExit(TInt aErr);
	void HandleWsEventL(const TWsEvent& aEvent, CCoeControl* aDestination);
private:
	CExitWait* iWait;
	CSDLWin* iSDLWin;
	CSDL* iSdl;
	TBool iExit;
	};

CExitWait::CExitWait(MExitWait& aWait) :
	CActive(CActive::EPriorityStandard), iWait(aWait)
	{
	CActiveScheduler::Add(this);
	}

CExitWait::~CExitWait()
	{
	Cancel();
	}

void CExitWait::RunL()
	{
	if (iStatusPtr != NULL)
		iWait.DoExit(iStatus.Int());
	}

void CExitWait::DoCancel()
	{
	if (iStatusPtr != NULL)
		User::RequestComplete(iStatusPtr, KErrCancel);
	}

void CExitWait::Start()
	{
	SetActive();
	iStatusPtr = &iStatus;
	}

void CSDLWin::ConstructL(const TRect& aRect)
	{
	iLeft = iRight = iUp = iDown = false;
	KeyLeft = 0;
	KeyRight= 0;
	KeyUp= 0;
	KeyDown= 0;
	CreateWindowL();
	SetRect(aRect);
	ActivateL();
	iAccelerometer = CRDAccelerometer::NewL(*this);

	const TInt tickInterval=3000000;
	iPeriodic=CPeriodic::NewL(0); // neutral priority
	iPeriodic->Start(tickInterval, tickInterval, TCallBack(Tick, this));
	}

RWindow& CSDLWin::GetWindow() const
	{
	return Window();
	}

void CSDLWin::Draw(const TRect& /*aRect*/) const
	{
	CWindowGc& gc = SystemGc();
	gc.SetPenStyle(CGraphicsContext::ESolidPen);
	gc.SetPenColor(KRgbGray);
	gc.SetBrushStyle(CGraphicsContext::ESolidBrush);
	gc.SetBrushColor(0xaaaaaa);
	gc.DrawRect(Rect());
	}

// -----------------------------------------------------------------------------
// CMovingBallAppView::HandleAccelerationL()
// Takes care of acceleration data.
// -----------------------------------------------------------------------------
//
void CSDLWin::HandleAccelerationL(TInt aX, TInt aY, TInt aZ)
	{
	if(aX < KThresholdRight - KPadding && aX> KThresholdLeft + KPadding)
		{
		if(iRight)
		//keyX.Set(TRawEvent::EKeyUp, EStdKeyRightArrow);
		KeyRight = 0;
		else if(iLeft)
		//keyX.Set(TRawEvent::EKeyUp, EStdKeyLeftArrow);
		KeyLeft = 0;
		iRight = iLeft = false;
		}
	else
		{
		if(!iRight && aX> KThresholdRight + KPadding)
			{
			//keyX.Set(TRawEvent::EKeyDown, EStdKeyRightArrow);
			KeyRight = 1;
			iRight = true;
			}
		else if(!iLeft && aX < KThresholdLeft - KPadding)
			{
			//keyX.Set(TRawEvent::EKeyDown, EStdKeyLeftArrow);
			KeyLeft = 1;
			iLeft = true;
			}
		}

	if(aY < KThresholdDown - KPadding && aY> KThresholdUp + KPadding)
		{
		if(iDown)
		//keyY.Set(TRawEvent::EKeyUp, EStdKeyDownArrow);
		KeyDown = 0;

		else if(iUp)
		//keyY.Set(TRawEvent::EKeyUp, EStdKeyUpArrow);
		KeyUp = 0;
		iDown = iUp = false;
		}
	else
		{
		if(!iDown && aY> KThresholdDown + KPadding)
			{
			//keyY.Set(TRawEvent::EKeyDown, EStdKeyDownArrow);
			KeyDown = 1;
			iDown = true;
			}
		else if(!iUp && aY < KThresholdUp - KPadding)
			{
			//keyY.Set(TRawEvent::EKeyDown, EStdKeyUpArrow);
			KeyUp = 1;
			iUp = true;
			}
		}

	gKeyUp = KeyRight;
	gKeyDown = KeyLeft;
	gKeyLeft = KeyUp;
	gKeyRight = KeyDown;
	}

// --------------------------------
// Preserve backlight
//---------------------------------
TInt CSDLWin::Tick(TAny* aObject)
	{
	((CSDLWin*)aObject)->DoTick(); // cast, and call non-static function
	return 1;
	}

void CSDLWin::DoTick()
	{
	User::ResetInactivityTime();
	}

CSDLWin::~CSDLWin()
	{
	delete iAccelerometer;
	iPeriodic->Cancel();
	delete iPeriodic;
	}

void CSdlAppUi::ConstructL()
	{
	BaseConstructL(ENoScreenFurniture);

	iSDLWin = new (ELeave) CSDLWin;
	iSDLWin->ConstructL(ApplicationRect());

	iWait = new (ELeave) CExitWait(*this);
	}

void CSdlAppUi::HandleCommandL(TInt aCommand)
	{
	switch (aCommand)
		{
		case EAknCmdExit:
		case EAknSoftkeyExit:
		case EEikCmdExit:
			iExit = ETrue;
			if (iWait == NULL || !iWait->IsActive())
				Exit();
			break;
		default:
			if (iSdl == NULL)
				StartTestL(aCommand);
		}
	}

void CSdlAppUi::StartTestL(TInt aCmd)
	{
	TInt flags = CSDL::ENoFlags;
	switch (aCmd)
		{
		case ETestGdi:
			flags |= CSDL::EDrawModeGdi;
			break;
		case ETestDsa:
			flags |= CSDL::ENoFlags;
			break;
		case ETestDsbDb:
			flags |= CSDL::EDrawModeDSBDoubleBuffer;
			break;
		case ETestDsbIu:
			flags |= CSDL::EDrawModeDSBIncrementalUpdate;
			break;
		case ETestDsbDbA:
			flags |= (CSDL::EDrawModeDSBDoubleBuffer | CSDL::EDrawModeDSBAsync);
			break;
		case ETestDsbIuA:
			flags |= (CSDL::EDrawModeDSBIncrementalUpdate
					| CSDL::EDrawModeDSBAsync);
			break;
		}

	iSdl = CSDL::NewL(flags);

	iSdl->SetContainerWindowL(iSDLWin->GetWindow(), 
	iEikonEnv->WsSession(), *iEikonEnv->ScreenDevice());
	iSdl->CallMainL(iWait->iStatus);
	iWait->Start();
	}

void CSdlAppUi::DoExit(TInt aErr)
	{
	if (aErr != KErrNone)
		{
		CAknErrorNote* err = new (ELeave) CAknErrorNote(ETrue);
		TBuf<64> buf;
		buf.Format(_L("SDL Error %d"), aErr);
		err->ExecuteLD(buf);
		}
	else
		{
		iExit = true;
		}
	delete iSdl;
	iSdl = NULL;

	if (iExit)
		Exit();
	}

void CSdlAppUi::HandleWsEventL(const TWsEvent& aEvent, CCoeControl* aDestination)
	{
	if (iSdl != NULL)
		iSdl->AppendWsEvent(aEvent);
	CAknAppUi::HandleWsEventL(aEvent, aDestination);
	}

CSdlAppUi::~CSdlAppUi()
	{
	if (iWait != NULL)
		iWait->Cancel();
	delete iSdl;
	delete iWait;
	delete iSDLWin;
	}

CEikAppUi* CSdlAppDocument::CreateAppUiL()
	{
	return new(ELeave) CSdlAppUi();
	}

TUid CSdlApplication::AppDllUid() const
	{
	return KUidSdlApp;
	}

CApaDocument* CSdlApplication::CreateDocumentL()
	{
	CSdlAppDocument* document = new (ELeave) CSdlAppDocument(*this);
	return document;
	}

LOCAL_C CApaApplication* NewApplication()
	{
	return new CSdlApplication;
	}

GLDEF_C TInt E32Main()
	{
	return EikStart::RunApplication(NewApplication);
	}
