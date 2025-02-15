﻿// MorphisChannelSwitchingDlg.cpp : implementation file
//
// Copyright © Matrox Electronic Systems Ltd., 1992-YYYY.
// All Rights Reserved


#include "stdafx.h"
#include "MorphisChannelSwitching.h"
#include "MorphisChannelSwitchingDlg.h"
#include ".\morphischannelswitchingdlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CMorphisChannelSwitchingDlg dialog
CMorphisChannelSwitchingDlg::CMorphisChannelSwitchingDlg(CWnd* pParent /*=NULL*/)
: CDialog(CMorphisChannelSwitchingDlg::IDD, pParent)
   {
   m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
   InitializeCriticalSection(&Lock);
   }
CMorphisChannelSwitchingDlg::~CMorphisChannelSwitchingDlg()
   {
   DeleteCriticalSection(&Lock);
   }

void CMorphisChannelSwitchingDlg::DoDataExchange(CDataExchange* pDX)
   {
   CDialog::DoDataExchange(pDX);
   DDX_Control(pDX, IDC_SLIDER_LOCK_SENSITIVITY, SliderLockSensitivity);
   DDX_Control(pDX, IDC_LOCK_SENSITIVITY, LockSensitivity);
   DDX_Control(pDX, IDC_GainLuma, GainLuma);
   DDX_Control(pDX, IDC_CONTRAST, Contrast);
   DDX_Control(pDX, IDC_BRIGHTNESS, Brightness);
   DDX_Control(pDX, IDC_SLIDER_GainLuma, SliderGainLuma);
   DDX_Control(pDX, IDC_SLIDER_CONTRAST, SliderContrast);
   DDX_Control(pDX, IDC_SLIDER_BRIGHTNESS, SliderBrightness);
   DDX_Control(pDX, IDC_CHECK_SET_ALL_CHANNELS, SetAllChannels);
   DDX_Control(pDX, IDC_EDIT_NBR_OF_CHANNELS, NumberOfChannels);
   DDX_Control(pDX, IDC_COMBO_DIG_TO_USE, ComboDigitizers);
   }

BEGIN_MESSAGE_MAP(CMorphisChannelSwitchingDlg, CDialog)
   ON_WM_PAINT()
   ON_WM_QUERYDRAGICON()
   //}}AFX_MSG_MAP
   ON_NOTIFY(NM_RELEASEDCAPTURE, IDC_SLIDER_LOCK_SENSITIVITY,
             OnNMReleasedcaptureSliderLockSensitivity)
   ON_NOTIFY(NM_RELEASEDCAPTURE, IDC_SLIDER_GainLuma, OnNMReleasedcaptureSliderGainLuma)
   ON_NOTIFY(NM_RELEASEDCAPTURE, IDC_SLIDER_CONTRAST, OnNMReleasedcaptureSliderContrast)
   ON_NOTIFY(NM_RELEASEDCAPTURE, IDC_SLIDER_BRIGHTNESS, OnNMReleasedcaptureSliderBrightness)
   ON_BN_CLICKED(IDC_CHECK_SET_ALL_CHANNELS, OnBnClickedCheckSetAllChannels)
   ON_CBN_SELCHANGE(IDC_COMBO_DIG_TO_USE, OnCbnSelchangeComboDigToUse)
   ON_BN_CLICKED(IDC_BUTTON_START, OnBnClickedButtonStart)
   ON_BN_CLICKED(IDC_BUTTON_STOP, OnBnClickedButtonStop)
   ON_BN_CLICKED(IDC_RADIO0, OnBnClickedRadio)
   ON_BN_CLICKED(IDC_RADIO1, OnBnClickedRadio)
   ON_BN_CLICKED(IDC_RADIO2, OnBnClickedRadio)
   ON_BN_CLICKED(IDC_RADIO3, OnBnClickedRadio)
   ON_BN_CLICKED(IDC_RADIO4, OnBnClickedRadio)
   ON_BN_CLICKED(IDC_RADIO5, OnBnClickedRadio)
   ON_BN_CLICKED(IDC_RADIO6, OnBnClickedRadio)
   ON_BN_CLICKED(IDC_RADIO7, OnBnClickedRadio)
   ON_BN_CLICKED(IDC_RADIO8, OnBnClickedRadio)
   ON_BN_CLICKED(IDC_RADIO9, OnBnClickedRadio)
   ON_BN_CLICKED(IDC_RADIO10, OnBnClickedRadio)
   ON_BN_CLICKED(IDC_RADIO11, OnBnClickedRadio)
   ON_BN_CLICKED(IDC_RADIO12, OnBnClickedRadio)
   ON_BN_CLICKED(IDC_RADIO13, OnBnClickedRadio)
   ON_BN_CLICKED(IDC_RADIO14, OnBnClickedRadio)
   ON_BN_CLICKED(IDC_RADIO15, OnBnClickedRadio)
   ON_BN_CLICKED(IDC_RADIO18_ANY,  OnBnClickedRadioField)
   ON_BN_CLICKED(IDC_RADIO19_ODD,  OnBnClickedRadioField)
   ON_BN_CLICKED(IDC_RADIO20_EVEN, OnBnClickedRadioField)
   ON_BN_CLICKED(IDC_CHECK_AUTO_GAIN, OnBnClickedCheckAutoGain)
   ON_BN_CLICKED(IDC_CHANNEL_ENABLED, OnBnClickedChannelEnabled)
   ON_EN_CHANGE(IDC_EDIT_OVERLAY, OnEnChangeEditOverlay)
   ON_WM_TIMER()
END_MESSAGE_MAP()


// CMorphisChannelSwitchingDlg message handlers

