#include "stdafx.h"

#include "MainWindow.hpp"

#include "Resource.h"
#include "versioninfo.h"
#include "Modules/ISessionModule.h"
#include "Modules/ILoginModule.h"
#include "Modules/ISysConfigModule.h"
#include "Modules/IMessageModule.h"
#include "Modules/IMiscModule.h"
#include "Modules/IUserListModule.h"
#include "Modules/ITcpClientModule.h"
#include "Modules/UIEventManager.h"
#include "Modules/IScreenCaptureModule.h"
#include "Modules/ITcpClientModule.h"
#include "Modules/ILoginModule.h"
#include "Modules/IMiscModule.h"
#include "Modules/IUserListModule.h"
#include "Modules/ISessionModule.h"
#include "Modules/ISysConfigModule.h"
#include "Modules/IFileTransferModule.h"
#include "utility/Multilingual.h"
#include "ProtocolBuffer/IM.BaseDefine.pb.h"
#include "Modules/UI/SessionLayout.h"
#include "DuiLib/Layout/UITabLayout.h"
//#include "Modules/UI/SessionLayout.h"
//#include "../Modules/Session/UI/UIIMEdit.h"


DUI_BEGIN_MESSAGE_MAP(MainWindow, WindowImplBase)
DUI_ON_MSGTYPE(DUI_MSGTYPE_WINDOWINIT, OnWindowInitialized)
DUI_ON_MSGTYPE(DUI_MSGTYPE_CLICK, OnClick)
DUI_ON_MSGTYPE(DUI_MSGTYPE_TEXTCHANGED, OnTextChanged)
DUI_END_MESSAGE_MAP()

namespace{
    DWORD WINAPI  WindowShake(LPVOID lpParam)//窗口抖动
    {
        HWND hwnd = (HWND)lpParam;
        RECT rect;
        memset(&rect, 0, sizeof(rect));
        GetWindowRect(hwnd, &rect);
        for (int i = 0; i < 10; i++)
        {
            MoveWindow(hwnd, rect.left - 6, rect.top, rect.right - rect.left, rect.bottom - rect.top, true);
            Sleep(30);
            MoveWindow(hwnd, rect.left, rect.top - 5, rect.right - rect.left, rect.bottom - rect.top, true);
            Sleep(30);
            MoveWindow(hwnd, rect.left + 5, rect.top, rect.right - rect.left, rect.bottom - rect.top, true);
            Sleep(30);
            MoveWindow(hwnd, rect.left, rect.top + 5, rect.right - rect.left, rect.bottom - rect.top, true);
            Sleep(30);
            MoveWindow(hwnd, rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top, true);
        }
        return 0;
    }
}
/******************************************************************************/

// -----------------------------------------------------------------------------
//  MainWindow: Public, Constructor

MainWindow::MainWindow()
:m_pbtnSysConfig(nullptr)
, m_pbtnMyFace(nullptr)
, m_bInstalled(false)
, m_bHidden(false)
, m_pbtnClose(nullptr)
, m_pbtnMinMize(nullptr)
{
}

// -----------------------------------------------------------------------------
//  MainWindow: Public, Destructor

MainWindow::~MainWindow()
{
    module::getLoginModule()->removeObserver(this);
    module::getUserListModule()->removeObserver(this);
    module::getSessionModule()->removeObserver(this);
    RemoveIcon();
}

LPCTSTR MainWindow::GetWindowClassName() const
{
#ifdef _DEBUG
    return _T("TeamTalkMainWindow_debug");
#else
    return _T("TeamTalkMainWindow");
#endif

}

DuiLib::CDuiString MainWindow::GetSkinFile()
{
    return  _T("MainWindow\\MainWindow.xml");
}

DuiLib::CDuiString MainWindow::GetSkinFolder()
{
    return _T("");
}

CControlUI* MainWindow::CreateControl(LPCTSTR pstrClass)
{
    return module::getSessionModule()->createMainDialogControl(pstrClass, m_PaintManager);
    
}
void MainWindow::OnFinalMessage(HWND hWnd)
{
    WindowImplBase::OnFinalMessage(hWnd);
}

