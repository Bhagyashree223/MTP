//------------------------------------------------------------------------------
// <copyright file="NuiImpl.cpp" company="Microsoft">
//     Copyright (c) Microsoft Corporation.  All rights reserved.
// </copyright>
//------------------------------------------------------------------------------

// Implementation of CSkeletalViewerApp methods dealing with NUI processing

//#define __CAPTURE_PLAYBACK 1

#include "stdafx.h"
#include "SkeletalViewer.h"
#include "resource.h"
#include <mmsystem.h>
#include <assert.h>
#include <strsafe.h>
#include <vector>

//Yetesh starts here
#include <algorithm>
#include <iostream>
#include <fstream>
//#include <opencv2/opencv.hpp>
#include <thread>
#include <chrono>

//using namespace cv;

using namespace std;

typedef struct node
{
	double x,y;
	long frameIdx;
} node;

node points[100000];
int ptscount=0;
bool flag=0,idxFlag=1,ptsFlag=0;
int arrayOfIdx[100000];
int idxcount=0,curr=0;
//Yetesh ends here

//lookups for color tinting based on player index
static const int g_IntensityShiftByPlayerR[] = { 1, 2, 0, 2, 0, 0, 2, 0 };
static const int g_IntensityShiftByPlayerG[] = { 1, 2, 2, 0, 2, 0, 0, 1 };
static const int g_IntensityShiftByPlayerB[] = { 1, 0, 2, 2, 0, 2, 0, 2 };

static const float g_JointThickness = 3.0f;
static const float g_TrackedBoneThickness = 6.0f;
static const float g_InferredBoneThickness = 1.0f;

const int g_BytesPerPixel = 4;

const int g_ScreenWidth = 320;
const int g_ScreenHeight = 240;


enum _SV_TRACKED_SKELETONS
{
    SV_TRACKED_SKELETONS_DEFAULT = 0,
    SV_TRACKED_SKELETONS_NEAREST1,
    SV_TRACKED_SKELETONS_NEAREST2,
    SV_TRACKED_SKELETONS_STICKY1,
    SV_TRACKED_SKELETONS_STICKY2
} SV_TRACKED_SKELETONS;

enum _SV_TRACKING_MODE
{
    SV_TRACKING_MODE_DEFAULT = 0,
    SV_TRACKING_MODE_SEATED
} SV_TRACKING_MODE;

enum _SV_RANGE
{
    SV_RANGE_DEFAULT = 0,
    SV_RANGE_NEAR,
} SV_RANGE;

#ifdef __CAPTURE_PLAYBACK
// Begin: Tanwi 20120809
#define DEPTH_STREAM_ID 1
#define COLOR_STREAM_ID 2
// Begin: Tanwi 20130121
#define SKELETON_STREAM_ID 3
// End: Tanwi 20130121



FILE *configFP = 0;					// Configuration file
									// First line has depth map file map
									// Second line has mode: 0 (playback) or 1 (capture)
//FILE *depthFP = 0;					// depth map capture / playback file
//FILE *colorFP = 0;					// color map capture / playback file
FILE *colorDepthDataFP = 0;
bool bCapture = true;				// Capture or Playback Mode
USHORT * pInputBufferRunDepth = 0;	// Buffer pointer for playback depth
BYTE * pInputBufferRunColor = 0;	// Buffer pointer for playback color
bool bColorHeaderCreated = false;
bool bDepthHeaderCreated = false;
unsigned int nStream = 0;
DWORD frameHeightColor;
DWORD frameWidthColor;
int	bytesPerPixelColor = g_BytesPerPixel;
DWORD frameHeightDepth;
DWORD frameWidthDepth;
int	bytesPerPixelDepth;
fpos_t posDepth;
fpos_t posColor;
fpos_t posBody;

// Begin: Tanwi 20140423
int SkeletonFrameNumber = 0;
extern int runStopFlag;
extern char fileName;
std::vector<jointInfo> jointOri;
// End: Tanwi 20140423

// Begin: Tanwi 20120821
extern int ColorOrIR;
// End: Tanwi 20120821

#if _DEBUG
FILE *debugLogFP = 0;
// Begin: Tanwi 20130123
bool captureSkeleton = 0;
extern unsigned int versionRead; // Version of recorded data file
// End: Tanwi 20130123

//Begin: Tanwi 20130122
FILE *debugSkeletonLogFP0 = 0;
FILE *debugSkeletonLogFP1 = 0;
//Begin: Tanwi 20130122
#endif // _DEBUG
// End: Tanwi 20120809

// Begin: Tanwi 20130909
FILE *SkeletonOrientationRead = 0;
FILE *SkeletonOrientationWrite = 0;
// End: Tanwi 20130909

// Begin: Tanwi 20130929
FILE *skeletontodepth = 0;
// End: Tanwi 20130929
#endif // __CAPTURE_PLAYBACK

/// <summary>
/// Zero out member variables
/// </summary>
void CSkeletalViewerApp::Nui_Zero()
{
    SafeRelease( m_pNuiSensor );

    m_pRenderTarget = NULL;
    m_pBrushJointTracked = NULL;
    m_pBrushJointInferred = NULL;
    m_pBrushBoneTracked = NULL;
    m_pBrushBoneInferred = NULL;
    ZeroMemory(m_Points,sizeof(m_Points));

    m_hNextDepthFrameEvent = NULL;
    m_hNextColorFrameEvent = NULL;
    m_hNextSkeletonEvent = NULL;
	// BEGIN: Tanwi: 20130513
	m_hNextIRFrameEvent = NULL;
	m_pIRStreamHandle = NULL;
	m_pDrawIR = NULL;
	// END: Tanwi: 20130513
    m_pDepthStreamHandle = NULL;
    m_pVideoStreamHandle = NULL;
    m_hThNuiProcess = NULL;
    m_hEvNuiProcessStop = NULL;
    m_LastSkeletonFoundTime = 0;
    m_bScreenBlanked = false;
    m_DepthFramesTotal = 0;
    m_LastDepthFPStime = 0;
    m_LastDepthFramesTotal = 0;
    m_pDrawDepth = NULL;
    m_pDrawColor = NULL;
    m_TrackedSkeletons = 0;
    m_SkeletonTrackingFlags = NUI_SKELETON_TRACKING_FLAG_ENABLE_IN_NEAR_RANGE;
    m_DepthStreamFlags = 0;
    ZeroMemory(m_StickySkeletonIds,sizeof(m_StickySkeletonIds));
}

/// <summary>
/// Callback to handle Kinect status changes, redirects to the class callback handler
/// </summary>
/// <param name="hrStatus">current status</param>
/// <param name="instanceName">instance name of Kinect the status change is for</param>
/// <param name="uniqueDeviceName">unique device name of Kinect the status change is for</param>
/// <param name="pUserData">additional data</param>
void CALLBACK CSkeletalViewerApp::Nui_StatusProcThunk( HRESULT hrStatus, const OLECHAR* instanceName, const OLECHAR* uniqueDeviceName, void * pUserData )
{
    reinterpret_cast<CSkeletalViewerApp *>(pUserData)->Nui_StatusProc( hrStatus, instanceName, uniqueDeviceName );
}

/// <summary>
/// Callback to handle Kinect status changes
/// </summary>
/// <param name="hrStatus">current status</param>
/// <param name="instanceName">instance name of Kinect the status change is for</param>
/// <param name="uniqueDeviceName">unique device name of Kinect the status change is for</param>
void CALLBACK CSkeletalViewerApp::Nui_StatusProc( HRESULT hrStatus, const OLECHAR* instanceName, const OLECHAR* uniqueDeviceName )
{
    // Update UI
    PostMessageW( m_hWnd, WM_USER_UPDATE_COMBO, 0, 0 );

    if( SUCCEEDED(hrStatus) )
    {
        if ( S_OK == hrStatus )
        {
            if ( m_instanceId && 0 == wcscmp(instanceName, m_instanceId) )
            {
                Nui_Init(m_instanceId);
            }
            else if ( !m_pNuiSensor )
            {
                Nui_Init();
            }
        }
    }
    else
    {
        if ( m_instanceId && 0 == wcscmp(instanceName, m_instanceId) )
        {
            Nui_UnInit();
            Nui_Zero();
        }
    }
}

/// <summary>
/// Initialize Kinect by instance name
/// </summary>
/// <param name="instanceName">instance name of Kinect to initialize</param>
/// <returns>S_OK if successful, otherwise an error code</returns>
HRESULT CSkeletalViewerApp::Nui_Init( OLECHAR *instanceName )
{
    // Generic creation failure
    if ( NULL == instanceName )
    {
        MessageBoxResource( IDS_ERROR_NUICREATE, MB_OK | MB_ICONHAND );
        return E_FAIL;
    }

    HRESULT hr = NuiCreateSensorById( instanceName, &m_pNuiSensor );
    
    // Generic creation failure
    if ( FAILED(hr) )
    {
        MessageBoxResource( IDS_ERROR_NUICREATE, MB_OK | MB_ICONHAND );
        return hr;
    }

    SysFreeString(m_instanceId);

    m_instanceId = m_pNuiSensor->NuiDeviceConnectionId();

    return Nui_Init();
}