BOOL CMorphisChannelSwitchingDlg::OnInitDialog()
   {
   CDialog::OnInitDialog();

   // Set the icon for this dialog.  The framework does this automatically
   // when the application's main window is not a dialog
   SetIcon(m_hIcon, TRUE);    // Set big icon
   SetIcon(m_hIcon, FALSE);   // Set small icon

   ShowWindow(SW_NORMAL);

   // Reset variables.
   m_MilApplication = m_MilSystem = m_MilDisplay = m_MilImageDisp = M_NULL;
   memset(m_MilImageDispChild, 0 , sizeof(m_MilImageDispChild));
   memset(m_ThreadHandle, 0 , sizeof(m_ThreadHandle));
   MIL_INT i;
   for (i = 0; i < 16; i++)
      m_ChannelInfo[i].Init();

   for (i = 0; i < MAX_DIGITIZERS; i++)
      m_DigInfo[i].Init();

   OnBnClickedButtonStop();

   // Update acquisition statistics every second.
   SetTimer(0, 1000, NULL);

   return TRUE;  // return TRUE  unless you set the focus to a control
   }

void CMorphisChannelSwitchingDlg::OnCancel()
   {
   OnBnClickedButtonStop();
   CDialog::OnCancel();
   }

// This function is called when Start button is pressed.
// It enables the windows and calls the AllocateMILSystem function.
void CMorphisChannelSwitchingDlg::OnBnClickedButtonStart()
   {
   CString InputText;
   MIL_INT NbrOfChannels = 0;
   NumberOfChannels.GetWindowText(InputText);
   m_NumberOfChannels = _ttoi((LPCTSTR)InputText);

   // Set limits on number of channels
   m_NumberOfChannels = min(m_NumberOfChannels, 16);
   m_NumberOfChannels = max(m_NumberOfChannels, 1);

   InputText.Format(_T("%d"), (int)m_NumberOfChannels);
   NumberOfChannels.SetWindowText(InputText);
   m_SelectedChannel = -1;

   // Enable the Windows.
   SliderLockSensitivity.EnableWindow(TRUE);
   SliderGainLuma.EnableWindow(TRUE);
   SliderContrast.EnableWindow(TRUE);
   SliderBrightness.EnableWindow(TRUE);

   // Inquire if grabbing in Field or Frame mode.
   if(GetCheckedRadioButton(IDC_RADIO16_FIELD, IDC_RADIO17_FRAME) == IDC_RADIO17_FRAME)
      m_GrabFieldNum = 2;
   else
      m_GrabFieldNum = 1;

   // Enable window items.
   GetDlgItem(IDC_EDIT_OVERLAY)->EnableWindow(TRUE);
   GetDlgItem(IDC_COMBO_DIG_TO_USE)->EnableWindow(TRUE);
   GetDlgItem(IDC_CHECK_SET_ALL_CHANNELS)->EnableWindow(TRUE);
   GetDlgItem(IDC_RADIO18_ANY)->EnableWindow(TRUE);
   GetDlgItem(IDC_RADIO19_ODD)->EnableWindow(TRUE);
   GetDlgItem(IDC_RADIO20_EVEN)->EnableWindow(TRUE);
   GetDlgItem(IDC_CHECK_AUTO_GAIN)->EnableWindow(TRUE);
   GetDlgItem(IDC_CHANNEL_ENABLED)->EnableWindow(FALSE);

   GetDlgItem(IDC_EDIT_NBR_OF_CHANNELS)->EnableWindow(FALSE);
   GetDlgItem(IDC_BUTTON_START)->EnableWindow(FALSE);
   GetDlgItem(IDC_RADIO16_FIELD)->EnableWindow(FALSE);
   GetDlgItem(IDC_RADIO17_FRAME)->EnableWindow(FALSE);

   // Call sliders handles to set state.
   LRESULT Result;
   OnNMReleasedcaptureSliderLockSensitivity(NULL, &Result);
   OnNMReleasedcaptureSliderGainLuma(NULL,  &Result);
   OnNMReleasedcaptureSliderContrast(NULL,  &Result);
   OnNMReleasedcaptureSliderBrightness(NULL,  &Result);

   // Allocate Mil objects.
   AllocateMILSystem();
   }