void MainWindow::OnHotkey(__in WPARAM wParam, __in LPARAM lParam)
{
    module::ScreenCaptureHotkeyId emHotkeyId = module::getScreenCaptureModule()->shouldHandle(lParam);
    if (module::SC_HK_START_CAPTURE == emHotkeyId)
    {
        //ctrl + shift + A

        SYSTEMTIME tm = { 0 };
        GetLocalTime(&tm);
        CString strFileName;
        strFileName.Format(_T("%4d%02d%02d%02d%02d%02d.bmp"), tm.wYear, tm.wMonth, tm.wDay, tm.wHour, tm.wMinute, tm.wSecond);
        CString strFilePath = module::getMiscModule()->getUserTempDir() + strFileName;
        module::getScreenCaptureModule()->startCapture(strFilePath.GetBuffer(), FALSE);
    }
    else if (module::SC_HK_ESCAPE == emHotkeyId)
    {
        module::getScreenCaptureModule()->cancelCapture();
    }
}

LRESULT MainWindow::HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    if (WM_HOTKEY == uMsg)
    {
        OnHotkey(wParam, lParam);
        return 0;
    }
    else if (WM_TRAYICON_NOTIFY == uMsg)
    {
        OnTrayIconNotify(wParam, lParam);
        return 0;
    }
    else if (WM_MENU_NOTIFY == uMsg)
    {
        LPCTSTR pName = (LPCTSTR)wParam;
        LPCTSTR pSid = (LPCTSTR)lParam;
        if (pName)
        {
            //LOG__(APP, _T("WM_MENU_NOTIFY:%s"),pName);
            OnMenuClicked(CString(pName), CString(pSid));
        }
        return 0;
    }
    else if (WM_TIMER == uMsg)
    {
        if (wParam == TIMER_TRAYEMOT)
        {
            static BOOL bTrans = FALSE;
            if (bTrans)
            {
                bTrans = FALSE;
                SetTrayIconIndex(ICON_TRAY_LOGO);
            }
            else
            {
                bTrans = TRUE;
                SetTrayIconIndex(-1);
            }
        }
    }
    else if (WM_ENDSESSION == uMsg)
    {
        BOOL bEnding = wParam;
        if (!bEnding)
            return 0;
        module::getMiscModule()->quitTheApplication();
        LOG__(APP, _T("MainWindow: WM_ENDSESSION System End Session OK"));
    }
    else if (WM_COPYDATA == uMsg)
    {
        COPYDATASTRUCT *pCopyData = (COPYDATASTRUCT*)lParam;
        OnCopyData(pCopyData);
    }

    return WindowImplBase::HandleMessage(uMsg, wParam, lParam);
}

void MainWindow::OnWindowInitialized(TNotifyUI& msg)
{
    module::getScreenCaptureModule()->initCapture(m_hWnd);
    module::getLoginModule()->addObserver(this, BIND_CALLBACK_2(MainWindow::MKOForLoginModuleCallBack));
    module::getUserListModule()->addObserver(this, BIND_CALLBACK_2(MainWindow::MKOForUserListModuleCallBack));
    module::getSessionModule()->addObserver(this, BIND_CALLBACK_2(MainWindow::MKOForSessionModuleCallBack));

    m_pbtnSysConfig = (CButtonUI*)m_PaintManager.FindControl(_T("sysconfig"));
    m_pbtnMyFace = (CButtonUI*)m_PaintManager.FindControl(_T("myfacebtn"));
    m_pbtnClose = (CButtonUI*)m_PaintManager.FindControl(_T("closebtn"));
    m_pbtnMinMize = (CButtonUI*)m_PaintManager.FindControl(_T("minbtn"));

    m_pTextUnreadMsgCount = static_cast<CTextUI*>(m_PaintManager.FindControl(_T("msgCount")));
    PTR_VOID(m_pTextUnreadMsgCount);

    //加载图标
    LoadIcons();
    //创建系统托盘
    CreateTrayIcon();
    //初始化
    Initilize();
}

void MainWindow::OnTextChanged(TNotifyUI& msg)
{
    if (msg.pSender->GetName() == _T("editSearch"))
    {
        CEditUI* pCEditUI = static_cast<CEditUI*>(m_PaintManager.FindControl(_T("editSearch")));
        CControlUI* pMainListLayout = m_PaintManager.FindControl(_T("MainListLayout"));
        if (pMainListLayout && pCEditUI)
        {
            if (!pCEditUI->GetText().IsEmpty())
            {
                pMainListLayout->SetVisible(false);
            }
            else
            {
                pMainListLayout->SetVisible(true);
            }
        }
    }
}