/// <summary>
/// Initialize Kinect
/// </summary>
/// <returns>S_OK if successful, otherwise an error code</returns>
HRESULT CSkeletalViewerApp::Nui_Init( )
{
	//Yetesh starts here
	if(idxFlag)
	{
		ifstream myfile;
		myfile.open ("bsplineder.txt");
		for(int i=0;i<53;i++)	myfile>>arrayOfIdx[idxcount++];
		sort(arrayOfIdx,arrayOfIdx+idxcount);
		for(int i=0;i<idxcount;i++)	cout<<arrayOfIdx[i]<<endl;
		myfile.close();
	}
	//Yetesh ends here




    HRESULT  hr;
    bool     result;

    if ( !m_pNuiSensor )
    {
        hr = NuiCreateSensorByIndex(0, &m_pNuiSensor);

        if ( FAILED(hr) )
        {
            // Original Code
			//return hr;
			
			// BEGIN: Tanwi: 20120827
			if (bCapture)
				return hr;
			else
				return Nui_Init_Playback();
			// END: Tanwi: 20120827
        }

        SysFreeString(m_instanceId);

        m_instanceId = m_pNuiSensor->NuiDeviceConnectionId();
    }

    m_hNextDepthFrameEvent = CreateEvent( NULL, TRUE, FALSE, NULL );
    m_hNextColorFrameEvent = CreateEvent( NULL, TRUE, FALSE, NULL );
    m_hNextSkeletonEvent = CreateEvent( NULL, TRUE, FALSE, NULL );
	// BEGIN: Tanwi: 20130513
	m_hNextIRFrameEvent = CreateEvent( NULL, TRUE, FALSE, NULL );
	// END: Tanwi: 20130513

    // reset the tracked skeletons, range, and tracking mode
    /*SendDlgItemMessage(m_hWnd, IDC_TRACKEDSKELETONS, CB_SETCURSEL, 0, 0);
    SendDlgItemMessage(m_hWnd, IDC_TRACKINGMODE, CB_SETCURSEL, 0, 0);
    SendDlgItemMessage(m_hWnd, IDC_RANGE, CB_SETCURSEL, 0, 0);*/

    EnsureDirect2DResources();

    m_pDrawDepth = new DrawDevice( );

	// BEGIN: Tanwi: 20130513
    /*result = m_pDrawDepth->Initialize( GetDlgItem( m_hWnd, IDC_DEPTHVIEWER ), m_pD2DFactory, 320, 240, 320 * 4 );
    if ( !result )
    {
        MessageBoxResource( IDS_ERROR_DRAWDEVICE, MB_OK | MB_ICONHAND );
        return E_FAIL;
    }*/
	result = m_pDrawDepth->Initialize( GetDlgItem( m_hWnd, IDC_DEPTHVIEWER ), m_pD2DFactory, 640, 480, 640 * 4 );
    if ( !result )
    {
        MessageBoxResource( IDS_ERROR_DRAWDEVICE, MB_OK | MB_ICONHAND );
        return E_FAIL;
    }
	// END: Tanwi: 20130513

    m_pDrawColor = new DrawDevice( );
    result = m_pDrawColor->Initialize( GetDlgItem( m_hWnd, IDC_VIDEOVIEW ), m_pD2DFactory, 640, 480, 640 * 4 );
    if ( !result )
    {
        MessageBoxResource( IDS_ERROR_DRAWDEVICE, MB_OK | MB_ICONHAND );
        return E_FAIL;
    }


	// BEGIN: Tanwi: 20130513
	m_pDrawIR = new DrawDevice( );

	result = m_pDrawIR->Initialize( GetDlgItem( m_hWnd, IDC_VIDEOVIEW ), m_pD2DFactory, 640, 480, 640 * 4 );
    if ( !result )
    {
        MessageBoxResource( IDS_ERROR_DRAWDEVICE, MB_OK | MB_ICONHAND );
        return E_FAIL;
    }
	// END: Tanwi: 20130513

    
    DWORD nuiFlags = NUI_INITIALIZE_FLAG_USES_DEPTH_AND_PLAYER_INDEX | NUI_INITIALIZE_FLAG_USES_SKELETON |  NUI_INITIALIZE_FLAG_USES_COLOR;

    hr = m_pNuiSensor->NuiInitialize(nuiFlags);
    if ( E_NUI_SKELETAL_ENGINE_BUSY == hr )
    {
        nuiFlags = NUI_INITIALIZE_FLAG_USES_DEPTH |  NUI_INITIALIZE_FLAG_USES_COLOR;
        hr = m_pNuiSensor->NuiInitialize( nuiFlags) ;
    }
  
    if ( FAILED( hr ) )
    {
        if ( E_NUI_DEVICE_IN_USE == hr )
        {
            MessageBoxResource( IDS_ERROR_IN_USE, MB_OK | MB_ICONHAND );
        }
        else
        {
            MessageBoxResource( IDS_ERROR_NUIINIT, MB_OK | MB_ICONHAND );
        }
        return hr;
    }

    if ( HasSkeletalEngine( m_pNuiSensor ) )
    {
        hr = m_pNuiSensor->NuiSkeletonTrackingEnable( m_hNextSkeletonEvent, m_SkeletonTrackingFlags );
        if( FAILED( hr ) )
        {
            MessageBoxResource( IDS_ERROR_SKELETONTRACKING, MB_OK | MB_ICONHAND );
            return hr;
        }
    }
	if (ColorOrIR == 1) {
		hr = m_pNuiSensor->NuiImageStreamOpen(
			NUI_IMAGE_TYPE_COLOR,
			NUI_IMAGE_RESOLUTION_640x480,
			0,
			2,
			m_hNextColorFrameEvent,
			&m_pVideoStreamHandle );

		if ( FAILED( hr ) )
		{
			MessageBoxResource( IDS_ERROR_VIDEOSTREAM, MB_OK | MB_ICONHAND );
			return hr;
		}
	}
	else {
		// BEGIN: Tanwi: 20130513 - add IR stream
		hr = m_pNuiSensor->NuiImageStreamOpen(
					NUI_IMAGE_TYPE_COLOR_INFRARED,
					NUI_IMAGE_RESOLUTION_640x480,
					0,
					2,
					m_hNextIRFrameEvent,
					&m_pIRStreamHandle);

		 if ( FAILED( hr ) )
		{
			MessageBoxResource(IDS_ERROR_IRSTREAM, MB_OK | MB_ICONHAND);
			return hr;
		}
		// END: Tanwi: 20130513
	}
	// BEGIN: Tanwi: 20130513 - increase depth resolution to 640x480
   /* hr = m_pNuiSensor->NuiImageStreamOpen(
        HasSkeletalEngine(m_pNuiSensor) ? NUI_IMAGE_TYPE_DEPTH_AND_PLAYER_INDEX : NUI_IMAGE_TYPE_DEPTH,
        NUI_IMAGE_RESOLUTION_320x240,
        m_DepthStreamFlags,
        2,
        m_hNextDepthFrameEvent,
        &m_pDepthStreamHandle );*/

	hr = m_pNuiSensor->NuiImageStreamOpen(
        HasSkeletalEngine(m_pNuiSensor) ? NUI_IMAGE_TYPE_DEPTH_AND_PLAYER_INDEX : NUI_IMAGE_TYPE_DEPTH,
        NUI_IMAGE_RESOLUTION_640x480,
        m_DepthStreamFlags | NUI_IMAGE_STREAM_FLAG_DISTINCT_OVERFLOW_DEPTH_VALUES,
        2,
        m_hNextDepthFrameEvent,
        &m_pDepthStreamHandle );
	// END: Tanwi: 20130513	

    if ( FAILED( hr ) )
    {
        MessageBoxResource(IDS_ERROR_DEPTHSTREAM, MB_OK | MB_ICONHAND);
        return hr;
    }

    // Start the Nui processing thread
    m_hEvNuiProcessStop = CreateEvent( NULL, FALSE, FALSE, NULL );
    m_hThNuiProcess = CreateThread( NULL, 0, Nui_ProcessThread, this, 0, NULL );
	
    return hr;
}


// BEGIN: Tanwi: 20120827
HRESULT CSkeletalViewerApp::Nui_Init_Playback( )
{
    HRESULT  hr;
    bool     result;

    // reset the tracked skeletons, range, and tracking mode
    /*SendDlgItemMessage(m_hWnd, IDC_TRACKEDSKELETONS, CB_SETCURSEL, 0, 0);
    SendDlgItemMessage(m_hWnd, IDC_TRACKINGMODE, CB_SETCURSEL, 0, 0);
    SendDlgItemMessage(m_hWnd, IDC_RANGE, CB_SETCURSEL, 0, 0);*/

    EnsureDirect2DResources();

    m_pDrawDepth = new DrawDevice( );

	// BEGIN: Tanwi: 20130513
   /* result = m_pDrawDepth->Initialize( GetDlgItem( m_hWnd, IDC_DEPTHVIEWER ), m_pD2DFactory, 320, 240, 320 * 4 );
    if ( !result )
    {
        MessageBoxResource( IDS_ERROR_DRAWDEVICE, MB_OK | MB_ICONHAND );
        return E_FAIL;
    }*/

	result = m_pDrawDepth->Initialize( GetDlgItem( m_hWnd, IDC_DEPTHVIEWER ), m_pD2DFactory, 640, 480, 640 * 4 );
    if ( !result )
    {
        MessageBoxResource( IDS_ERROR_DRAWDEVICE, MB_OK | MB_ICONHAND );
        return E_FAIL;
    }
	// END: Tanwi: 20130513

    m_pDrawColor = new DrawDevice( );
    result = m_pDrawColor->Initialize( GetDlgItem( m_hWnd, IDC_VIDEOVIEW ), m_pD2DFactory, 640, 480, 640 * 4 );
    if ( !result )
    {
        MessageBoxResource( IDS_ERROR_DRAWDEVICE, MB_OK | MB_ICONHAND );
        return E_FAIL;
    }
    
    // Start the Nui processing thread
    m_hEvNuiProcessStop = CreateEvent( NULL, FALSE, FALSE, NULL );
    m_hThNuiProcess = CreateThread( NULL, 0, Nui_ProcessThread, this, 0, NULL );

    return S_OK;
}
// END: Tanwi: 20120827

/// <summary>
/// Uninitialize Kinect
/// </summary>
void CSkeletalViewerApp::Nui_UnInit( )
{
    // Stop the Nui processing thread
    if ( NULL != m_hEvNuiProcessStop )
    {
        // Signal the thread
        SetEvent(m_hEvNuiProcessStop);

        // Wait for thread to stop
        if ( NULL != m_hThNuiProcess )
        {
            WaitForSingleObject( m_hThNuiProcess, INFINITE );
            CloseHandle( m_hThNuiProcess );
        }
        CloseHandle( m_hEvNuiProcessStop );
    }

    if ( m_pNuiSensor )
    {
        m_pNuiSensor->NuiShutdown( );
    }
    if ( m_hNextSkeletonEvent && ( m_hNextSkeletonEvent != INVALID_HANDLE_VALUE ) )
    {
        CloseHandle( m_hNextSkeletonEvent );
        m_hNextSkeletonEvent = NULL;
    }
    if ( m_hNextDepthFrameEvent && ( m_hNextDepthFrameEvent != INVALID_HANDLE_VALUE ) )
    {
        CloseHandle( m_hNextDepthFrameEvent );
        m_hNextDepthFrameEvent = NULL;
    }
    if ( m_hNextColorFrameEvent && ( m_hNextColorFrameEvent != INVALID_HANDLE_VALUE ) )
    {
        CloseHandle( m_hNextColorFrameEvent );
        m_hNextColorFrameEvent = NULL;
    }

	// BEGIN: Tanwi: 20130513
	if ( m_hNextIRFrameEvent && ( m_hNextIRFrameEvent != INVALID_HANDLE_VALUE ) )
    {
        CloseHandle( m_hNextIRFrameEvent );
        m_hNextIRFrameEvent = NULL;
    }
	// END: Tanwi: 20130513

    SafeRelease( m_pNuiSensor );
    
    // clean up Direct2D graphics
    delete m_pDrawDepth;
    m_pDrawDepth = NULL;

    delete m_pDrawColor;
    m_pDrawColor = NULL;

	// BEGIN: Tanwi: 20130513
	delete m_pDrawIR;
    m_pDrawIR = NULL;
	// END: Tanwi: 20130513

    DiscardDirect2DResources();
}