// This function is called when Stop button is pressed.
// It disables the windows and calls the FreeMILSystem function.
void CMorphisChannelSwitchingDlg::OnBnClickedButtonStop()
   {
   // Free Mil objects.
   FreeMILSystem();

   CString InputText;
   NumberOfChannels.GetWindowText(InputText);
   m_NumberOfChannels = _ttoi((LPCTSTR)InputText);
   if(!m_NumberOfChannels)
      {
      m_NumberOfChannels = 4;
      NumberOfChannels.SetWindowText(_T("4"));
      }

   m_SelectedChannel = 0;
   m_GrabFieldNum = 1;

   // Initialize to default value.
   SliderLockSensitivity.SetRange(15, 255);
   SliderGainLuma.SetRange(1,255);
   SliderContrast.SetRange(1,255);
   SliderBrightness.SetRange(1,255);
   SliderLockSensitivity.SetPos(60);
   SliderGainLuma.SetPos(20);
   SliderContrast.SetPos(128);
   SliderBrightness.SetPos(128);

   GetDlgItem(IDC_STATIC_STAT_DIG0)->SetWindowText(_T("Digitizer 0:"));
   GetDlgItem(IDC_STATIC_STAT_DIG1)->SetWindowText(_T("Digitizer 1:"));
   GetDlgItem(IDC_STATIC_STAT_DIG2)->SetWindowText(_T("Digitizer 2:"));
   GetDlgItem(IDC_STATIC_STAT_DIG3)->SetWindowText(_T("Digitizer 3:"));
   GetDlgItem(IDC_STATIC_STAT_TOTAL)->SetWindowText(_T("Total:"));

   // Disable unused window items.
   ((CButton*)GetDlgItem(IDC_RADIO0))->SetCheck(BST_CHECKED);
   for(int i = 0; i < 16; i++)
      GetDlgItem(IDC_RADIO0 + i)->EnableWindow(FALSE);

   GetDlgItem(IDC_CHECK_SET_ALL_CHANNELS)->EnableWindow(FALSE);
   GetDlgItem(IDC_COMBO_DIG_TO_USE)->EnableWindow(FALSE);
   GetDlgItem(IDC_EDIT_OVERLAY)->EnableWindow(FALSE);
   GetDlgItem(IDC_RADIO18_ANY)->EnableWindow(FALSE);
   GetDlgItem(IDC_RADIO19_ODD)->EnableWindow(FALSE);
   GetDlgItem(IDC_RADIO20_EVEN)->EnableWindow(FALSE);
   GetDlgItem(IDC_CHECK_AUTO_GAIN)->EnableWindow(FALSE);
   GetDlgItem(IDC_CHANNEL_ENABLED)->EnableWindow(FALSE);
   GetDlgItem(IDC_EDIT_NBR_OF_CHANNELS)->EnableWindow(TRUE);
   GetDlgItem(IDC_BUTTON_START)->EnableWindow(TRUE);
   GetDlgItem(IDC_RADIO16_FIELD)->EnableWindow(TRUE);
   GetDlgItem(IDC_RADIO17_FRAME)->EnableWindow(TRUE);
   SetAllChannels.SetCheck(TRUE);
   SliderLockSensitivity.EnableWindow(FALSE);
   SliderGainLuma.EnableWindow(FALSE);
   SliderContrast.EnableWindow(FALSE);
   SliderBrightness.EnableWindow(FALSE);
   GetDlgItem(IDC_EDIT_OVERLAY)->EnableWindow(FALSE);
   CheckRadioButton(IDC_RADIO16_FIELD, IDC_RADIO17_FRAME, IDC_RADIO16_FIELD);
   CheckRadioButton(IDC_RADIO18_ANY, IDC_RADIO20_EVEN, IDC_RADIO18_ANY);
   ((CButton *)GetDlgItem(IDC_CHECK_AUTO_GAIN))->SetCheck(TRUE);
   ((CButton *)GetDlgItem(IDC_CHANNEL_ENABLED))->SetCheck(TRUE);

   // Reset variables.
   m_MilApplication = m_MilSystem = m_MilDisplay = m_MilImageDisp = M_NULL;
   memset(m_MilImageDispChild, 0 , sizeof(m_MilImageDispChild));
   memset(m_ThreadHandle, 0 , sizeof(m_ThreadHandle));
   for (int i = 0; i < 16; i++)
      m_ChannelInfo[i].Init();
   for (int i = 0; i < MAX_DIGITIZERS; i++)
      m_DigInfo[i].Init();
   }