void MainWindow::CreateTrayIcon()
{
    CString sText = util::getMultilingual()->getStringById(_T("STRID_GLOBAL_CAPTION_NAME"));
#ifdef _DEBUG
    sText += _T("(Debug)");
#endif

    InstallIcon(sText, m_hIcons[ICON_TRAY_LOGO], IDR_MAINFRAME, FALSE);
}

void MainWindow::LoadIcons()
{
    CString csPath = module::getMiscModule()->getDataDir() + _T("icons\\newMsg.ico");
    HICON hIcon = 0;
    if (PathFileExists(csPath))
    {
        hIcon = (HICON)::LoadImage(NULL, csPath, IMAGE_ICON, 0, 0, LR_LOADFROMFILE);
        if (hIcon)
            m_hIcons[ICON_TRAY_NEWMSG] = hIcon;
        else
            LOG__(ERR, _T("LoadIcons failed ,icon %s"), csPath);
    }

    csPath = module::getMiscModule()->getDataDir() + _T("icons\\logo.ico");
    hIcon = 0;
    if (PathFileExists(csPath))
    {
        hIcon = (HICON)::LoadImage(NULL, csPath, IMAGE_ICON, 0, 0, LR_LOADFROMFILE);
        if (hIcon)
            m_hIcons[ICON_TRAY_LOGO] = hIcon;
        else
            LOG__(ERR, _T("LoadIcons failed ,icon %s"), csPath);
    }

    csPath = module::getMiscModule()->getDataDir() + _T("icons\\leave.ico");
    hIcon = 0;
    if (PathFileExists(csPath))
    {
        hIcon = (HICON)::LoadImage(NULL, csPath, IMAGE_ICON, 0, 0, LR_LOADFROMFILE);
        if (hIcon)
            m_hIcons[ICON_TRAY_LEAVE] = hIcon;
        else
            LOG__(ERR, _T("LoadIcons failed ,icon %s"), csPath);
    }
    csPath = module::getMiscModule()->getDataDir() + _T("icons\\offline.ico");
    hIcon = 0;
    if (PathFileExists(csPath))
    {
        hIcon = (HICON)::LoadImage(NULL, csPath, IMAGE_ICON, 0, 0, LR_LOADFROMFILE);
        if (hIcon)
            m_hIcons[ICON_TRAY_OFFLINE] = hIcon;
        else
            LOG__(ERR, _T("LoadIcons failed ,icon %s"), csPath);
    }
}

void MainWindow::UpdateLineStatus(UInt8 status)
{
    module::UserInfoEntity myInfo;
    module::getUserListModule()->getMyInfo(myInfo);
    m_pbtnMyFace->SetBkImage(util::stringToCString(myInfo.getAvatarPath()));

    if (IM::BaseDefine::UserStatType::USER_STATUS_ONLINE == status)
    {
        SetTrayIconIndex(ICON_TRAY_LOGO);
    }
    else
    {
        SetTrayIconIndex(ICON_TRAY_OFFLINE);
    }
}

void MainWindow::SetTrayIconIndex(int nIndex)
{
    HICON hIcon = NULL;
    if (nIndex < ICON_TRAY_LOGO || nIndex > ICON_COUNT)
        SetTrayIcon(0);
    else
        SetTrayIcon(m_hIcons[nIndex]);
}

void MainWindow::Initilize()
{
    //MKO
    module::getTcpClientModule()->addObserver(this, BIND_CALLBACK_2(MainWindow::MKOForTcpClientModuleCallBack));

    module::UserInfoEntity myInfo;
    module::getUserListModule()->getMyInfo(myInfo);
    m_pbtnMyFace->SetBkImage(util::stringToCString(myInfo.getAvatarPathWithoutOnlineState()));

    CString csPath = util::getAppPath() + _T("\\GifSmiley.dll");
    util::registerDll(csPath);


}