/// <summary>
/// Thread to handle Kinect processing, calls class instance thread processor
/// </summary>
/// <param name="pParam">instance pointer</param>
/// <returns>always 0</returns>
DWORD WINAPI CSkeletalViewerApp::Nui_ProcessThread( LPVOID pParam )
{
    CSkeletalViewerApp *pthis = (CSkeletalViewerApp *)pParam;
    return pthis->Nui_ProcessThread( );
}

/// <summary>
/// Thread to handle Kinect processing
/// </summary>
/// <returns>always 0</returns>
DWORD WINAPI CSkeletalViewerApp::Nui_ProcessThread( )
{

	// BEGIN: Tanwi: 20130513
	//const int numEvents = 4;
	//HANDLE hEvents[numEvents] = { m_hEvNuiProcessStop, m_hNextDepthFrameEvent, m_hNextColorFrameEvent, m_hNextSkeletonEvent };
	const int numEvents = 5;
	HANDLE hEvents[numEvents] = { m_hEvNuiProcessStop, m_hNextDepthFrameEvent, m_hNextColorFrameEvent, m_hNextSkeletonEvent, m_hNextIRFrameEvent };
	// END: Tanwi: 20130513
	
	int    nEventIdx;
	DWORD  t;

	m_LastDepthFPStime = timeGetTime( );

	// Blank the skeleton display on startup
	m_LastSkeletonFoundTime = 0;

	// Main thread loop
	bool continueProcessing = true;
	while ( continueProcessing )
	{
		// Wait for any of the events to be signalled
		nEventIdx = WaitForMultipleObjects( numEvents, hEvents, FALSE, 100 );

		// Timed out, continue
		if ( nEventIdx == WAIT_TIMEOUT )
		{
			continue;
		}

		// stop event was signalled 
		if ( WAIT_OBJECT_0 == nEventIdx )
		{
			continueProcessing = false;
			break;
		}
		//Begin: Tanwi-20120824
		if (bCapture) {
		//End: Tanwi-20120824

			// Wait for each object individually with a 0 timeout to make sure to
			// process all signalled objects if multiple objects were signalled
			// this loop iteration

			// In situations where perfect correspondance between color/depth/skeleton
			// is essential, a priority queue should be used to service the item
			// which has been updated the longest ago

			if ( WAIT_OBJECT_0 == WaitForSingleObject( m_hNextDepthFrameEvent, 0 ) )
			{
				//only increment frame count if a frame was successfully drawn

				if ( Nui_GotDepthAlert() )
				{
					++m_DepthFramesTotal;
				}
			}

			if ( WAIT_OBJECT_0 == WaitForSingleObject( m_hNextColorFrameEvent, 0 ) )
			{
				Nui_GotColorAlert();
			}

			if (  WAIT_OBJECT_0 == WaitForSingleObject( m_hNextSkeletonEvent, 0 ) )
			{
				Nui_GotSkeletonAlert( );
			}

			// BEGIN: Tanwi: 20130513
			if (  WAIT_OBJECT_0 == WaitForSingleObject( m_hNextIRFrameEvent, 0 ) )
			{
				Nui_GotIRAlert( );
			}
			// END: Tanwi: 20130513

			// Once per second, display the depth FPS
			t = timeGetTime( );
			/*if ( (t - m_LastDepthFPStime) > 1000 )
			{
				int fps = ((m_DepthFramesTotal - m_LastDepthFramesTotal) * 1000 + 500) / (t - m_LastDepthFPStime);
				PostMessageW( m_hWnd, WM_USER_UPDATE_FPS, IDC_FPS, fps );
				m_LastDepthFramesTotal = m_DepthFramesTotal;
				m_LastDepthFPStime = t;
			}*/

			// Blank the skeleton panel if we haven't found a skeleton recently
			if ( (t - m_LastSkeletonFoundTime) > 300 )
			{
				if ( !m_bScreenBlanked )
				{
					Nui_BlankSkeletonScreen( );
					m_bScreenBlanked = true;
				}
			}
		}
		//Begin: Tanwi-20120824
		else {
			DisplayDumpData();
		}
		//End: Tanwi-20120824
	}
	return 0;
}

//Begin: Tanwi-20120824
void CSkeletalViewerApp::DisplayDumpData(){
	if (!feof(colorDepthDataFP)) {
		while(!runStopFlag){
			//printf("Freezed");
		}
		unsigned int streamID = COLOR_STREAM_ID;
		fread (&streamID, sizeof(unsigned int), 1, colorDepthDataFP );
		if (streamID == COLOR_STREAM_ID) {
			ColorDumpDataDisplay();
		}
		else if (streamID == DEPTH_STREAM_ID) {
			DepthDumpDataDisplay();
		}
		else {
			SkeletonDumpDataDisplay();
		}
	}

	//Yetesh starts here
	else if(!flag && ptsFlag)
	{
		flag=1;
		cout<<"I m writing"<<endl;
		ofstream f;
		f.open("data.txt");
		for(int i=0;i<ptscount;i++)	f<<points[i].x<<" "<<points[i].y<<" "<<points[i].frameIdx<<endl;
		f.close();
		cout<<"I m done with writing"<<endl;

	}
	//Yetesh ends here

	return;
}

void CSkeletalViewerApp::ColorDumpDataDisplay() {

	LARGE_INTEGER BaseTimeCountColor;
	LARGE_INTEGER EndTimeCountColor;
	LARGE_INTEGER Frequency;
	
	QueryPerformanceFrequency(&Frequency);
	QueryPerformanceCounter(&BaseTimeCountColor);

	LARGE_INTEGER liMyTimeStamp;
	DWORD dwMyFrameNumber;
	DWORD frameWidth, frameHeight;
	frameWidth = frameWidthColor;
	frameHeight = frameHeightColor;
	fread(&liMyTimeStamp, sizeof(LARGE_INTEGER), 1, colorDepthDataFP);
	fread(&dwMyFrameNumber, sizeof(DWORD), 1, colorDepthDataFP);

	if (debugLogFP != 0) {
		fprintf(debugLogFP, "%s, ","Color");
		fprintf(debugLogFP, "%ld, ", dwMyFrameNumber);
		fprintf(debugLogFP, "%ld, %ld\n", liMyTimeStamp.HighPart, liMyTimeStamp.LowPart);
	}
	//fflush(debugLogFP);

	fread (pInputBufferRunColor, sizeof(BYTE), frameHeight * frameWidth * bytesPerPixelColor, colorDepthDataFP );
	int bufSize = frameHeight * frameWidth * bytesPerPixelColor;
	BYTE * pBufferRun = pInputBufferRunColor;
	
	////Yetesh starts here
	//
	//if(dwMyFrameNumber==arrayOfIdx[curr] && idxFlag)
	//{
	//	//Mat newImg = Mat(frameHeight, frameWidth, CV_8UC3, pBufferRun);
	//	//vector<int> compression_params;
	//	//compression_params.push_back(CV_IMWRITE_JPEG_QUALITY);
	//	//imwrite(strcat(strcat("image",itoa(curr,NULL,10)),"jpg"), newImg, compression_params);
	//	this_thread::sleep_for(chrono::seconds(10));
	//	curr++;
	//}
	////Yetesh ends here


    m_pDrawColor->Draw( static_cast<BYTE *>(pBufferRun), bufSize );

	QueryPerformanceCounter(&EndTimeCountColor);
	
	unsigned int colorTime = (unsigned int)((double)(EndTimeCountColor.LowPart - BaseTimeCountColor.LowPart)/(double)Frequency.LowPart*1000);
	DWORD sleepTime = (colorTime > 31)? 0: 31-colorTime;
	Sleep(sleepTime);

	return;
}

void CSkeletalViewerApp::DepthDumpDataDisplay() {
	
	LARGE_INTEGER liMyTimeStamp;
	DWORD dwMyFrameNumber;
	DWORD frameWidth, frameHeight;
	BYTE * rgbrun = m_depthRGBX;

	frameWidth = frameWidthDepth;
	frameHeight = frameHeightDepth;

	fread(&liMyTimeStamp, sizeof(LARGE_INTEGER), 1, colorDepthDataFP);
	fread(&dwMyFrameNumber, sizeof(DWORD), 1, colorDepthDataFP);

	if (debugLogFP != 0) {
		fprintf(debugLogFP, "%s, ","Depth");
		fprintf(debugLogFP, "%ld, ", dwMyFrameNumber);
		fprintf(debugLogFP, "%ld, %ld\n", liMyTimeStamp.HighPart, liMyTimeStamp.LowPart);
		//fflush(debugLogFP);
	}

	fread (pInputBufferRunDepth, sizeof(BYTE), frameHeight * frameWidth * bytesPerPixelDepth, colorDepthDataFP );
				
	const USHORT * pBufferRun = (const USHORT *) (byte *)pInputBufferRunDepth;
	const USHORT * pBufferEnd = pBufferRun + (frameWidth * frameHeight);


        while ( pBufferRun < pBufferEnd )
        {
            USHORT depth     = *pBufferRun;
            USHORT realDepth = NuiDepthPixelToDepth(depth);
            USHORT player    = NuiDepthPixelToPlayerIndex(depth);

            //Begin: Tanwi 20130611
			if(realDepth == 0){
				*(rgbrun++) = 255;
				*(rgbrun++) = 0;
				*(rgbrun++) = 0;
			 }
			 else if(realDepth == 8191){
				*(rgbrun++) = 0;
				*(rgbrun++) = 255;
				*(rgbrun++) = 0;
			 }
			 else if(realDepth == 4095){
				*(rgbrun++) = 0;
				*(rgbrun++) = 0;
				*(rgbrun++) = 255;
			 }
			 else{

				// transform 13-bit depth information into an 8-bit intensity appropriate
                // for display (we disregard information in most significant bit)
				BYTE intensity = static_cast<BYTE>(~(realDepth >> 4));

				// tint the intensity by dividing by per-player values
				*(rgbrun++) = intensity >> g_IntensityShiftByPlayerB[player];
				*(rgbrun++) = intensity >> g_IntensityShiftByPlayerG[player];
				*(rgbrun++) = intensity >> g_IntensityShiftByPlayerR[player];
			 }
			 //End: Tanwi 20130611

            // no alpha information, skip the last byte
            ++rgbrun;

            ++pBufferRun;
        }

        m_pDrawDepth->Draw( m_depthRGBX, frameWidth * frameHeight * g_BytesPerPixel );

		return;
    }
   