// Allocates all MIL objects (application, system, display, digitizers).
// One thread per digitizer is created. The channel switching is executed
// on channel switching threads.
void CMorphisChannelSwitchingDlg::AllocateMILSystem()
   {
   MIL_INT i = 0;
   MIL_INT NbGrab  = 0;
   MIL_INT DigSizeBand = 1;
   MIL_INT DigSizeX = 0;
   MIL_INT DigSizeY = 0;
   MIL_INT DispNbrImageX = 0;
   MIL_INT DispNbrImageY = 0;
   MIL_INT x, y;

   DWORD   ThreadId[MAX_DIGITIZERS];
   MIL_INT NbrOfCamerasPresent = 0;
   MIL_INT NbrOfCamerasPerDig = 0;
   MIL_INT NbrOfDigitizers = 0;
   MIL_INT DigIndex = 0;

   // Allocate MIL application, system and display.
   MappAlloc(M_NULL, M_DEFAULT, &m_MilApplication);
   MsysAlloc(M_DEFAULT, M_SYSTEM_MORPHIS, M_DEFAULT, M_DEFAULT, &m_MilSystem);
   MsysControl(m_MilSystem, M_TIMEOUT, 1); // timeout to 1 sec

   MdispAlloc(m_MilSystem, M_DEFAULT, MIL_TEXT("M_DEFAULT"), M_DEFAULT, &m_MilDisplay);

   // Inquire and allocate the available digitizers.
   NbrOfDigitizers = MsysInquire(m_MilSystem, M_DIGITIZER_NUM, M_NULL);
   for(i = 0; i < NbrOfDigitizers; i++)
      {
      CString Combo;
      MdigAlloc(m_MilSystem, Device[i], MIL_TEXT("M_DEFAULT"), M_DEFAULT,
                &m_DigInfo[i].MilDigitizer);
      MdigControl(m_DigInfo[i].MilDigitizer, M_GRAB_FIELD_NUM,  m_GrabFieldNum);
      MdigControl(m_DigInfo[i].MilDigitizer, M_GRAB_TIMEOUT, 1000); // timeout to 1 sec.

      Combo.Format(MIL_TEXT("%d"), (int)i);
      ComboDigitizers.AddString(Combo);

      m_DigInfo[i].MilSystem = m_MilSystem;
      m_DigInfo[i].DeviceNumber = i;
      m_DigInfo[i].pMorphisChannelSwitchingDlg = this;
      }
   ComboDigitizers.SetCurSel(0);

   // Inquire digitizer sizes. Used later from grab buffers and display buffers.
   DigSizeBand = MdigInquire(m_DigInfo[0].MilDigitizer, M_SIZE_BAND, M_NULL);
   DigSizeX    = MdigInquire(m_DigInfo[0].MilDigitizer, M_SIZE_X, M_NULL);
   DigSizeY    = MdigInquire(m_DigInfo[0].MilDigitizer, M_SIZE_Y, M_NULL);
   if(m_GrabFieldNum == 1)
      {
      DigSizeX /= 2;
      DigSizeY /= 2;
      }

   // Allocate the display size according to the number of cameras.
   switch(m_NumberOfChannels)
      {
      case 1:
         DispNbrImageX = 1; DispNbrImageY = 1; break;
      case 2:
         DispNbrImageX = 2; DispNbrImageY = 1; break;
      case 3:
      case 4:
         DispNbrImageX = 2; DispNbrImageY = 2; break;
      case 5:
      case 6:
         DispNbrImageX = 3; DispNbrImageY = 2; break;
      case 7:
      case 8:
      case 9:
         DispNbrImageX = 3; DispNbrImageY = 3; break;

      case 10:
      case 11:
      case 12:
         DispNbrImageX = 4; DispNbrImageY = 3; break;
      default:
         DispNbrImageX = 4; DispNbrImageY = 4; break;
      }

   // Disable error printing.
   MappControl(M_DEFAULT, M_ERROR, M_PRINT_DISABLE);

   // Allocate a display buffer, clear it and display it.
   MbufAllocColor(m_MilSystem,
                  DigSizeBand,
                  (MIL_INT) (DigSizeX * DispNbrImageX),
                  (MIL_INT) (DigSizeY * DispNbrImageY),
                  8L+M_UNSIGNED,
                  M_IMAGE + M_DISP + M_VIDEO_MEMORY + (DigSizeBand==3? M_YUV16 + M_PACKED: 0),
                  &m_MilImageDisp);

   // If cannot allocate in video memory, use host memory.
   if(m_MilImageDisp == 0)
      MbufAllocColor(m_MilSystem,
                     DigSizeBand,
                     (MIL_INT) (DigSizeX * DispNbrImageX),
                     (MIL_INT) (DigSizeY * DispNbrImageY),
                     8L+M_UNSIGNED,
                     M_IMAGE + M_DISP + (DigSizeBand == 3? M_YUV16 + M_PACKED: 0),
                     &m_MilImageDisp);

   MbufClear(m_MilImageDisp, 128L);
   MdispSelect(m_MilDisplay, m_MilImageDisp);

   // Enable error printing.
   MappControl(M_DEFAULT, M_ERROR, M_PRINT_ENABLE);

   // Allocate one child buffer for each input channel in the display buffer.
   i = 0;
   for(y = 0; y < DispNbrImageY; y++)
      {
      for(x = 0; x < DispNbrImageX; x++)
         {
         MbufChild2d(m_MilImageDisp,
                     (MIL_INT)(DigSizeX  * x),
                     (MIL_INT)(DigSizeY  * y),
                     (MIL_INT)(DigSizeX ),
                     (MIL_INT)(DigSizeY ),
                     &m_MilImageDispChild[i++]);
         }
      }

   // Distribute the available cameras to each available digitizer.
   NbrOfCamerasPerDig = (MIL_INT)((m_NumberOfChannels / (double)NbrOfDigitizers) + 0.5);
   DigIndex = -1;
   for(i = 0; i < m_NumberOfChannels; i++)
      {
      MIL_ID MilGrabBuffer;
      MbufAllocColor(m_MilSystem, DigSizeBand, DigSizeX, DigSizeY,
                     8L+M_UNSIGNED,
                     M_IMAGE + M_GRAB + (DigSizeBand == 3? M_YUV16 + M_PACKED:0),
                     &MilGrabBuffer);
      MbufClear(MilGrabBuffer, 0);

   if(NbrOfCamerasPerDig)
      {
      if((i % NbrOfCamerasPerDig) == 0)
         DigIndex++;
      }
   else
      DigIndex =0;

      m_ChannelInfo[i].pDigInfo = &m_DigInfo[DigIndex%NbrOfDigitizers];
      m_ChannelInfo[i].Index = i;
      m_ChannelInfo[i].MilChannel   = Channel[i];
      m_ChannelInfo[i].MilImageDisp = m_MilImageDispChild[i];
      m_ChannelInfo[i].MilGrabBuffer = MilGrabBuffer;
      m_ChannelInfo[i].OverlayText.Format(_T("Camera %d"), (int)i);
      }

   // Start the ChannelSwitchingThread for each digitizer.
   m_Exit = 0;
   for(i = 0; i < NbrOfDigitizers; i++)
      {
      m_ThreadHandle[i]=(HANDLE) CreateThread(NULL, 0L,
                                              (LPTHREAD_START_ROUTINE) &ChannelSwitchingThread,
                                              &m_DigInfo[i],  0L, &(ThreadId[i]));
      // wait a little to let time for the digitizer to start.
      MosSleep(500);
      }

   return;
   }

// Frees all MIL objects and closes the Channel switching thread.
void CMorphisChannelSwitchingDlg::FreeMILSystem()
   {
   MIL_INT i;

   // Stop channel switching thread.
   m_Exit = true;
   for(i = 0; i < MAX_DIGITIZERS; i++)
      if(m_ThreadHandle[i])
         {
         EnterCriticalSection(&Lock);
         WaitForSingleObject(m_ThreadHandle[i], INFINITE);
         LeaveCriticalSection(&Lock);
         }
   
   //close the threads' handle
   for(i = 0; i < MAX_DIGITIZERS; i++)
      {
         CloseHandle(m_ThreadHandle[i]);
      }

   for (i = 0; i < 16; i++)
      {
      m_ChannelInfo[i].pDigInfo = NULL;
      if(m_ChannelInfo[i].MilGrabBuffer)
         MbufFree(m_ChannelInfo[i].MilGrabBuffer);
      if(m_MilImageDispChild[i])
         MbufFree(m_MilImageDispChild[i]);
      }

   if(m_MilImageDisp)
      MbufFree(m_MilImageDisp);

   for(i = 0; i < MAX_DIGITIZERS; i++)
      if(m_DigInfo[i].MilDigitizer)
         {
         MdigFree(m_DigInfo[i].MilDigitizer);
         }

   if(m_MilDisplay)
      MdispFree(m_MilDisplay);

   if(m_MilSystem)
      MsysFree(m_MilSystem);

   if(m_MilApplication)
      MappFree(m_MilApplication);

   // Reset variables.
   m_MilApplication = m_MilSystem = m_MilDisplay = m_MilImageDisp = M_NULL;
   memset(m_MilImageDispChild, 0 , sizeof(m_MilImageDispChild));
   memset(m_ThreadHandle, 0 , sizeof(m_ThreadHandle));

   for (i = 0; i < 16; i++)
      m_ChannelInfo[i].Init();

   for (i = 0; i < MAX_DIGITIZERS; i++)
      m_DigInfo[i].Init();

   // Remove all content in digitizer combo box.
   ComboDigitizers.ResetContent();

   return;
   }