void MainWindow::InitWindow()
{
    MONITORINFO oMonitor = {};
    oMonitor.cbSize = sizeof(oMonitor);
    ::GetMonitorInfo(::MonitorFromWindow(*this, MONITOR_DEFAULTTOPRIMARY), &oMonitor);
    CDuiRect rcWork = oMonitor.rcWork;

    CDuiRect rcWnd;
    GetWindowRect(m_hWnd, &rcWnd);
    int nWidth = rcWnd.GetWidth();
    int nHeight = rcWnd.GetHeight();

#if 0
#ifdef _DEBUG
    SetWindowPos(m_hWnd, 0, rcWork.GetWidth() - nWidth - 100, 100, 280, rcWork.GetHeight() - 200, SWP_SHOWWINDOW);
#else
    SetWindowPos(m_hWnd, HWND_TOPMOST, rcWork.GetWidth() - nWidth - 100, 100, 280, rcWork.GetHeight() - 200, SWP_SHOWWINDOW);
#endif
#else
    CenterWindow();
#endif


}

LRESULT MainWindow::HandleCustomMessage(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    if (uMsg == WM_START_MOGUTALKINSTANCE)
    {
        BringToTop();
    }
    return 0;
}

LRESULT MainWindow::ResponseDefaultKeyEvent(WPARAM wParam)
{
    if (wParam == VK_ESCAPE)
    {
        return TRUE;
    }
    return __super::ResponseDefaultKeyEvent(wParam);
}

void MainWindow::_UpdateTotalUnReadMsgCount(void)
{
    PTR_VOID(m_pTextUnreadMsgCount);
    UInt32 nCount = module::getMessageModule()->getTotalUnReadMsgCount();
    if (0 == nCount)//没有消息
    {
        m_pTextUnreadMsgCount->SetVisible(false);
    }
    else if (nCount <= 99)
    {
        m_pTextUnreadMsgCount->SetText(util::int32ToCString(nCount));
        m_pTextUnreadMsgCount->SetVisible(true);
    }
    else if (nCount > 99)
    {
        m_pTextUnreadMsgCount->SetText(_T("99+"));
        m_pTextUnreadMsgCount->SetVisible(true);
    }

    CString sText = util::getMultilingual()->getStringById(_T("STRID_GLOBAL_CAPTION_NAME"));
#ifdef _DEBUG
    sText += _T("-Debug");
#endif
    CString sUnreadCnt;
    if (0 != nCount)
    {
        CString sFormat = util::getMultilingual()->getStringById(_T("STRID_MainWindow_TOOLTIP_MSGCNT"));
        sUnreadCnt.Format(sFormat, nCount);
    }
    SetTrayTooltipText(sText + sUnreadCnt);

}

void MainWindow::Notify(TNotifyUI& msg)
{
    __super::Notify(msg);
}

void MainWindow::CenterWindow()
{
    ASSERT(::IsWindow(m_hWnd));
    ASSERT((GetWindowStyle(m_hWnd)&WS_CHILD) == 0);
    RECT rcDlg = { 0 };
    ::GetWindowRect(m_hWnd, &rcDlg);
    RECT rcArea = { 0 };
    RECT rcCenter = { 0 };
    HWND hWnd = *this;
    HWND hWndParent = ::GetParent(m_hWnd);
    HWND hWndCenter = ::GetWindowOwner(m_hWnd);
    if (hWndCenter != NULL)
        hWnd = hWndCenter;

    // 处理多显示器模式下屏幕居中
    MONITORINFO oMonitor = {};
    oMonitor.cbSize = sizeof(oMonitor);
    ::GetMonitorInfo(::MonitorFromWindow(hWnd, MONITOR_DEFAULTTONEAREST), &oMonitor);
    rcArea = oMonitor.rcWork;

    if (hWndCenter == NULL)
        rcCenter = rcArea;
    else
        ::GetWindowRect(hWndCenter, &rcCenter);

    int DlgWidth = rcDlg.right - rcDlg.left;
    int DlgHeight = rcDlg.bottom - rcDlg.top;

    // Find dialog's upper left based on rcCenter
    int xLeft = (rcCenter.left + rcCenter.right) / 2 - DlgWidth / 2;
    int yTop = (rcCenter.top + rcCenter.bottom) / 2 - DlgHeight / 2;

    // The dialog is outside the screen, move it inside
    if (xLeft < rcArea.left) xLeft = rcArea.left;
    else if (xLeft + DlgWidth > rcArea.right) xLeft = rcArea.right - DlgWidth;
    if (yTop < rcArea.top) yTop = rcArea.top;
    else if (yTop + DlgHeight > rcArea.bottom) yTop = rcArea.bottom - DlgHeight;
    ::SetWindowPos(m_hWnd, NULL, xLeft, yTop, -1, -1, SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);
}