//Begin: Tanwi-20130121
void CSkeletalViewerApp::SkeletonDumpDataDisplay() {
	LARGE_INTEGER liTimeStamp;
	DWORD dwFrameNumber;
	Vector4 vFloorClipPlane;
	_NUI_SKELETON_DATA skeletonData;
	jointOri.clear();

	// Begin: Tanwi 20130909
	//NUI_SKELETON_FRAME SkeletonFrame = {0};
	// End: Tanwi 20130909

	SkeletonFrameNumber = SkeletonFrameNumber + 1;
	PostMessageW( m_hWnd, WM_USER_UPDATE_FN, IDC_FN, SkeletonFrameNumber);


	// Begin: Tanwi 20130929
	LONG   x, y;
    USHORT depth;
	// End: Tanwi 20130929

    // we found a skeleton, re-start the skeletal timer
    m_bScreenBlanked = false;

    // Endure Direct2D is ready to draw
    HRESULT hr = EnsureDirect2DResources( );
    if ( FAILED( hr ) )
    {
        return;
    }

	if (versionRead == 1) {

		fread(&liTimeStamp, sizeof( LARGE_INTEGER ), 1, colorDepthDataFP);	
		fread(&dwFrameNumber, sizeof( DWORD ), 1, colorDepthDataFP);	
		fread(&vFloorClipPlane, sizeof( Vector4 ), 1, colorDepthDataFP);	
	}

    m_pRenderTarget->BeginDraw();
    m_pRenderTarget->Clear( );

	RECT rct;
    GetClientRect( GetDlgItem( m_hWnd, IDC_SKELETALVIEW ), &rct);
    int width = rct.right;
    int height = rct.bottom;
	
    for ( int i = 0 ; i < NUI_SKELETON_COUNT; i++ )
    {
		fread(&skeletonData.eTrackingState, sizeof(NUI_SKELETON_TRACKING_STATE), 1, colorDepthDataFP);	
		if (skeletonData.eTrackingState == NUI_SKELETON_TRACKED)
		{

			fread(&skeletonData.dwTrackingID, sizeof( DWORD ), 1, colorDepthDataFP);	
			fread(&skeletonData.dwEnrollmentIndex, sizeof( DWORD ), 1, colorDepthDataFP);	
			fread(&skeletonData.dwUserIndex, sizeof( DWORD ), 1, colorDepthDataFP);	
			fread(&skeletonData.Position, sizeof( Vector4 ), 1, colorDepthDataFP);	
			fread(&skeletonData.SkeletonPositions, sizeof( Vector4  ), 20, colorDepthDataFP);	
			fread(&skeletonData.eSkeletonPositionTrackingState, sizeof( NUI_SKELETON_POSITION_TRACKING_STATE ), 20, colorDepthDataFP);	
			fread(&skeletonData.dwQualityFlags, sizeof( DWORD   ), 1, colorDepthDataFP);	
			
			if (debugSkeletonLogFP1)
			{
				fprintf(debugSkeletonLogFP1, "TRACKED, ");
				fprintf(debugSkeletonLogFP1, "%ld, ", skeletonData.dwTrackingID);
				fprintf(debugSkeletonLogFP1, "%ld, ", skeletonData.dwEnrollmentIndex);
				fprintf(debugSkeletonLogFP1, "%ld, ", skeletonData.dwUserIndex);
				fprintf(debugSkeletonLogFP1, "%f, ", skeletonData.Position.x);
				fprintf(debugSkeletonLogFP1, "%f, ", skeletonData.Position.y);
				fprintf(debugSkeletonLogFP1, "%f\n", skeletonData.Position.z);
			}


			//Yetesh starts here
			//store x,y and frame idx of right hand joint in points array
			//cout<<"****"<<dwFrameNumber<<endl;
				Vector4 v=skeletonData.SkeletonPositions[NUI_SKELETON_POSITION_HAND_RIGHT];
				points[ptscount].x=v.x;
				points[ptscount].y=v.y;
				points[ptscount++].frameIdx=dwFrameNumber;
				//cout<<points[ptscount-1].x<<","<<points[ptscount-1].y<<","<<points[ptscount-1].frameIdx<<endl;
			
			//Yetesh ends here
			
			//Yetesh starts here
	
			if(dwFrameNumber==arrayOfIdx[curr] && idxFlag && curr<idxcount)
			{
				//Mat newImg = Mat(frameHeight, frameWidth, CV_8UC3, pBufferRun);
				//vector<int> compression_params;
				//compression_params.push_back(CV_IMWRITE_JPEG_QUALITY);
				//imwrite(strcat(strcat("image",itoa(curr,NULL,10)),"jpg"), newImg, compression_params);
				this_thread::sleep_for(chrono::seconds(10));
				cout<<arrayOfIdx[curr]<<" "<<dwFrameNumber<<endl;
				curr++;
			}
			//Yetesh ends here


			Nui_DrawSkeleton( skeletonData, width, height );
			
			
			
		}
		else if (skeletonData.eTrackingState == NUI_SKELETON_POSITION_ONLY) {

			fread(&skeletonData.Position, sizeof( Vector4  ), 1, colorDepthDataFP);	
			
			if (debugSkeletonLogFP1) {
				fprintf(debugSkeletonLogFP1, "POSITION_ONLY, ");
				fprintf(debugSkeletonLogFP1, "%f, ", skeletonData.Position.x);
				fprintf(debugSkeletonLogFP1, "%f, ", skeletonData.Position.y);
				fprintf(debugSkeletonLogFP1, "%f\n", skeletonData.Position.z);
			}

			D2D1_ELLIPSE ellipse = D2D1::Ellipse(
					SkeletonToScreen( skeletonData.Position, width, height ),
					g_JointThickness,
					g_JointThickness
					);

			m_pRenderTarget->DrawEllipse(ellipse, m_pBrushJointTracked);
		}
		

		
		NUI_SKELETON_BONE_ORIENTATION boneOrientations[NUI_SKELETON_POSITION_COUNT];
		NuiSkeletonCalculateBoneOrientations(&skeletonData, boneOrientations);

		if (skeletonData.eTrackingState == NUI_SKELETON_TRACKED)
		{
			
			for (int j = 0; j < NUI_SKELETON_POSITION_COUNT; j++)
			{
				// Begin: Tanwi 20130929
				// Transform skeleton coordinates to depth image
				NuiTransformSkeletonToDepthImage(skeletonData.SkeletonPositions[j], &x, &y, &depth);
				if (0 != skeletontodepth) {
					fprintf(skeletontodepth, " %d, ", x);
					fprintf(skeletontodepth, " %d, ", y);
					fprintf(skeletontodepth, " %hd, ", depth);
				}
				// End: Tanwi 20130929
				
				NUI_SKELETON_BONE_ORIENTATION & orientation = boneOrientations[j];
				if (0 != SkeletonOrientationRead) {

					fprintf(SkeletonOrientationRead, "%d ", x);
					fprintf(SkeletonOrientationRead, "%d ", y);
					fprintf(SkeletonOrientationRead, "%hd\n", depth);
					fprintf(SkeletonOrientationRead, "%d", orientation.startJoint);
					fprintf(SkeletonOrientationRead, " %d", orientation.endJoint);
					fprintf(SkeletonOrientationRead, " %f", orientation.absoluteRotation.rotationMatrix.M11);
					fprintf(SkeletonOrientationRead, " %f", orientation.absoluteRotation.rotationMatrix.M12);
					fprintf(SkeletonOrientationRead, " %f", orientation.absoluteRotation.rotationMatrix.M13);
					fprintf(SkeletonOrientationRead, " %f", orientation.absoluteRotation.rotationMatrix.M21);
					fprintf(SkeletonOrientationRead, " %f", orientation.absoluteRotation.rotationMatrix.M22);
					fprintf(SkeletonOrientationRead, " %f", orientation.absoluteRotation.rotationMatrix.M23);
					fprintf(SkeletonOrientationRead, " %f", orientation.absoluteRotation.rotationMatrix.M31);
					fprintf(SkeletonOrientationRead, " %f", orientation.absoluteRotation.rotationMatrix.M32);
					fprintf(SkeletonOrientationRead, " %f\n", orientation.absoluteRotation.rotationMatrix.M33);


					fprintf(SkeletonOrientationRead, " %f", orientation.hierarchicalRotation.rotationMatrix.M11);
					fprintf(SkeletonOrientationRead, " %f", orientation.hierarchicalRotation.rotationMatrix.M12);
					fprintf(SkeletonOrientationRead, " %f", orientation.hierarchicalRotation.rotationMatrix.M13);
					fprintf(SkeletonOrientationRead, " %f", orientation.hierarchicalRotation.rotationMatrix.M21);
					fprintf(SkeletonOrientationRead, " %f", orientation.hierarchicalRotation.rotationMatrix.M22);
					fprintf(SkeletonOrientationRead, " %f", orientation.hierarchicalRotation.rotationMatrix.M23);
					fprintf(SkeletonOrientationRead, " %f", orientation.hierarchicalRotation.rotationMatrix.M31);
					fprintf(SkeletonOrientationRead, " %f", orientation.hierarchicalRotation.rotationMatrix.M32);
					fprintf(SkeletonOrientationRead, " %f\n", orientation.hierarchicalRotation.rotationMatrix.M33);

					jointInfo tempJointInfo;
					tempJointInfo.x = x;
					tempJointInfo.y = y;
					tempJointInfo.z = depth;
					tempJointInfo.startJoint = orientation.startJoint;
					tempJointInfo.endJoint = orientation.endJoint;

					tempJointInfo.AbsoluteMatrix[0][0] = orientation.absoluteRotation.rotationMatrix.M11;
					tempJointInfo.AbsoluteMatrix[0][1] = orientation.absoluteRotation.rotationMatrix.M12;
					tempJointInfo.AbsoluteMatrix[0][2] = orientation.absoluteRotation.rotationMatrix.M13;
					tempJointInfo.AbsoluteMatrix[1][0] = orientation.absoluteRotation.rotationMatrix.M21;
					tempJointInfo.AbsoluteMatrix[1][1] = orientation.absoluteRotation.rotationMatrix.M22;
					tempJointInfo.AbsoluteMatrix[1][2] = orientation.absoluteRotation.rotationMatrix.M23;
					tempJointInfo.AbsoluteMatrix[2][0] = orientation.absoluteRotation.rotationMatrix.M31;
					tempJointInfo.AbsoluteMatrix[2][1] = orientation.absoluteRotation.rotationMatrix.M32;
					tempJointInfo.AbsoluteMatrix[2][2] = orientation.absoluteRotation.rotationMatrix.M33;


					tempJointInfo.HierarichalMatrix[0][0] = orientation.hierarchicalRotation.rotationMatrix.M11;
					tempJointInfo.HierarichalMatrix[0][1] = orientation.hierarchicalRotation.rotationMatrix.M12;
					tempJointInfo.HierarichalMatrix[0][2] = orientation.hierarchicalRotation.rotationMatrix.M13;
					tempJointInfo.HierarichalMatrix[1][0] = orientation.hierarchicalRotation.rotationMatrix.M21;
					tempJointInfo.HierarichalMatrix[1][1] = orientation.hierarchicalRotation.rotationMatrix.M22;
					tempJointInfo.HierarichalMatrix[1][2] = orientation.hierarchicalRotation.rotationMatrix.M23;
					tempJointInfo.HierarichalMatrix[2][0] = orientation.hierarchicalRotation.rotationMatrix.M31;
					tempJointInfo.HierarichalMatrix[2][1] = orientation.hierarchicalRotation.rotationMatrix.M32;
					tempJointInfo.HierarichalMatrix[2][2] = orientation.hierarchicalRotation.rotationMatrix.M33;
					jointOri.push_back(tempJointInfo);

				}
			}
			fprintf(SkeletonOrientationRead, "\n\n");
			fprintf(skeletontodepth, "\n");
		}
		
	}

	// Begin: Tanwi 20130909
	/*SkeletonFrame.liTimeStamp = liTimeStamp;
	SkeletonFrame.dwFrameNumber = dwFrameNumber;
	SkeletonFrame.dwFlags = 0;
	SkeletonFrame.vFloorClipPlane = vFloorClipPlane;
	SkeletonFrame.vNormalToGravity.x = 0;
	SkeletonFrame.vNormalToGravity.y = 0;
	SkeletonFrame.vNormalToGravity.z = 0;
	SkeletonFrame.vNormalToGravity.w = 0;
	SkeletonFrame.SkeletonData = skeletonData;*/
	// End: Tanwi 20130909

    hr = m_pRenderTarget->EndDraw();

    // Device lost, need to recreate the render target
    // We'll dispose it now and retry drawing
    if ( hr == D2DERR_RECREATE_TARGET )
    {
        hr = S_OK;
        DiscardDirect2DResources();
        return;
    }
}
//End: Tanwi-20130121