// ChannelSwitchingThread
// This function perform the channel switching.
// It scans all channels and grabs on the ones that matches the  digitizer.
unsigned int CMorphisChannelSwitchingDlg::ChannelSwitchingThread(void *TParam)
   {
   DIG_DEVICE_INFO *pDigInfo = (DIG_DEVICE_INFO *)TParam;
   CMorphisChannelSwitchingDlg *pThis =
      (CMorphisChannelSwitchingDlg *) pDigInfo->pMorphisChannelSwitchingDlg;

   CHANNEL_INFO *pChannelInfo = pThis->m_ChannelInfo;
   CHANNEL_INFO *pCurChannel  = NULL;
   CHANNEL_INFO *pPrevChannel = NULL;
   MIL_ID  MilDigitizer       = pDigInfo->MilDigitizer;
   MIL_ID  MilSystem          = pDigInfo->MilSystem;
   MIL_INT Index              = 0;
   MIL_INT i = 0;
   double TimeStart, TimeStartFrameRate, TimeEnd;
   MIL_INT CameraCount = 0;
   double CurrentTime;

   MIL_INT LockSensitivity = -1;
   MIL_INT MilChannel = -1;
   MIL_INT GrabStartMode = -1;
   MIL_INT GrabAutoGain = -1;
   MIL_INT GainLuma = -1;
   MIL_INT Contrast = -1;
   MIL_INT Brightness = -1;



   MdigControl(MilDigitizer, M_CAMERA_LOCK, M_DISABLE);

   // Set the acquisition in synchronous mode.
   MdigControl(MilDigitizer, M_GRAB_MODE, M_ASYNCHRONOUS_QUEUED);

   if(pThis->m_GrabFieldNum == 1)
      MdigControl(MilDigitizer, M_GRAB_SCALE_X, 0.5);

   // Disable error printing.
   MappControl(M_DEFAULT, M_ERROR, M_PRINT_DISABLE);

   // Boost the thread priority to increase channel switching rate.
   SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_HIGHEST);

   MappTimer(M_DEFAULT, M_TIMER_READ, &TimeStart);
   TimeStartFrameRate = TimeStart;

   Index = 0;
   i = 0;
   while(!pThis->m_Exit)
      {
      MappTimer(M_DEFAULT, M_TIMER_READ, &CurrentTime);

      // Find a camera that matches this digitizer.
      if(Index < (pThis->m_NumberOfChannels-1))
         Index++;
      else
         {
         Index = 0;

         // If no cameras are assigned on this digitizer, wait a little.
         if(CameraCount == 0)
            {
            GainLuma = Contrast = Brightness = -1;
            MilChannel = LockSensitivity = GrabStartMode = GrabAutoGain = -1;
            pDigInfo->AvgFrameRate = 0;
            pDigInfo->AvgFrameRateCurrent = 0;
            MosSleep(1000);
            }
         CameraCount = 0;
         }

      if(pChannelInfo[Index].pDigInfo == NULL)
         continue;

      // Does channel matches the digitizer of this thread?
      if(MilDigitizer != pChannelInfo[Index].pDigInfo->MilDigitizer)
         continue;

      // Get current channel and grab.
      pCurChannel = &pChannelInfo[Index];

      // On first grab set starting channel.
      if(pDigInfo->NbrFramesGrabbed == 0)
         {
         pPrevChannel = pCurChannel = &pChannelInfo[Index];
         MdigControl(MilDigitizer, M_CHANNEL, pPrevChannel->MilChannel);
         }

      if(pCurChannel->IsEnabled)
         {
         CameraCount++;

         // Start asynchronous grab previously set channel.
         MdigGrab(MilDigitizer, pPrevChannel->MilGrabBuffer);

         // While the grab is in progress, queue the channel switch for the next grab.
         // This command is asynchronous.
         if(LockSensitivity != pCurChannel->LockSensitivity)
            MdigControl(MilDigitizer, M_CAMERA_LOCK_SENSITIVITY, pCurChannel->LockSensitivity);

         if(MilChannel != pCurChannel->MilChannel)
            MdigControl(MilDigitizer, M_CHANNEL, pCurChannel->MilChannel);

         // Set the controls for this channel. These controls will take effect once the current
         // queued grab is done.
         if(GrabStartMode != pCurChannel->GrabStartMode)
            MdigControl(MilDigitizer, M_GRAB_START_MODE, pCurChannel->GrabStartMode);

         if(pCurChannel->UseAutomaticInputGain)
            {
            if(GrabAutoGain != M_ENABLE)
               MdigControl(MilDigitizer, M_GRAB_AUTOMATIC_INPUT_GAIN, M_ENABLE);
            }
         else
            {
            if(GrabAutoGain != M_DISABLE)
               MdigControl(MilDigitizer, M_GRAB_AUTOMATIC_INPUT_GAIN, M_DISABLE);

            if(GainLuma != pCurChannel->GainLuma)
               MdigControl(MilDigitizer, M_GRAB_INPUT_GAIN, pCurChannel->GainLuma);
            }

         if(Contrast != pCurChannel->Contrast)
            MdigControl(MilDigitizer, M_CONTRAST_REF, (double)pCurChannel->Contrast);
         if(Brightness != pCurChannel->Brightness)
            MdigControl(MilDigitizer, M_BRIGHTNESS_REF, (double)pCurChannel->Brightness);

         // Keep current settings on this digitizer. Reprogram them only if they change.
         LockSensitivity   = pCurChannel->LockSensitivity;
         MilChannel        = pCurChannel->MilChannel;
         GrabAutoGain      = pCurChannel->UseAutomaticInputGain? M_ENABLE: M_DISABLE;
         GrabStartMode     = pCurChannel->GrabStartMode;
         GainLuma          = pCurChannel->GainLuma;
         Contrast          = pCurChannel->Contrast;
         Brightness        = pCurChannel->Brightness;

         // This is a good place to do other processing on the grabbed buffer.
         // Set the overlay and update the display.
         MgraText(M_DEFAULT, pPrevChannel->MilGrabBuffer, 10, 10,
                  pPrevChannel->OverlayText.GetBuffer());
         MbufCopy(pPrevChannel->MilGrabBuffer, pPrevChannel->MilImageDisp);

         // Wait for the current grab to complete before continuing.
         MdigGrabWait(MilDigitizer, M_GRAB_END);

         // If we have a grab error  Disable the channel if the camera is not present anymore.
         if(MappGetError(M_DEFAULT, M_CURRENT, M_NULL))
            {
            MdigControl(MilDigitizer, M_CHANNEL, pPrevChannel->MilChannel);
            MosSleep(CAMERA_PRESENT_TIME_IN_MS);
            if(MdigInquire(MilDigitizer, M_CAMERA_PRESENT, M_NULL) == 0)
               {
               // Disable channel if camera is not present.
               MIL_INT Channel = pPrevChannel->Index;
               CString ChannelText;
               ChannelText.Format(_T("~Camera %d"), (int)Channel);
                if(!pThis->m_Exit)
                  {
                  EnterCriticalSection(&pThis->Lock);
                  pThis->GetDlgItem(IDC_RADIO0 + (int)Channel)->SetWindowText(ChannelText);
                  LeaveCriticalSection(&pThis->Lock);
                  }
               pPrevChannel->IsEnabled = false;
               }

            MdigControl(MilDigitizer, M_CHANNEL, pCurChannel->MilChannel);
            MosSleep(CAMERA_PRESENT_TIME_IN_MS);

            GainLuma = Contrast = Brightness = -1;
            MilChannel = LockSensitivity = GrabStartMode = GrabAutoGain = -1;
            }

         // Update statistics.
         pDigInfo->NbrFramesGrabbed++;
         if((pDigInfo->NbrFramesGrabbed % 50) == 0)
            {
            MappTimer(M_DEFAULT, M_TIMER_READ, &TimeEnd);
            pDigInfo->AvgFrameRateCurrent = 50.0/(TimeEnd-TimeStartFrameRate);
            pDigInfo->AvgFrameRate = pDigInfo->NbrFramesGrabbed/((TimeEnd- TimeStart));
            MappTimer(M_DEFAULT, M_TIMER_READ, &TimeStartFrameRate);
            }
         pPrevChannel = pCurChannel;
         }
      }
   return 0;
   }