void MainWindow::OnTrayIconNotify(WPARAM wParam, LPARAM lParam)
{
    if (wParam != m_niData.uID)
        return;

    switch (LOWORD(lParam))
    {
    case WM_RBUTTONUP:
    {
        POINT point;
        GetCursorPos(&point);			// 获取鼠标指针位置（屏幕坐标）
        CMenuWnd* pMenu = new CMenuWnd(m_hWnd);
        //ScreenToClient(m_hWnd, &point);	// 将鼠标指针位置转换为窗口坐标
        //ClientToScreen(m_hWnd, &point);
        STRINGorID xml(_T("menu\\TrayIconMenu.xml"));
        pMenu->Init(NULL, xml, _T("xml"), point);
    }
    break;
    case WM_LBUTTONDOWN:
        BringToTop();
        StopNewMsgTrayEmot();
        break;
    case WM_LBUTTONDBLCLK:
        break;
    default:
        break;
    }
}

BOOL MainWindow::InstallIcon(LPCTSTR lpszToolTip, HICON hIcon, UINT nID, BOOL bReInstall /*= FALSE*/)
{
    if (bReInstall)
    {
        RemoveIcon();
    }
    else if (m_bInstalled)
    {
        return false;
    }

    ATLASSERT(m_hWnd && IsWindow(m_hWnd));
    m_niData.hWnd = m_hWnd;

    m_niData.uID = nID;
    m_niData.hIcon = hIcon;
    m_niData.uFlags = NIF_MESSAGE | NIF_ICON | NIF_TIP;
    m_niData.uCallbackMessage = WM_TRAYICON_NOTIFY;
    _tcscpy_s(m_niData.szTip, sizeof(m_niData.szTip) / sizeof(TCHAR), lpszToolTip);

    m_bInstalled = Shell_NotifyIcon(NIM_ADD, &m_niData) ? TRUE : false;

    return m_bInstalled;
}

BOOL MainWindow::RemoveIcon()
{
    if (!m_bInstalled)
        return false;

    m_niData.uFlags = 0;
    BOOL ret = Shell_NotifyIcon(NIM_DELETE, &m_niData) ? TRUE : false;

    if (ret)
    {
        m_bInstalled = false;
        m_bHidden = TRUE;
    }

    return ret;
}

BOOL MainWindow::SetTrayIcon(HICON hIcon)
{
    if (!m_bInstalled)
        return false;

    m_niData.uFlags = NIF_ICON;
    m_niData.hIcon = hIcon;

    return Shell_NotifyIcon(NIM_MODIFY, &m_niData) ? TRUE : false;
}

BOOL MainWindow::HideIcon(void)
{
    if (!m_bInstalled)
        return false;

    BOOL bResult = TRUE;
    if (!m_bHidden)
    {
        if (GetShellVersion() >= 5) //use the Shell v5 way of hiding the icon
        {
#if (_WIN32_IE >= 0x0500)
            m_niData.uFlags = NIF_STATE;
            m_niData.dwState = NIS_HIDDEN;
            m_niData.dwStateMask = NIS_HIDDEN;
#endif
            bResult = Shell_NotifyIcon(NIM_MODIFY, &m_niData) ? TRUE : false;
        }
        else
        {
            m_niData.uFlags = 0;
            bResult = Shell_NotifyIcon(NIM_DELETE, &m_niData) ? TRUE : false;
        }

        if (bResult)
        {
            m_bHidden = TRUE;
        }

    }
    return bResult;
}

BOOL MainWindow::ShowIcon()
{
    if (!m_bInstalled)
        return false;

    BOOL bResult = TRUE;
    if (m_bHidden)
    {
        if (GetShellVersion() >= 5) //use the Shell v5 way of showing the icon
        {
#if (_WIN32_IE >= 0x0500)
            m_niData.uFlags = NIF_STATE;
            m_niData.dwState = 0;
            m_niData.dwStateMask = NIS_HIDDEN;
            bResult = Shell_NotifyIcon(NIM_MODIFY, &m_niData) ? TRUE : false;
#endif
        }
        else
        {
            m_niData.uFlags = NIF_MESSAGE | NIF_ICON | NIF_TIP;
            bResult = Shell_NotifyIcon(NIM_ADD, &m_niData) ? TRUE : false;
        }

        if (bResult)
        {
            m_bHidden = false;
        }
    }
    return bResult;
}