/// <summary>
/// Handle new color data
/// </summary>
/// <returns>true if a frame was processed, false otherwise</returns>
bool CSkeletalViewerApp::Nui_GotColorAlert( )
{
    NUI_IMAGE_FRAME imageFrame;
    bool processedFrame = true;

    HRESULT hr = m_pNuiSensor->NuiImageStreamGetNextFrame( m_pVideoStreamHandle, 0, &imageFrame );

    if ( FAILED( hr ) )
    {
        return false;
    }

    INuiFrameTexture * pTexture = imageFrame.pFrameTexture;
    NUI_LOCKED_RECT LockedRect;
    pTexture->LockRect( 0, &LockedRect, NULL, 0 );
    if ( LockedRect.Pitch != 0 )
    {
#ifndef __CAPTURE_PLAYBACK
		//Original code commented
        m_pDrawColor->Draw( static_cast<BYTE *>(LockedRect.pBits), LockedRect.size );
#else // __CAPTURE_PLAYBACK
		// Added New

		//Begin: Tanwi 20120809
		DWORD frameWidth, frameHeight;
		unsigned int streamID = COLOR_STREAM_ID;

        NuiImageResolutionToSize( imageFrame.eResolution, frameWidth, frameHeight );

		int bufSize = LockedRect.size;
		//int bufSize = (bCapture)?LockedRect.size: (frameHeight * frameWidth * bytesPerPixelColor);

		BYTE * pBufferRun = LockedRect.pBits;
		//End: Tanwi 20120809
		
        m_pDrawColor->Draw( static_cast<BYTE *>(pBufferRun), bufSize );
        
		//Begin: Tanwi 20120809
		
		if (colorDepthDataFP) {
			unsigned int streamID = COLOR_STREAM_ID;

			if (!bColorHeaderCreated) {
				fgetpos (colorDepthDataFP, &posBody);
				fsetpos (colorDepthDataFP, &posColor);

				fwrite(&streamID, sizeof(unsigned int), 1, colorDepthDataFP);
				fwrite(&frameHeight, sizeof(DWORD), 1, colorDepthDataFP);
				fwrite(&frameWidth, sizeof(DWORD), 1, colorDepthDataFP);
				fwrite(&bytesPerPixelColor, sizeof(int), 1, colorDepthDataFP);

				fsetpos (colorDepthDataFP, &posBody);

				bColorHeaderCreated = true;
				nStream++;
			}

#if _DEBUG
			if (debugLogFP != 0) {
				// Dump Frame Information
				fprintf(debugLogFP, "%s, ", (COLOR_STREAM_ID == streamID)? "Color": "Depth");
				fprintf(debugLogFP, "%ld, ", imageFrame.dwFrameNumber);
				fprintf(debugLogFP, "%ld, %ld\n", imageFrame.liTimeStamp.HighPart, imageFrame.liTimeStamp.LowPart);
				fflush(debugLogFP);
			}
#endif // _DEBUG

			fwrite(&streamID, sizeof(unsigned int), 1, colorDepthDataFP);
			fwrite(&(imageFrame.liTimeStamp), sizeof(LARGE_INTEGER), 1, colorDepthDataFP);
			fwrite(&(imageFrame.dwFrameNumber), sizeof(DWORD), 1, colorDepthDataFP);
			fwrite (pBufferRun, sizeof(BYTE), bufSize, colorDepthDataFP);
		}
		
		//End: Tanwi 20120809
#endif // __CAPTURE_PLAYBACK
    }
    else
    {
        OutputDebugString( L"Buffer length of received texture is bogus\r\n" );
        processedFrame = false;
    }

    pTexture->UnlockRect( 0 );

    m_pNuiSensor->NuiImageStreamReleaseFrame( m_pVideoStreamHandle, &imageFrame );

    return processedFrame;
}


// BEGIN: Tanwi: 20130513
/// <summary>
/// Handle new IR data
/// </summary>
/// <returns>true if a frame was processed, false otherwise</returns>
void CSkeletalViewerApp::Nui_GotIRAlert( )
{
	 HRESULT hr;
    NUI_IMAGE_FRAME imageFrame;

    // Attempt to get the color frame
    hr = m_pNuiSensor->NuiImageStreamGetNextFrame(m_pIRStreamHandle, 0, &imageFrame);
    if (FAILED(hr))
    {
        return;
    }

    INuiFrameTexture * pTexture = imageFrame.pFrameTexture;
    NUI_LOCKED_RECT LockedRect;

	DWORD frameWidth, frameHeight;
	
    NuiImageResolutionToSize( imageFrame.eResolution, frameWidth, frameHeight );

    // Lock the frame data so the Kinect knows not to modify it while we're reading it
    pTexture->LockRect(0, &LockedRect, NULL, 0);

    // Make sure we've received valid data
    if (LockedRect.Pitch != 0)
    {
        if (m_pTempIRBuffer == NULL)
        {
            m_pTempIRBuffer = new RGBQUAD[frameWidth * frameHeight];
        }

        for (int i = 0; i < frameWidth * frameHeight; ++i)
        {
            BYTE intensity = reinterpret_cast<USHORT*>(LockedRect.pBits)[i] >> 8;

            RGBQUAD *pQuad = &m_pTempIRBuffer[i];
            pQuad->rgbBlue = intensity;
            pQuad->rgbGreen = intensity;
            pQuad->rgbRed = intensity;
            pQuad->rgbReserved = 255;
        }

        // Draw the data with Direct2D
        m_pDrawIR->Draw(reinterpret_cast<BYTE*>(m_pTempIRBuffer), frameWidth * frameHeight * sizeof(RGBQUAD));
    }

    // We're done with the texture so unlock it
    pTexture->UnlockRect(0);

    // Release the frame
    m_pNuiSensor->NuiImageStreamReleaseFrame(m_pIRStreamHandle, &imageFrame);
}
// END: Tanwi: 20130513