// "Number of lines to wait for lock" slider.
void CMorphisChannelSwitchingDlg::OnNMReleasedcaptureSliderLockSensitivity(NMHDR *pNMHDR,
                                                                           LRESULT *pResult)
   {
   CString Text;
   Text.Format(MIL_TEXT("%d"),(int)SliderLockSensitivity.GetPos());
   LockSensitivity.SetWindowText(Text);
   UpdateChannelSettings();
   *pResult = 0;
   }

// Manuel gain slider.
void CMorphisChannelSwitchingDlg::OnNMReleasedcaptureSliderGainLuma(NMHDR *pNMHDR,
                                                                    LRESULT *pResult)
   {
   CString Text;
   Text.Format(MIL_TEXT("%d"),(int)SliderGainLuma.GetPos());
   GainLuma.SetWindowText(Text);
   UpdateChannelSettings();
   *pResult = 0;
   }

// Contrast slider.
void CMorphisChannelSwitchingDlg::OnNMReleasedcaptureSliderContrast(NMHDR *pNMHDR,
                                                                    LRESULT *pResult)
   {
   CString Text;
   Text.Format(MIL_TEXT("%d"),(int)SliderContrast.GetPos());
   Contrast.SetWindowText(Text);
   UpdateChannelSettings();
   *pResult = 0;
   }

// Brightness slider.
void CMorphisChannelSwitchingDlg::OnNMReleasedcaptureSliderBrightness(NMHDR *pNMHDR,
                                                                      LRESULT *pResult)
   {
   CString Text;
   Text.Format(MIL_TEXT("%d"),(int)SliderBrightness.GetPos());
   Brightness.SetWindowText(Text);
   UpdateChannelSettings();
   *pResult = 0;
   }

// Overlay text edit box.
void CMorphisChannelSwitchingDlg::OnEnChangeEditOverlay()
   {
   CString InputText;
   GetDlgItem(IDC_EDIT_OVERLAY)->GetWindowText(InputText);

   if(m_SelectedChannel != -1)
      m_ChannelInfo[m_SelectedChannel].OverlayText = InputText;
   else
      for(MIL_INT i = 0; i < m_NumberOfChannels; i++)
         m_ChannelInfo[i].OverlayText = InputText;
   }

// Automatic gain check box.
void CMorphisChannelSwitchingDlg::OnBnClickedCheckAutoGain()
   {
   UpdateChannelSettings();
   }

// "Enable acquisition" check box.
void CMorphisChannelSwitchingDlg::OnBnClickedChannelEnabled()
   {
   if(m_SelectedChannel != -1)
      {
      m_ChannelInfo[m_SelectedChannel].IsEnabled  =
         (((CButton *)GetDlgItem(IDC_CHANNEL_ENABLED))->GetCheck())? true: false;
      }

   UpdateChannelSettings();
   }