BOOL MainWindow::SetTrayTooltipText(LPCTSTR pszTooltipText)
{
    if (!m_bInstalled)
        return false;

    m_niData.uFlags = NIF_TIP;
    _tcscpy_s(m_niData.szTip, sizeof(m_niData.szTip) / sizeof(TCHAR), pszTooltipText);

    return Shell_NotifyIcon(NIM_MODIFY, &m_niData) ? TRUE : false;
}

BOOL MainWindow::SetBalloonDetails(LPCTSTR pszBalloonText, LPCTSTR pszBalloonCaption, BalloonStyle style /*= BALLOON_INFO*/, UINT nTimeout /*= 1000*/, HICON hUserIcon /*= NULL*/, BOOL bNoSound /*= FALSE*/)
{
    if (!m_bInstalled)
        return false;

    ATLASSERT(GetShellVersion() >= 5); //Only supported on Shell v5 or later

#if (_WIN32_IE >= 0x0500)
    m_niData.uFlags = NIF_INFO;
#endif
#if (_MSC_VER >= 1400)
    _tcscpy_s(m_niData.szInfo, sizeof(m_niData.szInfo) / sizeof(TCHAR), pszBalloonText);
    _tcscpy_s(m_niData.szInfoTitle, sizeof(m_niData.szInfoTitle) / sizeof(TCHAR), pszBalloonCaption);
#else  
    _tcscpy(m_niData.szInfo, pszBalloonText);
    _tcscpy(m_niData.szInfoTitle, pszBalloonCaption);
#endif  
    m_niData.uTimeout = nTimeout;
#if (_WIN32_IE >= 0x0500)
    switch (style)
    {
    case BALLOON_WARNING:
        m_niData.dwInfoFlags = NIIF_WARNING;
        break;
    case BALLOON_ERROR:
        m_niData.dwInfoFlags = NIIF_ERROR;
        break;
    case BALLOON_INFO:
        m_niData.dwInfoFlags = NIIF_INFO;
        break;
    case BALLOON_NONE:
        m_niData.dwInfoFlags = NIIF_NONE;
        break;
#if (_WIN32_IE >= 0x0600)
    case BALLOON_USER:
        ATLASSERT(hUserIcon != NULL);
        m_niData.dwInfoFlags = NIIF_USER;
        m_niData.hIcon = hUserIcon;
        break;
#endif
    default:
        ATLASSERT(FALSE);
        break;
    }
#endif
#if (_WIN32_IE >= 0x0501)
    if (bNoSound)
        m_niData.dwInfoFlags |= NIIF_NOSOUND;
#endif
    return Shell_NotifyIcon(NIM_MODIFY, &m_niData) ? TRUE : false;
}

BOOL MainWindow::IsIconInstalled()
{
    return m_bInstalled;
}

BOOL MainWindow::SetVersion(UINT uVersion)
{
    ATLASSERT(GetShellVersion() >= 5);

    m_niData.uVersion = uVersion;
#if (_WIN32_IE >= 0x0500)
    return Shell_NotifyIcon(NIM_SETVERSION, &m_niData) ? TRUE : false;
#else
    return FALSE;
#endif
}

DWORD MainWindow::GetShellVersion(void)
{
    if (m_dwShellVersion)
    {
        return m_dwShellVersion;
    }
    else
    {
        m_dwShellVersion = 4;

        typedef HRESULT(CALLBACK *PFNDLLGETVERSION)(DLLVERSIONINFO*);

        HMODULE hShell32 = GetModuleHandle(_T("shell32.dll"));
        PFNDLLGETVERSION lpfnDllGetVersion = reinterpret_cast<PFNDLLGETVERSION>(GetProcAddress(hShell32, "DllGetVersion"));
        if (lpfnDllGetVersion)
        {
            DLLVERSIONINFO vinfo;
            vinfo.cbSize = sizeof(DLLVERSIONINFO);
            if (SUCCEEDED(lpfnDllGetVersion(&vinfo)))
                m_dwShellVersion = vinfo.dwMajorVersion;
        }
    }

    return m_dwShellVersion;
}