/// <summary>
/// Handle new depth data
/// </summary>
/// <returns>true if a frame was processed, false otherwise</returns>
bool CSkeletalViewerApp::Nui_GotDepthAlert( )
{
    NUI_IMAGE_FRAME imageFrame;
    bool processedFrame = true;

    HRESULT hr = m_pNuiSensor->NuiImageStreamGetNextFrame(
        m_pDepthStreamHandle,
        0,
        &imageFrame );

    if ( FAILED( hr ) )
    {
        return false;
    }

    INuiFrameTexture * pTexture = imageFrame.pFrameTexture;
    NUI_LOCKED_RECT LockedRect;
    pTexture->LockRect( 0, &LockedRect, NULL, 0 );
    if ( 0 != LockedRect.Pitch )
    {
        DWORD frameWidth, frameHeight;
        
        NuiImageResolutionToSize( imageFrame.eResolution, frameWidth, frameHeight );
        
        // draw the bits to the bitmap
        BYTE * rgbrun = m_depthRGBX;

#ifndef __CAPTURE_PLAYBACK
 		// Original Code Commented // Tanwi: 20120808
		const USHORT * pBufferRun = (const USHORT *)LockedRect.pBits;
#else // __CAPTURE_PLAYBACK

		//Begin: Tanwi 20120809
//		int bufSize = (bCapture)?LockedRect.size: (frameWidth * frameHeight * sizeof(short));
		assert(LockedRect.size == (frameWidth * frameHeight * sizeof(short)));

		unsigned int streamID = DEPTH_STREAM_ID;

		const USHORT * pBufferRun = (const USHORT *)LockedRect.pBits;
		//const USHORT * pBufferRun = (const USHORT *)((bCapture)?LockedRect.pBits: (byte *)pInputBufferRunDepth);
		// End: Tanwi 20120808
#endif // __CAPTURE_PLAYBACK

		// end pixel is start + width*height - 1
        const USHORT * pBufferEnd = pBufferRun + (frameWidth * frameHeight);

#ifdef __CAPTURE_PLAYBACK
		// Begin: Tanwi 20120808
		
		if (colorDepthDataFP) {
			unsigned int streamID = DEPTH_STREAM_ID;
			unsigned int size = sizeof(short);

			if (!bDepthHeaderCreated) {
				fgetpos (colorDepthDataFP, &posBody);
				fsetpos (colorDepthDataFP, &posDepth);

				fwrite(&streamID, sizeof(unsigned int), 1, colorDepthDataFP);
				fwrite(&frameHeight, sizeof(DWORD), 1, colorDepthDataFP);
				fwrite(&frameWidth, sizeof(DWORD), 1, colorDepthDataFP);
				fwrite(&size, sizeof(unsigned int), 1, colorDepthDataFP);

				fsetpos (colorDepthDataFP, &posBody);

				bDepthHeaderCreated = true;
				nStream++;
			}

#if _DEBUG
			if (debugLogFP != 0) {
				// Dump Frame Information
				fprintf(debugLogFP, "%s, ", (COLOR_STREAM_ID == streamID)? "Color": "Depth");
				fprintf(debugLogFP, "%ld, ", imageFrame.dwFrameNumber);
				fprintf(debugLogFP, "%ld, %ld\n", imageFrame.liTimeStamp.HighPart, imageFrame.liTimeStamp.LowPart);
				fflush(debugLogFP);
			}
#endif // _DEBUG

			fwrite(&streamID, sizeof(unsigned int), 1, colorDepthDataFP);
			fwrite(&(imageFrame.liTimeStamp), sizeof(LARGE_INTEGER), 1, colorDepthDataFP);
			fwrite(&(imageFrame.dwFrameNumber), sizeof(DWORD), 1, colorDepthDataFP);
			fwrite (pBufferRun, sizeof(short), frameWidth * frameHeight, colorDepthDataFP);
		}
		
		// End: Tanwi 20120808
#endif // __CAPTURE_PLAYBACK

		assert( frameWidth * frameHeight * g_BytesPerPixel <= ARRAYSIZE(m_depthRGBX) );

        while ( pBufferRun < pBufferEnd )
        {
            USHORT depth     = *pBufferRun;
            USHORT realDepth = NuiDepthPixelToDepth(depth);
            USHORT player    = NuiDepthPixelToPlayerIndex(depth);

            
            //Begin: Tanwi 20130611
			if(realDepth == 0){
				*(rgbrun++) = 255;
				*(rgbrun++) = 0;
				*(rgbrun++) = 0;
			 }
			 else if(realDepth == 8191){
				*(rgbrun++) = 0;
				*(rgbrun++) = 255;
				*(rgbrun++) = 0;
			 }
			 else if(realDepth == 4095){
				*(rgbrun++) = 0;
				*(rgbrun++) = 0;
				*(rgbrun++) = 255;
			 }
			 else{
				// transform 13-bit depth information into an 8-bit intensity appropriate
                // for display (we disregard information in most significant bit)
				BYTE intensity = static_cast<BYTE>(~(realDepth >> 4));

				// tint the intensity by dividing by per-player values
				*(rgbrun++) = intensity >> g_IntensityShiftByPlayerB[player];
				*(rgbrun++) = intensity >> g_IntensityShiftByPlayerG[player];
				*(rgbrun++) = intensity >> g_IntensityShiftByPlayerR[player];
			 }
			 //End: Tanwi 20130611

            // no alpha information, skip the last byte
            ++rgbrun;

            ++pBufferRun;
        }

        m_pDrawDepth->Draw( m_depthRGBX, frameWidth * frameHeight * g_BytesPerPixel );
    }
    else
    {
        processedFrame = false;
        OutputDebugString( L"Buffer length of received texture is bogus\r\n" );
    }

    pTexture->UnlockRect( 0 );

    m_pNuiSensor->NuiImageStreamReleaseFrame( m_pDepthStreamHandle, &imageFrame );

    return processedFrame;
}

/// <summary>
/// Blank the skeleton display
/// </summary>
void CSkeletalViewerApp::Nui_BlankSkeletonScreen( )
{
    m_pRenderTarget->BeginDraw();
    m_pRenderTarget->Clear( );
    m_pRenderTarget->EndDraw( );
}

/// <summary>
/// Draws a line between two bones
/// </summary>
/// <param name="skel">skeleton to draw bones from</param>
/// <param name="bone0">bone to start drawing from</param>
/// <param name="bone1">bone to end drawing at</param>
void CSkeletalViewerApp::Nui_DrawBone( const NUI_SKELETON_DATA & skel, NUI_SKELETON_POSITION_INDEX bone0, NUI_SKELETON_POSITION_INDEX bone1 )
{
    NUI_SKELETON_POSITION_TRACKING_STATE bone0State = skel.eSkeletonPositionTrackingState[bone0];
    NUI_SKELETON_POSITION_TRACKING_STATE bone1State = skel.eSkeletonPositionTrackingState[bone1];

    // If we can't find either of these joints, exit
    if ( bone0State == NUI_SKELETON_POSITION_NOT_TRACKED || bone1State == NUI_SKELETON_POSITION_NOT_TRACKED )
    {
        return;
    }
    
    // Don't draw if both points are inferred
    if ( bone0State == NUI_SKELETON_POSITION_INFERRED && bone1State == NUI_SKELETON_POSITION_INFERRED )
    {
        return;
    }

    // We assume all drawn bones are inferred unless BOTH joints are tracked
    if ( bone0State == NUI_SKELETON_POSITION_TRACKED && bone1State == NUI_SKELETON_POSITION_TRACKED )
    {
        m_pRenderTarget->DrawLine( m_Points[bone0], m_Points[bone1], m_pBrushBoneTracked, g_TrackedBoneThickness );
    }
    else
    {
        m_pRenderTarget->DrawLine( m_Points[bone0], m_Points[bone1], m_pBrushBoneInferred, g_InferredBoneThickness );
    }
}

/// <summary>
/// Draws a skeleton
/// </summary>
/// <param name="skel">skeleton to draw</param>
/// <param name="windowWidth">width (in pixels) of output buffer</param>
/// <param name="windowHeight">height (in pixels) of output buffer</param>
void CSkeletalViewerApp::Nui_DrawSkeleton( const NUI_SKELETON_DATA & skel, int windowWidth, int windowHeight )
{      
    int i;

    for (i = 0; i < NUI_SKELETON_POSITION_COUNT; i++)
    {
        m_Points[i] = SkeletonToScreen( skel.SkeletonPositions[i], windowWidth, windowHeight );
    }

    // Render Torso
    Nui_DrawBone( skel, NUI_SKELETON_POSITION_HEAD, NUI_SKELETON_POSITION_SHOULDER_CENTER );
    Nui_DrawBone( skel, NUI_SKELETON_POSITION_SHOULDER_CENTER, NUI_SKELETON_POSITION_SHOULDER_LEFT );
    Nui_DrawBone( skel, NUI_SKELETON_POSITION_SHOULDER_CENTER, NUI_SKELETON_POSITION_SHOULDER_RIGHT );
    Nui_DrawBone( skel, NUI_SKELETON_POSITION_SHOULDER_CENTER, NUI_SKELETON_POSITION_SPINE );
    Nui_DrawBone( skel, NUI_SKELETON_POSITION_SPINE, NUI_SKELETON_POSITION_HIP_CENTER );
    Nui_DrawBone( skel, NUI_SKELETON_POSITION_HIP_CENTER, NUI_SKELETON_POSITION_HIP_LEFT );
    Nui_DrawBone( skel, NUI_SKELETON_POSITION_HIP_CENTER, NUI_SKELETON_POSITION_HIP_RIGHT );

    // Left Arm
    Nui_DrawBone( skel, NUI_SKELETON_POSITION_SHOULDER_LEFT, NUI_SKELETON_POSITION_ELBOW_LEFT );
    Nui_DrawBone( skel, NUI_SKELETON_POSITION_ELBOW_LEFT, NUI_SKELETON_POSITION_WRIST_LEFT );
    Nui_DrawBone( skel, NUI_SKELETON_POSITION_WRIST_LEFT, NUI_SKELETON_POSITION_HAND_LEFT );

    // Right Arm
    Nui_DrawBone( skel, NUI_SKELETON_POSITION_SHOULDER_RIGHT, NUI_SKELETON_POSITION_ELBOW_RIGHT );
    Nui_DrawBone( skel, NUI_SKELETON_POSITION_ELBOW_RIGHT, NUI_SKELETON_POSITION_WRIST_RIGHT );
    Nui_DrawBone( skel, NUI_SKELETON_POSITION_WRIST_RIGHT, NUI_SKELETON_POSITION_HAND_RIGHT );

    // Left Leg
    Nui_DrawBone( skel, NUI_SKELETON_POSITION_HIP_LEFT, NUI_SKELETON_POSITION_KNEE_LEFT );
    Nui_DrawBone( skel, NUI_SKELETON_POSITION_KNEE_LEFT, NUI_SKELETON_POSITION_ANKLE_LEFT );
    Nui_DrawBone( skel, NUI_SKELETON_POSITION_ANKLE_LEFT, NUI_SKELETON_POSITION_FOOT_LEFT );

    // Right Leg
    Nui_DrawBone( skel, NUI_SKELETON_POSITION_HIP_RIGHT, NUI_SKELETON_POSITION_KNEE_RIGHT );
    Nui_DrawBone( skel, NUI_SKELETON_POSITION_KNEE_RIGHT, NUI_SKELETON_POSITION_ANKLE_RIGHT );
    Nui_DrawBone( skel, NUI_SKELETON_POSITION_ANKLE_RIGHT, NUI_SKELETON_POSITION_FOOT_RIGHT );
    
    // Draw the joints in a different color
    for ( i = 0; i < NUI_SKELETON_POSITION_COUNT; i++ )
    {
        D2D1_ELLIPSE ellipse = D2D1::Ellipse( m_Points[i], g_JointThickness, g_JointThickness );

        if ( skel.eSkeletonPositionTrackingState[i] == NUI_SKELETON_POSITION_INFERRED )
        {
            m_pRenderTarget->DrawEllipse(ellipse, m_pBrushJointInferred);
        }
        else if ( skel.eSkeletonPositionTrackingState[i] == NUI_SKELETON_POSITION_TRACKED )
        {
            m_pRenderTarget->DrawEllipse(ellipse, m_pBrushJointTracked);
        }
    }
}