// Toggle between all channels and single channel check box.
void CMorphisChannelSwitchingDlg::OnBnClickedCheckSetAllChannels()
   {
   CString ChannelSettingsText;

   MIL_INT EnableChannels = !(SetAllChannels.GetCheck());
   if(EnableChannels)
      {
      m_SelectedChannel = GetCheckedRadioButton(IDC_RADIO0, IDC_RADIO15) - IDC_RADIO0;
      m_SelectedChannel = max(m_SelectedChannel, 0);
      ChannelSettingsText.Format(_T("Camera %d acquisition settings"), (int)m_SelectedChannel);

      // Set Grab Start Mode.
      switch(m_ChannelInfo[m_SelectedChannel].GrabStartMode)
         {
      case M_FIELD_START_ODD:
         CheckRadioButton(IDC_RADIO18_ANY, IDC_RADIO20_EVEN, IDC_RADIO19_ODD);
         break;
      case M_FIELD_START_EVEN:
         CheckRadioButton(IDC_RADIO18_ANY, IDC_RADIO20_EVEN, IDC_RADIO20_EVEN);
         break;
      default:
      case M_FIELD_START:
         CheckRadioButton(IDC_RADIO18_ANY, IDC_RADIO20_EVEN, IDC_RADIO18_ANY);
         break;
         }

      // Set digitizer device number.
      ComboDigitizers.SetCurSel((int)m_ChannelInfo[m_SelectedChannel].pDigInfo->DeviceNumber);

      // Enable "Enable camera" check box.
      GetDlgItem(IDC_CHANNEL_ENABLED)->EnableWindow(TRUE);

      // Set overlay text.
      GetDlgItem(IDC_EDIT_OVERLAY)->SetWindowText(
         m_ChannelInfo[m_SelectedChannel].OverlayText);

      // Set sliders.
      UpdateDisplayChannelSettings(m_SelectedChannel);
      }
   else
      {
      m_SelectedChannel = -1;
      ChannelSettingsText.Format(_T("Set all camera acquisition settings"));

      // Disable "Enable camera" check box.
      GetDlgItem(IDC_CHANNEL_ENABLED)->EnableWindow(FALSE);
      }

   GetDlgItem(IDC_STATIC_CHANNEL_SETTINGS)->SetWindowText(ChannelSettingsText);

   UpdateChannelSettings();

   for(int i = 0; i < 16; i++)
      {
      if(i < m_NumberOfChannels)
         GetDlgItem(IDC_RADIO0 + i)->EnableWindow((BOOL)EnableChannels);
      else
         GetDlgItem(IDC_RADIO0 + i)->EnableWindow(FALSE);
      }
   }

// Camera (0 to 15) radio button.
void CMorphisChannelSwitchingDlg::OnBnClickedRadio()
   {
   OnBnClickedCheckSetAllChannels();
   }

// Acquisition start mode radio button.
void CMorphisChannelSwitchingDlg::OnBnClickedRadioField()
   {
   MIL_INT GrabStartModeRadioButton = 0;
   MIL_INT MilGrabStartMode = 0;
   GrabStartModeRadioButton = GetCheckedRadioButton(IDC_RADIO18_ANY, IDC_RADIO20_EVEN);

   if(GrabStartModeRadioButton == IDC_RADIO20_EVEN)
      MilGrabStartMode = M_FIELD_START_EVEN;
   else if(GrabStartModeRadioButton == IDC_RADIO19_ODD)
      MilGrabStartMode = M_FIELD_START_ODD;
   else
      MilGrabStartMode = M_FIELD_START;

   if(m_SelectedChannel == -1)
      {
      for(MIL_INT i = 0; i < m_NumberOfChannels; i++)
         {
         m_ChannelInfo[i].GrabStartMode = MilGrabStartMode;
         }
      }
   else
      {
      m_ChannelInfo[m_SelectedChannel].GrabStartMode = MilGrabStartMode;
      }
   }

// Combo box to select the digitizer to be used on the selected camera.
void CMorphisChannelSwitchingDlg::OnCbnSelchangeComboDigToUse()
   {
   // Select Digitizer.
   MIL_INT CurSel = ComboDigitizers.GetCurSel();

   if(m_SelectedChannel == -1)
      {
      for(MIL_INT i = 0; i < 16; i++)
         {
         // Select Digitizer for all channels.
         m_ChannelInfo[i].pDigInfo = &m_DigInfo[CurSel];
         }
      }
   else
      {
      // Select Digitizer for selected channel.
      m_ChannelInfo[m_SelectedChannel].pDigInfo = &m_DigInfo[CurSel];
      }
   }

// Inquires the state of the dialog controls and sets them to the structure of the
// selected channel or channels.
void CMorphisChannelSwitchingDlg::UpdateChannelSettings()
   {
   if(m_SelectedChannel == -1)
      {
      for(MIL_INT i = 0; i < m_NumberOfChannels; i++)
         {
         UpdateChannelSettings(i);
         }
      }
   else
      {
      UpdateChannelSettings(m_SelectedChannel);
      }
   }

// Inquires the state of the dialog controls and sets them to the structure of the
// specified channel.
void CMorphisChannelSwitchingDlg::UpdateChannelSettings(MIL_INT Channel)
   {
   MIL_INT GrabStartModeRadioButton = 0;
   MIL_INT MilGrabStartMode = 0;
   GrabStartModeRadioButton = GetCheckedRadioButton(IDC_RADIO18_ANY, IDC_RADIO20_EVEN);

   // Update GrabStartMode.
   if(GrabStartModeRadioButton == IDC_RADIO20_EVEN)
      MilGrabStartMode = M_FIELD_START_EVEN;
   else if(GrabStartModeRadioButton == IDC_RADIO19_ODD)
      MilGrabStartMode = M_FIELD_START_ODD;
   else
      MilGrabStartMode = M_FIELD_START;

   m_ChannelInfo[Channel].GrabStartMode = MilGrabStartMode;

   // Update lock sensitivity.
   m_ChannelInfo[Channel].LockSensitivity = SliderLockSensitivity.GetPos();

   // Update GainLuma.
   m_ChannelInfo[Channel].GainLuma = SliderGainLuma.GetPos();

   // Update Contrast.
   m_ChannelInfo[Channel].Contrast = SliderContrast.GetPos();

   // Update Brightness.
   m_ChannelInfo[Channel].Brightness = SliderBrightness.GetPos();

   // Update automatic input gain.
   m_ChannelInfo[Channel].UseAutomaticInputGain  =
      (((CButton *)GetDlgItem(IDC_CHECK_AUTO_GAIN))->GetCheck())? true: false;

   // Update channel enabled.
   CString ChannelText;
   if(m_ChannelInfo[Channel].IsEnabled)
      ChannelText.Format(_T("Camera %d"), (int)Channel);
   else
      ChannelText.Format(_T("~Camera %d"), (int)Channel);

   GetDlgItem(IDC_RADIO0 + (int)Channel)->SetWindowText(ChannelText);

   // Disable Gain slider when using automatic input gain.
   if(m_ChannelInfo[Channel].UseAutomaticInputGain == false)
      SliderGainLuma.EnableWindow(TRUE);
   else
      SliderGainLuma.EnableWindow(FALSE);
   }