void MainWindow::OnClick(TNotifyUI& msg)
{
    PTR_VOID(msg.pSender);
    if (msg.pSender == m_pbtnSysConfig)
    {
        //系统设置
        module::getSysConfigModule()->showSysConfigDialog(m_hWnd);
    }
    else if (msg.pSender == m_pbtnMyFace)
    {
        //show the detail of myself.
        module::getSysConfigModule()->asynNotifyObserver(module::KEY_SYSCONFIG_SHOW_USERDETAILDIALOG, module::getSysConfigModule()->userID());
    }
    else if (msg.pSender == m_pbtnClose
        || msg.pSender == m_pbtnMinMize)
    {
        ShowWindow(false);
        return;
    }
    __super::OnClick(msg);
}

void MainWindow::MKOForTcpClientModuleCallBack(const std::string& keyId, MKO_TUPLE_PARAM mkoParam)
{
    if (module::KEY_TCPCLIENT_STATE == keyId)
    {
        //TCP长连断开处理
        if (module::TCPCLIENT_STATE_DISCONNECT == module::getTcpClientModule()->getTcpClientNetState())
        {
            LOG__(ERR, _T("TCPCLIENT_STATE_DISCONNECT:TCP Client link broken!!!"));

            //托盘区图标灰度掉
            CString csTip = util::getMultilingual()->getStringById(_T("STRID_MAINDIALOG_TIP_DISCONNECT"));
            SetTrayTooltipText(csTip);
            UpdateLineStatus(IM::BaseDefine::UserStatType::USER_STATUS_OFFLINE);

            //开启重连业务
            module::getTcpClientModule()->shutdown();
            if (!module::getLoginModule()->isOfflineByMyself())
                module::getLoginModule()->relogin(FALSE);
        }
    }
}
void MainWindow::OnCopyData(IN COPYDATASTRUCT* pCopyData)
{
    if (nullptr == pCopyData)
    {
        return;
    }
    module::getSessionModule()->asynNotifyObserver(module::KEY_SESSION_TRAY_COPYDATA, pCopyData->dwData);
}
void MainWindow::MKOForLoginModuleCallBack(const std::string& keyId, MKO_TUPLE_PARAM mkoParam)
{
    if (module::KEY_LOGIN_RELOGINOK == keyId)
    {
        CString sText = util::getMultilingual()->getStringById(_T("STRID_GLOBAL_CAPTION_NAME"));
#ifdef _DEBUG
        sText += _T("(Debug)");
#endif
        SetTrayTooltipText(sText);
        UpdateLineStatus(IM::BaseDefine::UserStatType::USER_STATUS_ONLINE);
    }
    else if (module::KEY_LOGIN_KICKOUT == keyId)//账户被踢出了，设置自己离线
    {
        Int32 nRes = std::get<MKO_INT>(mkoParam);
        LOG__(APP, _T("Offline by kickout,%d"), nRes);
        module::getTcpClientModule()->shutdown();
        module::getLoginModule()->setOfflineByMyself(TRUE);
        CString strText = util::getMultilingual()->getStringById(_T("STRID_LOGINDIALOG_TIP_DUPLICATE_USER_KICKOUT"));
        if (IM::BaseDefine::KickReasonType::KICK_REASON_MOBILE_KICK == nRes)
        {
            strText = util::getMultilingual()->getStringById(_T("STRID_LOGINDIALOG_TIP_MOBILE_KICK"));
        }
        CString strCaption = util::getMultilingual()->getStringById(_T("STRID_LOGINDIALOG_TIP_CAPTION"));
        ::MessageBoxEx(m_hWnd, strText, strCaption, MB_OK, LANG_CHINESE);
    }
}