/// <summary>
/// Determines which skeletons to track and tracks them
/// </summary>
/// <param name="skel">skeleton frame information</param>
void CSkeletalViewerApp::UpdateTrackedSkeletons( const NUI_SKELETON_FRAME & skel )
{
    DWORD nearestIDs[2] = { 0, 0 };
    USHORT nearestDepths[2] = { NUI_IMAGE_DEPTH_MAXIMUM, NUI_IMAGE_DEPTH_MAXIMUM };

    // Purge old sticky skeleton IDs, if the user has left the frame, etc
    bool stickyID0Found = false;
    bool stickyID1Found = false;
    for ( int i = 0 ; i < NUI_SKELETON_COUNT; i++ )
    {
        NUI_SKELETON_TRACKING_STATE trackingState = skel.SkeletonData[i].eTrackingState;

        if ( trackingState == NUI_SKELETON_TRACKED || trackingState == NUI_SKELETON_POSITION_ONLY )
        {
            if ( skel.SkeletonData[i].dwTrackingID == m_StickySkeletonIds[0] )
            {
                stickyID0Found = true;
            }
            else if ( skel.SkeletonData[i].dwTrackingID == m_StickySkeletonIds[1] )
            {
                stickyID1Found = true;
            }
        }
    }

    if ( !stickyID0Found && stickyID1Found )
    {
        m_StickySkeletonIds[0] = m_StickySkeletonIds[1];
        m_StickySkeletonIds[1] = 0;
    }
    else if ( !stickyID0Found )
    {
        m_StickySkeletonIds[0] = 0;
    }
    else if ( !stickyID1Found )
    {
        m_StickySkeletonIds[1] = 0;
    }

    // Calculate nearest and sticky skeletons
    for ( int i = 0 ; i < NUI_SKELETON_COUNT; i++ )
    {
        NUI_SKELETON_TRACKING_STATE trackingState = skel.SkeletonData[i].eTrackingState;

        if ( trackingState == NUI_SKELETON_TRACKED || trackingState == NUI_SKELETON_POSITION_ONLY )
        {
            // Save SkeletonIds for sticky mode if there's none already saved
            if ( 0 == m_StickySkeletonIds[0] && m_StickySkeletonIds[1] != skel.SkeletonData[i].dwTrackingID )
            {
                m_StickySkeletonIds[0] = skel.SkeletonData[i].dwTrackingID;
            }
            else if ( 0 == m_StickySkeletonIds[1] && m_StickySkeletonIds[0] != skel.SkeletonData[i].dwTrackingID )
            {
                m_StickySkeletonIds[1] = skel.SkeletonData[i].dwTrackingID;
            }

            LONG x, y;
            USHORT depth;

            // calculate the skeleton's position on the screen
            NuiTransformSkeletonToDepthImage( skel.SkeletonData[i].Position, &x, &y, &depth );

            if ( depth < nearestDepths[0] )
            {
                nearestDepths[1] = nearestDepths[0];
                nearestIDs[1] = nearestIDs[0];

                nearestDepths[0] = depth;
                nearestIDs[0] = skel.SkeletonData[i].dwTrackingID;
            }
            else if ( depth < nearestDepths[1] )
            {
                nearestDepths[1] = depth;
                nearestIDs[1] = skel.SkeletonData[i].dwTrackingID;
            }
        }
    }

    if ( SV_TRACKED_SKELETONS_NEAREST1 == m_TrackedSkeletons || SV_TRACKED_SKELETONS_NEAREST2 == m_TrackedSkeletons )
    {
        // Only track the closest single skeleton in nearest 1 mode
        if ( SV_TRACKED_SKELETONS_NEAREST1 == m_TrackedSkeletons )
        {
            nearestIDs[1] = 0;
        }
        m_pNuiSensor->NuiSkeletonSetTrackedSkeletons(nearestIDs);
    }

    if ( SV_TRACKED_SKELETONS_STICKY1 == m_TrackedSkeletons || SV_TRACKED_SKELETONS_STICKY2 == m_TrackedSkeletons )
    {
        DWORD stickyIDs[2] = { m_StickySkeletonIds[0], m_StickySkeletonIds[1] };

        // Only track a single skeleton in sticky 1 mode
        if ( SV_TRACKED_SKELETONS_STICKY1 == m_TrackedSkeletons )
        {
            stickyIDs[1] = 0;
        }
        m_pNuiSensor->NuiSkeletonSetTrackedSkeletons(stickyIDs);
    }
}

/// <summary>
/// Converts a skeleton point to screen space
/// </summary>
/// <param name="skeletonPoint">skeleton point to tranform</param>
/// <param name="width">width (in pixels) of output buffer</param>
/// <param name="height">height (in pixels) of output buffer</param>
/// <returns>point in screen-space</returns>
D2D1_POINT_2F CSkeletalViewerApp::SkeletonToScreen( Vector4 skeletonPoint, int width, int height )
{
    LONG x, y;
    USHORT depth;

    // calculate the skeleton's position on the screen
    // NuiTransformSkeletonToDepthImage returns coordinates in NUI_IMAGE_RESOLUTION_320x240 space
    NuiTransformSkeletonToDepthImage( skeletonPoint, &x, &y, &depth );

    float screenPointX = static_cast<float>(x * width) / g_ScreenWidth;
    float screenPointY = static_cast<float>(y * height) / g_ScreenHeight;

    return D2D1::Point2F(screenPointX, screenPointY);
}

/// <summary>
/// Handle new skeleton data
/// </summary>
bool CSkeletalViewerApp::Nui_GotSkeletonAlert( )
{
    NUI_SKELETON_FRAME SkeletonFrame = {0};

    bool foundSkeleton = false;

    if ( SUCCEEDED(m_pNuiSensor->NuiSkeletonGetNextFrame( 0, &SkeletonFrame )) )
    {
        for ( int i = 0 ; i < NUI_SKELETON_COUNT ; i++ )
        {
            NUI_SKELETON_TRACKING_STATE trackingState = SkeletonFrame.SkeletonData[i].eTrackingState;

            if ( trackingState == NUI_SKELETON_TRACKED || trackingState == NUI_SKELETON_POSITION_ONLY )
            {
                foundSkeleton = true;
            }
        }
    }

    // no skeletons!
    if( !foundSkeleton )
    {
        return true;
    }

    // smooth out the skeleton data
    HRESULT hr = m_pNuiSensor->NuiTransformSmooth(&SkeletonFrame,NULL);
    if ( FAILED(hr) )
    {
        return false;
    }

    // we found a skeleton, re-start the skeletal timer
    m_bScreenBlanked = false;
    m_LastSkeletonFoundTime = timeGetTime( );

 	//Begin: Tanwi 20130121
	unsigned int streamID = SKELETON_STREAM_ID;
	if (captureSkeleton) {
		fwrite(&streamID, sizeof(unsigned int), 1, colorDepthDataFP);	
		fwrite(&SkeletonFrame.liTimeStamp, sizeof( LARGE_INTEGER ), 1, colorDepthDataFP);	
		fwrite(&SkeletonFrame.dwFrameNumber, sizeof( DWORD ), 1, colorDepthDataFP);	
		fwrite(&SkeletonFrame.vFloorClipPlane, sizeof( Vector4 ), 1, colorDepthDataFP);	
	}
	//End: Tanwi 20130121

   // Endure Direct2D is ready to draw
    hr = EnsureDirect2DResources( );
    if ( FAILED( hr ) )
    {
        return false;
    }

    m_pRenderTarget->BeginDraw();
    m_pRenderTarget->Clear( );
    
    RECT rct;
    GetClientRect( GetDlgItem( m_hWnd, IDC_SKELETALVIEW ), &rct);
    int width = rct.right;
    int height = rct.bottom;

    for ( int i = 0 ; i < NUI_SKELETON_COUNT; i++ )
    {
        NUI_SKELETON_TRACKING_STATE trackingState = SkeletonFrame.SkeletonData[i].eTrackingState;

		//Begin: Tanwi 20130121
		if (captureSkeleton) {
			fwrite(&trackingState, sizeof(NUI_SKELETON_TRACKING_STATE), 1, colorDepthDataFP);	
		}
		//End: Tanwi 20130121

        if ( trackingState == NUI_SKELETON_TRACKED )
        {
            // We're tracking the skeleton, draw it
            Nui_DrawSkeleton( SkeletonFrame.SkeletonData[i], width, height );

			//Begin: Tanwi 20130121
			if (captureSkeleton) {
				if (debugSkeletonLogFP0) {
					fprintf(debugSkeletonLogFP0, "TRACKED, ");
					fprintf(debugSkeletonLogFP0, "%ld, ", SkeletonFrame.SkeletonData[i].dwTrackingID);
					fprintf(debugSkeletonLogFP0, "%ld, ", SkeletonFrame.SkeletonData[i].dwEnrollmentIndex);
					fprintf(debugSkeletonLogFP0, "%ld, ", SkeletonFrame.SkeletonData[i].dwUserIndex);
					fprintf(debugSkeletonLogFP0, "%f, ", SkeletonFrame.SkeletonData[i].Position.x);
					fprintf(debugSkeletonLogFP0, "%f, ", SkeletonFrame.SkeletonData[i].Position.y);
					fprintf(debugSkeletonLogFP0, "%f\n", SkeletonFrame.SkeletonData[i].Position.z);
				}
				/*for (int i = 0; i < 20; i++){
					fprintf(debugSkeletonLogFP0, "%f, ", SkeletonFrame.SkeletonData[i].SkeletonPositions);
				}*/
			
		
				fwrite(&SkeletonFrame.SkeletonData[i].dwTrackingID, sizeof( DWORD ), 1, colorDepthDataFP);	
				fwrite(&SkeletonFrame.SkeletonData[i].dwEnrollmentIndex, sizeof( DWORD ), 1, colorDepthDataFP);	
				fwrite(&SkeletonFrame.SkeletonData[i].dwUserIndex, sizeof( DWORD ), 1, colorDepthDataFP);	
				fwrite(&SkeletonFrame.SkeletonData[i].Position, sizeof( Vector4 ), 1, colorDepthDataFP);	
				fwrite(&SkeletonFrame.SkeletonData[i].SkeletonPositions, sizeof( Vector4  ), 20, colorDepthDataFP);	
				fwrite(&SkeletonFrame.SkeletonData[i].eSkeletonPositionTrackingState, sizeof( NUI_SKELETON_POSITION_TRACKING_STATE ), 20, colorDepthDataFP);	
				fwrite(&SkeletonFrame.SkeletonData[i].dwQualityFlags, sizeof( DWORD   ), 1, colorDepthDataFP);	
			}

			//End: Tanwi 20130121
        }
        else if ( trackingState == NUI_SKELETON_POSITION_ONLY )
        {
			//Begin: Tanwi 20130121
			if (captureSkeleton) {
				if (debugSkeletonLogFP0) {
					fprintf(debugSkeletonLogFP0, "POSITION_ONLY, ");
					fprintf(debugSkeletonLogFP0, "%f, ", SkeletonFrame.SkeletonData[i].Position.x);
					fprintf(debugSkeletonLogFP0, "%f, ", SkeletonFrame.SkeletonData[i].Position.y);
					fprintf(debugSkeletonLogFP0, "%f\n", SkeletonFrame.SkeletonData[i].Position.z);
				}

				fwrite(&SkeletonFrame.SkeletonData[i].Position, sizeof( Vector4  ), 1, colorDepthDataFP);
			}

			//End: Tanwi 20130121

            // we've only received the center point of the skeleton, draw that
            D2D1_ELLIPSE ellipse = D2D1::Ellipse(
                SkeletonToScreen( SkeletonFrame.SkeletonData[i].Position, width, height ),
                g_JointThickness,
                g_JointThickness
                );

            m_pRenderTarget->DrawEllipse(ellipse, m_pBrushJointTracked);
        }

		// Begin: Tanwi 20130909
		const NUI_SKELETON_DATA & skeleton = SkeletonFrame.SkeletonData[i];
 
		NUI_SKELETON_BONE_ORIENTATION boneOrientations[NUI_SKELETON_POSITION_COUNT];
		NuiSkeletonCalculateBoneOrientations(&skeleton, boneOrientations);

		if (skeleton.eTrackingState == NUI_SKELETON_TRACKED)
		{
			for (int j = 0; j < NUI_SKELETON_POSITION_COUNT; j++)
			{
			  NUI_SKELETON_BONE_ORIENTATION & orientation = boneOrientations[j];

			  if (0 != SkeletonOrientationWrite) {

				  fprintf(SkeletonOrientationWrite, "%d", orientation.startJoint);
				  fprintf(SkeletonOrientationWrite, " %d", orientation.endJoint);
				  fprintf(SkeletonOrientationWrite, " %f", orientation.absoluteRotation.rotationMatrix.M11);
				  fprintf(SkeletonOrientationWrite, " %f", orientation.absoluteRotation.rotationMatrix.M12);
				  fprintf(SkeletonOrientationWrite, " %f", orientation.absoluteRotation.rotationMatrix.M13);
				  fprintf(SkeletonOrientationWrite, " %f", orientation.absoluteRotation.rotationMatrix.M21);
				  fprintf(SkeletonOrientationWrite, " %f", orientation.absoluteRotation.rotationMatrix.M22);
				  fprintf(SkeletonOrientationWrite, " %f", orientation.absoluteRotation.rotationMatrix.M23);
				  fprintf(SkeletonOrientationWrite, " %f", orientation.absoluteRotation.rotationMatrix.M31);
				  fprintf(SkeletonOrientationWrite, " %f", orientation.absoluteRotation.rotationMatrix.M32);
				  fprintf(SkeletonOrientationWrite, " %f\n", orientation.absoluteRotation.rotationMatrix.M33);
				  


				  fprintf(SkeletonOrientationWrite, " %f", orientation.hierarchicalRotation.rotationMatrix.M11);
				  fprintf(SkeletonOrientationWrite, " %f", orientation.hierarchicalRotation.rotationMatrix.M12);
				  fprintf(SkeletonOrientationWrite, " %f", orientation.hierarchicalRotation.rotationMatrix.M13);
				  //fprintf(SkeletonOrientationWrite, " %f", orientation.hierarchicalRotation.rotationMatrix.M14);
				  fprintf(SkeletonOrientationWrite, " %f", orientation.hierarchicalRotation.rotationMatrix.M21);
				  fprintf(SkeletonOrientationWrite, " %f", orientation.hierarchicalRotation.rotationMatrix.M22);
				  fprintf(SkeletonOrientationWrite, " %f", orientation.hierarchicalRotation.rotationMatrix.M23);
				  //fprintf(SkeletonOrientationWrite, " %f", orientation.hierarchicalRotation.rotationMatrix.M24);
				  fprintf(SkeletonOrientationWrite, " %f", orientation.hierarchicalRotation.rotationMatrix.M31);
				  fprintf(SkeletonOrientationWrite, " %f", orientation.hierarchicalRotation.rotationMatrix.M32);
				  fprintf(SkeletonOrientationWrite, " %f\n", orientation.hierarchicalRotation.rotationMatrix.M33);
				  //fprintf(SkeletonOrientationWrite, " %f", orientation.hierarchicalRotation.rotationMatrix.M34);
				  //fprintf(SkeletonOrientationWrite, " %f", orientation.hierarchicalRotation.rotationMatrix.M41);
				  //fprintf(SkeletonOrientationWrite, " %f", orientation.hierarchicalRotation.rotationMatrix.M42);
				  //fprintf(SkeletonOrientationWrite, " %f", orientation.hierarchicalRotation.rotationMatrix.M43);
				  //fprintf(SkeletonOrientationWrite, " %f\n", orientation.hierarchicalRotation.rotationMatrix.M44);
				 
			  }
                   
			}
			fprintf(SkeletonOrientationWrite, "\n");
		}
		//End: Tanwi 20130909
    }

    hr = m_pRenderTarget->EndDraw();

    UpdateTrackedSkeletons( SkeletonFrame );

    // Device lost, need to recreate the render target
    // We'll dispose it now and retry drawing
    if ( hr == D2DERR_RECREATE_TARGET )
    {
        hr = S_OK;
        DiscardDirect2DResources();
        return false;
    }

    return true;
}