// Set the dialog controls to the state of the channel structure.
void CMorphisChannelSwitchingDlg::UpdateDisplayChannelSettings(MIL_INT Channel)
   {
   // Update selected digitizer.
   ComboDigitizers.SetCurSel((int)m_ChannelInfo[Channel].pDigInfo->DeviceNumber);

   // Update lock sensitivity.
   SliderLockSensitivity.SetPos((int)m_ChannelInfo[Channel].LockSensitivity);

   // Update GainLuma.
   SliderGainLuma.SetPos((int)m_ChannelInfo[Channel].GainLuma);

   // Update Contrast.
   SliderContrast.SetPos((int)m_ChannelInfo[Channel].Contrast);

   // Update Brightness.
   SliderBrightness.SetPos((int)m_ChannelInfo[Channel].Brightness);

   // Update Automatic input gain.
   ((CButton *)GetDlgItem(IDC_CHECK_AUTO_GAIN))->SetCheck(
      m_ChannelInfo[Channel].UseAutomaticInputGain);

   // Update Enable camera.
   ((CButton *)GetDlgItem(IDC_CHANNEL_ENABLED))->SetCheck(m_ChannelInfo[Channel].IsEnabled);

   // Disable Gain slider when using automatic input gain.
   if(m_ChannelInfo[Channel].UseAutomaticInputGain == false)
      SliderGainLuma.EnableWindow(TRUE);
   else
      SliderGainLuma.EnableWindow(FALSE);

   LRESULT Result;
   OnNMReleasedcaptureSliderLockSensitivity(NULL, &Result);
   OnNMReleasedcaptureSliderGainLuma(NULL,  &Result);
   OnNMReleasedcaptureSliderContrast(NULL,  &Result);
   OnNMReleasedcaptureSliderBrightness(NULL,  &Result);
   }


// Updates the acquisition statistics at the specified interval.
void CMorphisChannelSwitchingDlg::OnTimer(UINT_PTR nIDEvent)
   {
   MIL_INT NumberOfCameras[MAX_DIGITIZERS] = {0};
   MIL_INT TotalFrames = 0;
   MIL_INT TotalCameras = 0;
   double AvrFrameRate = 0;
   double AvrFrameRateCurrent = 0;
   int   i;
   CString Statistics;

   for(i = 0; i < m_NumberOfChannels; i++)
      {
      if(m_ChannelInfo[i].pDigInfo && m_ChannelInfo[i].IsEnabled)
         {
         MIL_INT Index = m_ChannelInfo[i].pDigInfo->DeviceNumber;
         NumberOfCameras[Index]++;
         }
      }

   for(i = 0; i < 4; i++)
      {
      TotalCameras += NumberOfCameras[i];
      TotalFrames += m_DigInfo[i].NbrFramesGrabbed;
      AvrFrameRate += m_DigInfo[i].AvgFrameRate;
      AvrFrameRateCurrent +=  m_DigInfo[i].AvgFrameRateCurrent;

      Statistics.Format(_T("Digitizer %1.1d:\t%9.4d frames,  \t%d cameras,\t%5.2f fps ")
                        _T("(current)\t%5.2f fps"),
                        i, m_DigInfo[i].NbrFramesGrabbed, NumberOfCameras[i],
                        m_DigInfo[i].AvgFrameRateCurrent, m_DigInfo[i].AvgFrameRate);
      GetDlgItem(IDC_STATIC_STAT_DIG0+i)->SetWindowText(Statistics);
      if(m_DigInfo[i].MilDigitizer)
         GetDlgItem(IDC_STATIC_STAT_DIG0+i)->EnableWindow(TRUE);
      else
         GetDlgItem(IDC_STATIC_STAT_DIG0+i)->EnableWindow(FALSE);
      }

   Statistics.Format(_T("Total:      \t%9.4lld frames,  \t%d cameras,\t%5.2f fps ")
                     _T("(current)\t%5.2f fps"),
                     (long long)TotalFrames, (int)TotalCameras, AvrFrameRateCurrent, AvrFrameRate);
   GetDlgItem(IDC_STATIC_STAT_TOTAL)->SetWindowText(Statistics);
   }

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.
void CMorphisChannelSwitchingDlg::OnPaint()
   {
   if (IsIconic())
      {
      CPaintDC dc(this); // device context for painting

      SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

      // Center icon in client rectangle
      int cxIcon = GetSystemMetrics(SM_CXICON);
      int cyIcon = GetSystemMetrics(SM_CYICON);
      CRect rect;
      GetClientRect(&rect);
      int x = (rect.Width() - cxIcon + 1) / 2;
      int y = (rect.Height() - cyIcon + 1) / 2;

      // Draw the icon
      dc.DrawIcon(x, y, m_hIcon);
      }
   else
      {
      CDialog::OnPaint();
      }
   }

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CMorphisChannelSwitchingDlg::OnQueryDragIcon()
   {
   return static_cast<HCURSOR>(m_hIcon);
   }