void MainWindow::OnMenuClicked(IN const CString& itemName, IN const CString& strLparam)
{
    if (_T("exitMenuItem") == itemName)
    {
        module::getMiscModule()->quitTheApplication();
    }
    else if (_T("LogDir") == itemName)//打开日志目录
    {
        CString strLogDir = util::getParentAppPath() + _T("bin\\log\\ttlog.log");
        if (PathFileExists(strLogDir))
        {
            HINSTANCE hr = ShellExecute(NULL, _T("open"), _T("Explorer.exe"), _T("/select,") + strLogDir, NULL, SW_SHOWDEFAULT);
            if ((long)hr < 32)
            {
                DWORD dwLastError = ::GetLastError();
                LOG__(ERR, _T("ShellExecute Failed GetLastError = %d")
                    , dwLastError);
            }
        }
    }
    else if (_T("OnlineMenuItem") == itemName)
    {
        if (IM::BaseDefine::USER_STATUS_ONLINE == module::getUserListModule()->getMyLineStatus())
        {
            LOG__(APP, _T("already online"));
            return;
        }
        module::getLoginModule()->setOfflineByMyself(FALSE);
        module::getLoginModule()->relogin(TRUE);
    }
    else if (_T("OfflineMenuItem") == itemName)
    {
        if (IM::BaseDefine::USER_STATUS_OFFLINE == module::getUserListModule()->getMyLineStatus())
        {
            LOG__(APP, _T("already offline"));
            return;
        }
        LOG__(APP, _T("Offline by myself"));
        module::getTcpClientModule()->shutdown();
        module::getLoginModule()->setOfflineByMyself(TRUE);
    }
    else if (_T("SysSettingMenuItem") == itemName)
    {
        //系统设置
        module::getSysConfigModule()->showSysConfigDialog(m_hWnd);
    }
    else if (_T("serverAddressSetting") == itemName)
    {
        module::getSysConfigModule()->showServerConfigDialog(m_hWnd);
    }
    else if (_T("showFileTransferDialog") == itemName)
    {
        //打开文件传输面板
        module::getFileTransferModule()->showFileTransferDialog();
    }
    else if (_T("UserDetailInfo") == itemName)
    {
        if (strLparam.IsEmpty())
        {
            return;
        }
        std::string sid = util::cStringToString(strLparam);
        module::getSysConfigModule()->asynNotifyObserver(module::KEY_SYSCONFIG_SHOW_USERDETAILDIALOG, sid);
    }
}

void MainWindow::MKOForUserListModuleCallBack(const std::string& keyId, MKO_TUPLE_PARAM mkoParam)
{
    if (module::KEY_USERLIST_DOWNAVATAR_SUCC == keyId)
    {
        std::shared_ptr<void> p = std::get<MKO_SHARED_VOIDPTR>(mkoParam);
        std::tuple<std::string, std::string>* pTuple = (std::tuple<std::string, std::string>*)p.get();
        std::string& sId = std::get<1>(*pTuple);
        //刷新头像
        module::UserInfoEntity myInfo;
        module::getUserListModule()->getMyInfo(myInfo);
        if (sId == myInfo.sId)
        {
            if (m_pbtnMyFace)
                m_pbtnMyFace->SetBkImage(util::stringToCString(myInfo.getAvatarPath()));
        }
    }
    else if (module::KEY_USERLIST_USERSIGNINFO_CHANGED == keyId)
    {
        //更新自己的签名，若失败则改回去。
        std::string& sId = std::get<MKO_STRING>(mkoParam);
        if (sId != module::getSysConfigModule()->userID())
        {
            return;
        }
    }
}
void MainWindow::MKOForSessionModuleCallBack(const std::string& keyId, MKO_TUPLE_PARAM mkoParam)
{
    if (module::KEY_SESSION_TRAY_STARTEMOT == keyId)
    {
        StartNewMsgTrayEmot();
    }
    else if (module::KEY_SESSION_TRAY_STOPEMOT == keyId)
    {
        StopNewMsgTrayEmot();
    }
    else if (module::KEY_SESSION_UPDATE_TOTAL_UNREADMSG_COUNT == keyId)
    {
        _UpdateTotalUnReadMsgCount();//更新总未读计数
    }
    else if (module::KEY_SESSION_SHAKEWINDOW_MSG == keyId){
        {
            HANDLE hThread = CreateThread(NULL, 0, WindowShake, m_hWnd, 0, NULL);//窗口抖动
            CloseHandle(hThread);
        }
    }
}

void MainWindow::StartNewMsgTrayEmot()
{
    SetTimer(m_hWnd, TIMER_TRAYEMOT, 500, NULL);
}

void MainWindow::StopNewMsgTrayEmot()
{
    KillTimer(m_hWnd, TIMER_TRAYEMOT);
    UpdateLineStatus(module::getUserListModule()->getMyLineStatus());
}