/// <summary>
/// Invoked when the user changes the selection of tracked skeletons
/// </summary>
/// <param name="mode">skelton tracking mode to switch to</param>
void CSkeletalViewerApp::UpdateTrackedSkeletonSelection( int mode )
{
    m_TrackedSkeletons = mode;

    UpdateSkeletonTrackingFlag(
        NUI_SKELETON_TRACKING_FLAG_TITLE_SETS_TRACKED_SKELETONS,
        (mode != SV_TRACKED_SKELETONS_DEFAULT));
}

/// <summary>
/// Invoked when the user changes the tracking mode
/// </summary>
/// <param name="mode">tracking mode to switch to</param>
void CSkeletalViewerApp::UpdateTrackingMode( int mode )
{
    UpdateSkeletonTrackingFlag(
        NUI_SKELETON_TRACKING_FLAG_ENABLE_SEATED_SUPPORT,
        (mode == SV_TRACKING_MODE_SEATED) );
}

/// <summary>
/// Invoked when the user changes the range
/// </summary>
/// <param name="mode">range to switch to</param>
void CSkeletalViewerApp::UpdateRange( int mode )
{
    UpdateDepthStreamFlag(
        NUI_IMAGE_STREAM_FLAG_ENABLE_NEAR_MODE,
        (mode != SV_RANGE_DEFAULT) );
}

/// <summary>
/// Sets or clears the specified skeleton tracking flag
/// </summary>
/// <param name="flag">flag to set or clear</param>
/// <param name="value">true to set, false to clear</param>
void CSkeletalViewerApp::UpdateSkeletonTrackingFlag( DWORD flag, bool value )
{
    DWORD newFlags = m_SkeletonTrackingFlags;

    if (value)
    {
        newFlags |= flag;
    }
    else
    {
        newFlags &= ~flag;
    }

    if (NULL != m_pNuiSensor && newFlags != m_SkeletonTrackingFlags)
    {
        if ( !HasSkeletalEngine(m_pNuiSensor) )
        {
            MessageBoxResource(IDS_ERROR_SKELETONTRACKING, MB_OK | MB_ICONHAND);
        }

        m_SkeletonTrackingFlags = newFlags;

        HRESULT hr = m_pNuiSensor->NuiSkeletonTrackingEnable( m_hNextSkeletonEvent, m_SkeletonTrackingFlags );

        if ( FAILED( hr ) )
        {
            MessageBoxResource(IDS_ERROR_SKELETONTRACKING, MB_OK | MB_ICONHAND);
        }
    }
}

/// <summary>
/// Sets or clears the specified depth stream flag
/// </summary>
/// <param name="flag">flag to set or clear</param>
/// <param name="value">true to set, false to clear</param>
void CSkeletalViewerApp::UpdateDepthStreamFlag( DWORD flag, bool value )
{
    DWORD newFlags = m_DepthStreamFlags;

    if (value)
    {
        newFlags |= flag;
    }
    else
    {
        newFlags &= ~flag;
    }

    if (NULL != m_pNuiSensor && newFlags != m_DepthStreamFlags)
    {
        m_DepthStreamFlags = newFlags;
        m_pNuiSensor->NuiImageStreamSetImageFrameFlags( m_pDepthStreamHandle, m_DepthStreamFlags );
    }
}

/// <summary>
/// Ensure necessary Direct2d resources are created
/// </summary>
/// <returns>S_OK if successful, otherwise an error code</returns>
HRESULT CSkeletalViewerApp::EnsureDirect2DResources()
{
    HRESULT hr = S_OK;

    if ( !m_pRenderTarget )
    {
        RECT rc;
        GetWindowRect( GetDlgItem( m_hWnd, IDC_SKELETALVIEW ), &rc );  
    
        int width = rc.right - rc.left;
        int height = rc.bottom - rc.top;
        D2D1_SIZE_U size = D2D1::SizeU( width, height );
        D2D1_RENDER_TARGET_PROPERTIES rtProps = D2D1::RenderTargetProperties();
        rtProps.pixelFormat = D2D1::PixelFormat( DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_IGNORE);
        rtProps.usage = D2D1_RENDER_TARGET_USAGE_GDI_COMPATIBLE;

        // Create a Hwnd render target, in order to render to the window set in initialize
        hr = m_pD2DFactory->CreateHwndRenderTarget(
            rtProps,
            D2D1::HwndRenderTargetProperties(GetDlgItem( m_hWnd, IDC_SKELETALVIEW), size),
            &m_pRenderTarget
            );
        if ( FAILED( hr ) )
        {
            MessageBoxResource( IDS_ERROR_DRAWDEVICE, MB_OK | MB_ICONHAND );
            return E_FAIL;
        }

        //light green
        m_pRenderTarget->CreateSolidColorBrush( D2D1::ColorF( 68, 192, 68 ), &m_pBrushJointTracked );

        //yellow
        m_pRenderTarget->CreateSolidColorBrush( D2D1::ColorF( 255, 255, 0 ), &m_pBrushJointInferred );

        //green
        m_pRenderTarget->CreateSolidColorBrush( D2D1::ColorF( 0, 128, 0 ), &m_pBrushBoneTracked );

        //gray
        m_pRenderTarget->CreateSolidColorBrush( D2D1::ColorF( 128, 128, 128 ), &m_pBrushBoneInferred );
    }

    return hr;
}

/// <summary>
/// Dispose Direct2d resources 
/// </summary>
void CSkeletalViewerApp::DiscardDirect2DResources( )
{
    SafeRelease(m_pRenderTarget);

    SafeRelease( m_pBrushJointTracked );
    SafeRelease( m_pBrushJointInferred );
    SafeRelease( m_pBrushBoneTracked );
    SafeRelease( m_pBrushBoneInferred );
}
