/****************************************************************************
**
** Copyright (C) 2013 Ivan Vizir <define-true-false@yandex.com>
** Copyright (C) 2013 Digia Plc and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/legal
**
** This file is part of the QtWinExtras module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Digia.  For licensing terms and
** conditions see http://qt.digia.com/licensing.  For further information
** use the contact form at http://qt.digia.com/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Digia gives you certain additional
** rights.  These rights are described in the Digia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3.0 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU General Public License version 3.0 requirements will be
** met: http://www.gnu.org/copyleft/gpl.html.
**
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "qwinfunctions.h"
#include "qwinfunctions_p.h"
#include "qwineventfilter_p.h"
#include "windowsguidsdefs_p.h"

#include <QGuiApplication>
#include <QWindow>
#include <QSettings>
#include <QPixmap>
#include <QBitmap>
#include <QImage>
#include <QColor>
#include <QRegion>
#include <QMargins>

#include <comdef.h>
#include "winshobjidl_p.h"

QT_BEGIN_NAMESPACE

Q_GUI_EXPORT HBITMAP qt_createIconMask(const QBitmap &bitmap);
Q_GUI_EXPORT HBITMAP qt_pixmapToWinHBITMAP(const QPixmap &p, int hbitmapFormat = 0);
Q_GUI_EXPORT QPixmap qt_pixmapFromWinHBITMAP(HBITMAP bitmap, int hbitmapFormat = 0);
Q_GUI_EXPORT HICON   qt_pixmapToWinHICON(const QPixmap &p);
Q_GUI_EXPORT QImage  qt_imageFromWinHBITMAP(HDC hdc, HBITMAP bitmap, int w, int h);
Q_GUI_EXPORT QPixmap qt_pixmapFromWinHICON(HICON icon);

/*!
    \namespace QtWin
    \inmodule QtWinExtras
    \brief The QtWin namespace contains miscellaneous Windows-specific functions.
    \inheaderfile QtWin

 */

/*!
    \since 5.2

    Creates a \c HBITMAP equivalent of the QBitmap \a bitmap.

    It is the caller's responsibility to free the \c HBITMAP data after use.

    \sa toHBITMAP()
*/
HBITMAP QtWin::createMask(const QBitmap &bitmap)
{
    return qt_createIconMask(bitmap);
}

/*!
    \since 5.2

    Creates a \c HBITMAP equivalent of the QPixmap \a p,
    based on the given \a format. Returns the \c HBITMAP handle.

    It is the caller's responsibility to free the \c HBITMAP data
    after use.

    \sa fromHBITMAP()
*/
HBITMAP QtWin::toHBITMAP(const QPixmap &p, QtWin::HBitmapFormat format)
{
    return qt_pixmapToWinHBITMAP(p, format);
}

/*!
    \since 5.2

    Returns a QPixmap that is equivalent to the
    given \a bitmap. The conversion is based on the specified \a format.

    \sa toHBITMAP()
*/
QPixmap QtWin::fromHBITMAP(HBITMAP bitmap, QtWin::HBitmapFormat format)
{
    return qt_pixmapFromWinHBITMAP(bitmap, format);
}

/*!
    \since 5.2

    Creates a \c HICON equivalent of the QPixmap \a p.
    Returns the \c HICON handle.

    It is the caller's responsibility to free the \c HICON data after use.

    \sa fromHICON()
*/
HICON QtWin::toHICON(const QPixmap &p)
{
    return qt_pixmapToWinHICON(p);
}

/*!
    \since 5.2

    Returns a QImage that is equivalent to the
    given \a bitmap. The conversion is based on the specified \c HDC context \a hdc
    using the specified \a width and \a height.

    \sa toHBITMAP()
*/
QImage QtWin::imageFromHBITMAP(HDC hdc, HBITMAP bitmap, int width, int height)
{
    return qt_imageFromWinHBITMAP(hdc, bitmap, width, height);
}

/*!
    \since 5.2

    Returns a QPixmap that is equivalent to the given \a icon.

    \sa toHICON()
*/
QPixmap QtWin::fromHICON(HICON icon)
{
    return qt_pixmapFromWinHICON(icon);
}

HRGN qt_RectToHRGN(const QRect &rc)
{
    return CreateRectRgn(rc.left(), rc.top(), rc.right()+1, rc.bottom()+1);
}

/*!
    \since 5.2

    Returns a HRGN that is equivalent to the given \a region.
 */
HRGN QtWin::toHRGN(const QRegion &region)
{
    if (region.isNull() || region.rectCount() == 0) {
        return 0;
    }
    HRGN resultRgn = 0;
    QVector<QRect> rects = region.rects();
    resultRgn = qt_RectToHRGN(rects.at(0));
    const int size = rects.size();
    for (int i = 1; i < size; i++) {
        HRGN tmpRgn = qt_RectToHRGN(rects.at(i));
        int err = CombineRgn(resultRgn, resultRgn, tmpRgn, RGN_OR);
        if (err == ERROR)
            qWarning("Error combining HRGNs.");
        DeleteObject(tmpRgn);
    }
    return resultRgn;
}

/*!
    \since 5.2

    Returns a QRegion that is equivalent to the given \a hrgn.
 */
QRegion QtWin::fromHRGN(HRGN hrgn)
{
    DWORD regionDataSize = GetRegionData(hrgn, 0, NULL);
    if (regionDataSize == 0)
        return QRegion();

    LPRGNDATA regionData = (LPRGNDATA)malloc(regionDataSize);
    if (!regionData)
        return QRegion();

    QRegion region;
    if (GetRegionData(hrgn, regionDataSize, regionData) == regionDataSize) {
        LPRECT pRect = (LPRECT)regionData->Buffer;
        for (DWORD i = 0; i < regionData->rdh.nCount; ++i)
            region += QRect(pRect[i].left, pRect[i].top,
                            pRect[i].right - pRect[i].left,
                            pRect[i].bottom - pRect[i].top);
    }

    free(regionData);
    return region;
}

/*!
    \since 5.2

    Returns a message string that explains the \a hresult error id specified or
    an empty string if the explanation cannot be found.
 */
QString QtWin::stringFromHresult(HRESULT hresult)
{
    _com_error error(hresult);
    QString errorMsg;
    if (sizeof(TCHAR) == sizeof(wchar_t))
        errorMsg = QString::fromWCharArray((wchar_t*) error.ErrorMessage());
    else
        errorMsg = QString::fromLocal8Bit((char*) error.ErrorMessage());
    return errorMsg;
}

/*!
    \since 5.2

    Returns the code name of the \a hresult error id specified (usually the name
    of the WinAPI macro) or an empty string if the message is unknown.
 */
QString QtWin::errorStringFromHresult(HRESULT hresult)
{
    switch (hresult) {
    case 0x8000FFFF : return QStringLiteral("E_UNEXPECTED");
    case 0x80004001 : return QStringLiteral("E_NOTIMPL");
    case 0x8007000E : return QStringLiteral("E_OUTOFMEMORY");
    case 0x80070057 : return QStringLiteral("E_INVALIDARG");
    case 0x80004002 : return QStringLiteral("E_NOINTERFACE");
    case 0x80004003 : return QStringLiteral("E_POINTER");
    case 0x80070006 : return QStringLiteral("E_HANDLE");
    case 0x80004004 : return QStringLiteral("E_ABORT");
    case 0x80004005 : return QStringLiteral("E_FAIL");
    case 0x80070005 : return QStringLiteral("E_ACCESSDENIED");
    case 0x8000000A : return QStringLiteral("E_PENDING");
    case 0x80004006 : return QStringLiteral("CO_E_INIT_TLS");
    case 0x80004007 : return QStringLiteral("CO_E_INIT_SHARED_ALLOCATOR");
    case 0x80004008 : return QStringLiteral("CO_E_INIT_MEMORY_ALLOCATOR");
    case 0x80004009 : return QStringLiteral("CO_E_INIT_CLASS_CACHE");
    case 0x8000400A : return QStringLiteral("CO_E_INIT_RPC_CHANNEL");
    case 0x8000400B : return QStringLiteral("CO_E_INIT_TLS_SET_CHANNEL_CONTROL");
    case 0x8000400C : return QStringLiteral("CO_E_INIT_TLS_CHANNEL_CONTROL");
    case 0x8000400D : return QStringLiteral("CO_E_INIT_UNACCEPTED_USER_ALLOCATOR");
    case 0x8000400E : return QStringLiteral("CO_E_INIT_SCM_MUTEX_EXISTS");
    case 0x8000400F : return QStringLiteral("CO_E_INIT_SCM_FILE_MAPPING_EXISTS");
    case 0x80004010 : return QStringLiteral("CO_E_INIT_SCM_MAP_VIEW_OF_FILE");
    case 0x80004011 : return QStringLiteral("CO_E_INIT_SCM_EXEC_FAILURE");
    case 0x80004012 : return QStringLiteral("CO_E_INIT_ONLY_SINGLE_THREADED");
    case 0x80004013 : return QStringLiteral("CO_E_CANT_REMOTE");
    case 0x80004014 : return QStringLiteral("CO_E_BAD_SERVER_NAME");
    case 0x80004015 : return QStringLiteral("CO_E_WRONG_SERVER_IDENTITY");
    case 0x80004016 : return QStringLiteral("CO_E_OLE1DDE_DISABLED");
    case 0x80004017 : return QStringLiteral("CO_E_RUNAS_SYNTAX");
    case 0x80004018 : return QStringLiteral("CO_E_CREATEPROCESS_FAILURE");
    case 0x80004019 : return QStringLiteral("CO_E_RUNAS_CREATEPROCESS_FAILURE");
    case 0x8000401A : return QStringLiteral("CO_E_RUNAS_LOGON_FAILURE");
    case 0x8000401B : return QStringLiteral("CO_E_LAUNCH_PERMSSION_DENIED");
    case 0x8000401C : return QStringLiteral("CO_E_START_SERVICE_FAILURE");
    case 0x8000401D : return QStringLiteral("CO_E_REMOTE_COMMUNICATION_FAILURE");
    case 0x8000401E : return QStringLiteral("CO_E_SERVER_START_TIMEOUT");
    case 0x8000401F : return QStringLiteral("CO_E_CLSREG_INCONSISTENT");
    case 0x80004020 : return QStringLiteral("CO_E_IIDREG_INCONSISTENT");
    case 0x80004021 : return QStringLiteral("CO_E_NOT_SUPPORTED");
    case 0x80004022 : return QStringLiteral("CO_E_RELOAD_DLL");
    case 0x80004023 : return QStringLiteral("CO_E_MSI_ERROR");
    case 0x80004024 : return QStringLiteral("CO_E_ATTEMPT_TO_CREATE_OUTSIDE_CLIENT_CONTEXT");
    case 0x80004025 : return QStringLiteral("CO_E_SERVER_PAUSED");
    case 0x80004026 : return QStringLiteral("CO_E_SERVER_NOT_PAUSED");
    case 0x80004027 : return QStringLiteral("CO_E_CLASS_DISABLED");
    case 0x80004028 : return QStringLiteral("CO_E_CLRNOTAVAILABLE");
    case 0x80004029 : return QStringLiteral("CO_E_ASYNC_WORK_REJECTED");
    case 0x8000402A : return QStringLiteral("CO_E_SERVER_INIT_TIMEOUT");
    case 0x8000402B : return QStringLiteral("CO_E_NO_SECCTX_IN_ACTIVATE");
    case 0x80004030 : return QStringLiteral("CO_E_TRACKER_CONFIG");
    case 0x80004031 : return QStringLiteral("CO_E_THREADPOOL_CONFIG");
    case 0x80004032 : return QStringLiteral("CO_E_SXS_CONFIG");
    case 0x80004033 : return QStringLiteral("CO_E_MALFORMED_SPN");
    case 0x80040000 : return QStringLiteral("OLE_E_OLEVERB");
    case 0x80040001 : return QStringLiteral("OLE_E_ADVF");
    case 0x80040002 : return QStringLiteral("OLE_E_ENUM_NOMORE");
    case 0x80040003 : return QStringLiteral("OLE_E_ADVISENOTSUPPORTED");
    case 0x80040004 : return QStringLiteral("OLE_E_NOCONNECTION");
    case 0x80040005 : return QStringLiteral("OLE_E_NOTRUNNING");
    case 0x80040006 : return QStringLiteral("OLE_E_NOCACHE");
    case 0x80040007 : return QStringLiteral("OLE_E_BLANK");
    case 0x80040008 : return QStringLiteral("OLE_E_CLASSDIFF");
    case 0x80040009 : return QStringLiteral("OLE_E_CANT_GETMONIKER");
    case 0x8004000A : return QStringLiteral("OLE_E_CANT_BINDTOSOURCE");
    case 0x8004000B : return QStringLiteral("OLE_E_STATIC");
    case 0x8004000C : return QStringLiteral("OLE_E_PROMPTSAVECANCELLED");
    case 0x8004000D : return QStringLiteral("OLE_E_INVALIDRECT");
    case 0x8004000E : return QStringLiteral("OLE_E_WRONGCOMPOBJ");
    case 0x8004000F : return QStringLiteral("OLE_E_INVALIDHWND");
    case 0x80040010 : return QStringLiteral("OLE_E_NOT_INPLACEACTIVE");
    case 0x80040011 : return QStringLiteral("OLE_E_CANTCONVERT");
    case 0x80040012 : return QStringLiteral("OLE_E_NOSTORAGE");
    case 0x80040064 : return QStringLiteral("DV_E_FORMATETC");
    case 0x80040065 : return QStringLiteral("DV_E_DVTARGETDEVICE");
    case 0x80040066 : return QStringLiteral("DV_E_STGMEDIUM");
    case 0x80040067 : return QStringLiteral("DV_E_STATDATA");
    case 0x80040068 : return QStringLiteral("DV_E_LINDEX");
    case 0x80040069 : return QStringLiteral("DV_E_TYMED");
    case 0x8004006A : return QStringLiteral("DV_E_CLIPFORMAT");
    case 0x8004006B : return QStringLiteral("DV_E_DVASPECT");
    case 0x8004006C : return QStringLiteral("DV_E_DVTARGETDEVICE_SIZE");
    case 0x8004006D : return QStringLiteral("DV_E_NOIVIEWOBJECT");
    case 0x80040100 : return QStringLiteral("DRAGDROP_E_NOTREGISTERED");
    case 0x80040101 : return QStringLiteral("DRAGDROP_E_ALREADYREGISTERED");
    case 0x80040102 : return QStringLiteral("DRAGDROP_E_INVALIDHWND");
    case 0x80040110 : return QStringLiteral("CLASS_E_NOAGGREGATION");
    case 0x80040111 : return QStringLiteral("CLASS_E_CLASSNOTAVAILABLE");
    case 0x80040112 : return QStringLiteral("CLASS_E_NOTLICENSED");
    case 0x80040140 : return QStringLiteral("VIEW_E_DRAW");
    case 0x80040150 : return QStringLiteral("REGDB_E_READREGDB");
    case 0x80040151 : return QStringLiteral("REGDB_E_WRITEREGDB");
    case 0x80040152 : return QStringLiteral("REGDB_E_KEYMISSING");
    case 0x80040153 : return QStringLiteral("REGDB_E_INVALIDVALUE");
    case 0x80040154 : return QStringLiteral("REGDB_E_CLASSNOTREG");
    case 0x80040155 : return QStringLiteral("REGDB_E_IIDNOTREG");
    case 0x80040156 : return QStringLiteral("REGDB_E_BADTHREADINGMODEL");
    case 0x80040160 : return QStringLiteral("CAT_E_CATIDNOEXIST");
    case 0x80040161 : return QStringLiteral("CAT_E_NODESCRIPTION");
    case 0x80040164 : return QStringLiteral("CS_E_PACKAGE_NOTFOUND");
    case 0x80040165 : return QStringLiteral("CS_E_NOT_DELETABLE");
    case 0x80040166 : return QStringLiteral("CS_E_CLASS_NOTFOUND");
    case 0x80040167 : return QStringLiteral("CS_E_INVALID_VERSION");
    case 0x80040168 : return QStringLiteral("CS_E_NO_CLASSSTORE");
    case 0x80040169 : return QStringLiteral("CS_E_OBJECT_NOTFOUND");
    case 0x8004016A : return QStringLiteral("CS_E_OBJECT_ALREADY_EXISTS");
    case 0x8004016B : return QStringLiteral("CS_E_INVALID_PATH");
    case 0x8004016C : return QStringLiteral("CS_E_NETWORK_ERROR");
    case 0x8004016D : return QStringLiteral("CS_E_ADMIN_LIMIT_EXCEEDED");
    case 0x8004016E : return QStringLiteral("CS_E_SCHEMA_MISMATCH");
    case 0x8004016F : return QStringLiteral("CS_E_INTERNAL_ERROR");
    case 0x80040170 : return QStringLiteral("CACHE_E_NOCACHE_UPDATED");
    case 0x80040180 : return QStringLiteral("OLEOBJ_E_NOVERBS");
    case 0x80040181 : return QStringLiteral("OLEOBJ_E_INVALIDVERB");
    case 0x800401A0 : return QStringLiteral("INPLACE_E_NOTUNDOABLE");
    case 0x800401A1 : return QStringLiteral("INPLACE_E_NOTOOLSPACE");
    case 0x800401C0 : return QStringLiteral("CONVERT10_E_OLESTREAM_GET");
    case 0x800401C1 : return QStringLiteral("CONVERT10_E_OLESTREAM_PUT");
    case 0x800401C2 : return QStringLiteral("CONVERT10_E_OLESTREAM_FMT");
    case 0x800401C3 : return QStringLiteral("CONVERT10_E_OLESTREAM_BITMAP_TO_DIB");
    case 0x800401C4 : return QStringLiteral("CONVERT10_E_STG_FMT");
    case 0x800401C5 : return QStringLiteral("CONVERT10_E_STG_NO_STD_STREAM");
    case 0x800401C6 : return QStringLiteral("CONVERT10_E_STG_DIB_TO_BITMAP");
    case 0x800401D0 : return QStringLiteral("CLIPBRD_E_CANT_OPEN");
    case 0x800401D1 : return QStringLiteral("CLIPBRD_E_CANT_EMPTY");
    case 0x800401D2 : return QStringLiteral("CLIPBRD_E_CANT_SET");
    case 0x800401D3 : return QStringLiteral("CLIPBRD_E_BAD_DATA");
    case 0x800401D4 : return QStringLiteral("CLIPBRD_E_CANT_CLOSE");
    case 0x800401E0 : return QStringLiteral("MK_E_CONNECTMANUALLY");
    case 0x800401E1 : return QStringLiteral("MK_E_EXCEEDEDDEADLINE");
    case 0x800401E2 : return QStringLiteral("MK_E_NEEDGENERIC");
    case 0x800401E3 : return QStringLiteral("MK_E_UNAVAILABLE");
    case 0x800401E4 : return QStringLiteral("MK_E_SYNTAX");
    case 0x800401E5 : return QStringLiteral("MK_E_NOOBJECT");
    case 0x800401E6 : return QStringLiteral("MK_E_INVALIDEXTENSION");
    case 0x800401E7 : return QStringLiteral("MK_E_INTERMEDIATEINTERFACENOTSUPPORTED");
    case 0x800401E8 : return QStringLiteral("MK_E_NOTBINDABLE");
    case 0x800401E9 : return QStringLiteral("MK_E_NOTBOUND");
    case 0x800401EA : return QStringLiteral("MK_E_CANTOPENFILE");
    case 0x800401EB : return QStringLiteral("MK_E_MUSTBOTHERUSER");
    case 0x800401EC : return QStringLiteral("MK_E_NOINVERSE");
    case 0x800401ED : return QStringLiteral("MK_E_NOSTORAGE");
    case 0x800401EE : return QStringLiteral("MK_E_NOPREFIX");
    case 0x800401EF : return QStringLiteral("MK_E_ENUMERATION_FAILED");
    case 0x800401F0 : return QStringLiteral("CO_E_NOTINITIALIZED");
    case 0x800401F1 : return QStringLiteral("CO_E_ALREADYINITIALIZED");
    case 0x800401F2 : return QStringLiteral("CO_E_CANTDETERMINECLASS");
    case 0x800401F3 : return QStringLiteral("CO_E_CLASSSTRING");
    case 0x800401F4 : return QStringLiteral("CO_E_IIDSTRING");
    case 0x800401F5 : return QStringLiteral("CO_E_APPNOTFOUND");
    case 0x800401F6 : return QStringLiteral("CO_E_APPSINGLEUSE");
    case 0x800401F7 : return QStringLiteral("CO_E_ERRORINAPP");
    case 0x800401F8 : return QStringLiteral("CO_E_DLLNOTFOUND");
    case 0x800401F9 : return QStringLiteral("CO_E_ERRORINDLL");
    case 0x800401FA : return QStringLiteral("CO_E_WRONGOSFORAPP");
    case 0x800401FB : return QStringLiteral("CO_E_OBJNOTREG");
    case 0x800401FC : return QStringLiteral("CO_E_OBJISREG");
    case 0x800401FD : return QStringLiteral("CO_E_OBJNOTCONNECTED");
    case 0x800401FE : return QStringLiteral("CO_E_APPDIDNTREG");
    case 0x800401FF : return QStringLiteral("CO_E_RELEASED");
    case 0x00040200 : return QStringLiteral("EVENT_S_SOME_SUBSCRIBERS_FAILED");
    case 0x80040201 : return QStringLiteral("EVENT_E_ALL_SUBSCRIBERS_FAILED");
    case 0x00040202 : return QStringLiteral("EVENT_S_NOSUBSCRIBERS");
    case 0x80040203 : return QStringLiteral("EVENT_E_QUERYSYNTAX");
    case 0x80040204 : return QStringLiteral("EVENT_E_QUERYFIELD");
    case 0x80040205 : return QStringLiteral("EVENT_E_INTERNALEXCEPTION");
    case 0x80040206 : return QStringLiteral("EVENT_E_INTERNALERROR");
    case 0x80040207 : return QStringLiteral("EVENT_E_INVALID_PER_USER_SID");
    case 0x80040208 : return QStringLiteral("EVENT_E_USER_EXCEPTION");
    case 0x80040209 : return QStringLiteral("EVENT_E_TOO_MANY_METHODS");
    case 0x8004020A : return QStringLiteral("EVENT_E_MISSING_EVENTCLASS");
    case 0x8004020B : return QStringLiteral("EVENT_E_NOT_ALL_REMOVED");
    case 0x8004020C : return QStringLiteral("EVENT_E_COMPLUS_NOT_INSTALLED");
    case 0x8004020D : return QStringLiteral("EVENT_E_CANT_MODIFY_OR_DELETE_UNCONFIGURED_OBJECT");
    case 0x8004020E : return QStringLiteral("EVENT_E_CANT_MODIFY_OR_DELETE_CONFIGURED_OBJECT");
    case 0x8004020F : return QStringLiteral("EVENT_E_INVALID_EVENT_CLASS_PARTITION");
    case 0x80040210 : return QStringLiteral("EVENT_E_PER_USER_SID_NOT_LOGGED_ON");
    case 0x8004D000 : return QStringLiteral("XACT_E_ALREADYOTHERSINGLEPHASE");
    case 0x8004D001 : return QStringLiteral("XACT_E_CANTRETAIN");
    case 0x8004D002 : return QStringLiteral("XACT_E_COMMITFAILED");
    case 0x8004D003 : return QStringLiteral("XACT_E_COMMITPREVENTED");
    case 0x8004D004 : return QStringLiteral("XACT_E_HEURISTICABORT");
    case 0x8004D005 : return QStringLiteral("XACT_E_HEURISTICCOMMIT");
    case 0x8004D006 : return QStringLiteral("XACT_E_HEURISTICDAMAGE");
    case 0x8004D007 : return QStringLiteral("XACT_E_HEURISTICDANGER");
    case 0x8004D008 : return QStringLiteral("XACT_E_ISOLATIONLEVEL");
    case 0x8004D009 : return QStringLiteral("XACT_E_NOASYNC");
    case 0x8004D00A : return QStringLiteral("XACT_E_NOENLIST");
    case 0x8004D00B : return QStringLiteral("XACT_E_NOISORETAIN");
    case 0x8004D00C : return QStringLiteral("XACT_E_NORESOURCE");
    case 0x8004D00D : return QStringLiteral("XACT_E_NOTCURRENT");
    case 0x8004D00E : return QStringLiteral("XACT_E_NOTRANSACTION");
    case 0x8004D00F : return QStringLiteral("XACT_E_NOTSUPPORTED");
    case 0x8004D010 : return QStringLiteral("XACT_E_UNKNOWNRMGRID");
    case 0x8004D011 : return QStringLiteral("XACT_E_WRONGSTATE");
    case 0x8004D012 : return QStringLiteral("XACT_E_WRONGUOW");
    case 0x8004D013 : return QStringLiteral("XACT_E_XTIONEXISTS");
    case 0x8004D014 : return QStringLiteral("XACT_E_NOIMPORTOBJECT");
    case 0x8004D015 : return QStringLiteral("XACT_E_INVALIDCOOKIE");
    case 0x8004D016 : return QStringLiteral("XACT_E_INDOUBT");
    case 0x8004D017 : return QStringLiteral("XACT_E_NOTIMEOUT");
    case 0x8004D018 : return QStringLiteral("XACT_E_ALREADYINPROGRESS");
    case 0x8004D019 : return QStringLiteral("XACT_E_ABORTED");
    case 0x8004D01A : return QStringLiteral("XACT_E_LOGFULL");
    case 0x8004D01B : return QStringLiteral("XACT_E_TMNOTAVAILABLE");
    case 0x8004D01C : return QStringLiteral("XACT_E_CONNECTION_DOWN");
    case 0x8004D01D : return QStringLiteral("XACT_E_CONNECTION_DENIED");
    case 0x8004D01E : return QStringLiteral("XACT_E_REENLISTTIMEOUT");
    case 0x8004D01F : return QStringLiteral("XACT_E_TIP_CONNECT_FAILED");
    case 0x8004D020 : return QStringLiteral("XACT_E_TIP_PROTOCOL_ERROR");
    case 0x8004D021 : return QStringLiteral("XACT_E_TIP_PULL_FAILED");
    case 0x8004D022 : return QStringLiteral("XACT_E_DEST_TMNOTAVAILABLE");
    case 0x8004D023 : return QStringLiteral("XACT_E_TIP_DISABLED");
    case 0x8004D024 : return QStringLiteral("XACT_E_NETWORK_TX_DISABLED");
    case 0x8004D025 : return QStringLiteral("XACT_E_PARTNER_NETWORK_TX_DISABLED");
    case 0x8004D026 : return QStringLiteral("XACT_E_XA_TX_DISABLED");
    case 0x8004D027 : return QStringLiteral("XACT_E_UNABLE_TO_READ_DTC_CONFIG");
    case 0x8004D028 : return QStringLiteral("XACT_E_UNABLE_TO_LOAD_DTC_PROXY");
    case 0x8004D029 : return QStringLiteral("XACT_E_ABORTING");
    case 0x8004D080 : return QStringLiteral("XACT_E_CLERKNOTFOUND");
    case 0x8004D081 : return QStringLiteral("XACT_E_CLERKEXISTS");
    case 0x8004D082 : return QStringLiteral("XACT_E_RECOVERYINPROGRESS");
    case 0x8004D083 : return QStringLiteral("XACT_E_TRANSACTIONCLOSED");
    case 0x8004D084 : return QStringLiteral("XACT_E_INVALIDLSN");
    case 0x8004D085 : return QStringLiteral("XACT_E_REPLAYREQUEST");
    case 0x0004D000 : return QStringLiteral("XACT_S_ASYNC");
    case 0x0004D001 : return QStringLiteral("XACT_S_DEFECT");
    case 0x0004D002 : return QStringLiteral("XACT_S_READONLY");
    case 0x0004D003 : return QStringLiteral("XACT_S_SOMENORETAIN");
    case 0x0004D004 : return QStringLiteral("XACT_S_OKINFORM");
    case 0x0004D005 : return QStringLiteral("XACT_S_MADECHANGESCONTENT");
    case 0x0004D006 : return QStringLiteral("XACT_S_MADECHANGESINFORM");
    case 0x0004D007 : return QStringLiteral("XACT_S_ALLNORETAIN");
    case 0x0004D008 : return QStringLiteral("XACT_S_ABORTING");
    case 0x0004D009 : return QStringLiteral("XACT_S_SINGLEPHASE");
    case 0x0004D00A : return QStringLiteral("XACT_S_LOCALLY_OK");
    case 0x0004D010 : return QStringLiteral("XACT_S_LASTRESOURCEMANAGER");
    case 0x8004E002 : return QStringLiteral("CONTEXT_E_ABORTED");
    case 0x8004E003 : return QStringLiteral("CONTEXT_E_ABORTING");
    case 0x8004E004 : return QStringLiteral("CONTEXT_E_NOCONTEXT");
    case 0x8004E005 : return QStringLiteral("CONTEXT_E_WOULD_DEADLOCK");
    case 0x8004E006 : return QStringLiteral("CONTEXT_E_SYNCH_TIMEOUT");
    case 0x8004E007 : return QStringLiteral("CONTEXT_E_OLDREF");
    case 0x8004E00C : return QStringLiteral("CONTEXT_E_ROLENOTFOUND");
    case 0x8004E00F : return QStringLiteral("CONTEXT_E_TMNOTAVAILABLE");
    case 0x8004E021 : return QStringLiteral("CO_E_ACTIVATIONFAILED");
    case 0x8004E022 : return QStringLiteral("CO_E_ACTIVATIONFAILED_EVENTLOGGED");
    case 0x8004E023 : return QStringLiteral("CO_E_ACTIVATIONFAILED_CATALOGERROR");
    case 0x8004E024 : return QStringLiteral("CO_E_ACTIVATIONFAILED_TIMEOUT");
    case 0x8004E025 : return QStringLiteral("CO_E_INITIALIZATIONFAILED");
    case 0x8004E026 : return QStringLiteral("CONTEXT_E_NOJIT");
    case 0x8004E027 : return QStringLiteral("CONTEXT_E_NOTRANSACTION");
    case 0x8004E028 : return QStringLiteral("CO_E_THREADINGMODEL_CHANGED");
    case 0x8004E029 : return QStringLiteral("CO_E_NOIISINTRINSICS");
    case 0x8004E02A : return QStringLiteral("CO_E_NOCOOKIES");
    case 0x8004E02B : return QStringLiteral("CO_E_DBERROR");
    case 0x8004E02C : return QStringLiteral("CO_E_NOTPOOLED");
    case 0x8004E02D : return QStringLiteral("CO_E_NOTCONSTRUCTED");
    case 0x8004E02E : return QStringLiteral("CO_E_NOSYNCHRONIZATION");
    case 0x8004E02F : return QStringLiteral("CO_E_ISOLEVELMISMATCH");
    case 0x00040000 : return QStringLiteral("OLE_S_USEREG");
    case 0x00040001 : return QStringLiteral("OLE_S_STATIC");
    case 0x00040002 : return QStringLiteral("OLE_S_MAC_CLIPFORMAT");
    case 0x00040100 : return QStringLiteral("DRAGDROP_S_DROP");
    case 0x00040101 : return QStringLiteral("DRAGDROP_S_CANCEL");
    case 0x00040102 : return QStringLiteral("DRAGDROP_S_USEDEFAULTCURSORS");
    case 0x00040130 : return QStringLiteral("DATA_S_SAMEFORMATETC");
    case 0x00040140 : return QStringLiteral("VIEW_S_ALREADY_FROZEN");
    case 0x00040170 : return QStringLiteral("CACHE_S_FORMATETC_NOTSUPPORTED");
    case 0x00040171 : return QStringLiteral("CACHE_S_SAMECACHE");
    case 0x00040172 : return QStringLiteral("CACHE_S_SOMECACHES_NOTUPDATED");
    case 0x00040180 : return QStringLiteral("OLEOBJ_S_INVALIDVERB");
    case 0x00040181 : return QStringLiteral("OLEOBJ_S_CANNOT_DOVERB_NOW");
    case 0x00040182 : return QStringLiteral("OLEOBJ_S_INVALIDHWND");
    case 0x000401A0 : return QStringLiteral("INPLACE_S_TRUNCATED");
    case 0x000401C0 : return QStringLiteral("CONVERT10_S_NO_PRESENTATION");
    case 0x000401E2 : return QStringLiteral("MK_S_REDUCED_TO_SELF");
    case 0x000401E4 : return QStringLiteral("MK_S_ME");
    case 0x000401E5 : return QStringLiteral("MK_S_HIM");
    case 0x000401E6 : return QStringLiteral("MK_S_US");
    case 0x000401E7 : return QStringLiteral("MK_S_MONIKERALREADYREGISTERED");
    case 0x00041300 : return QStringLiteral("SCHED_S_TASK_READY");
    case 0x00041301 : return QStringLiteral("SCHED_S_TASK_RUNNING");
    case 0x00041302 : return QStringLiteral("SCHED_S_TASK_DISABLED");
    case 0x00041303 : return QStringLiteral("SCHED_S_TASK_HAS_NOT_RUN");
    case 0x00041304 : return QStringLiteral("SCHED_S_TASK_NO_MORE_RUNS");
    case 0x00041305 : return QStringLiteral("SCHED_S_TASK_NOT_SCHEDULED");
    case 0x00041306 : return QStringLiteral("SCHED_S_TASK_TERMINATED");
    case 0x00041307 : return QStringLiteral("SCHED_S_TASK_NO_VALID_TRIGGERS");
    case 0x00041308 : return QStringLiteral("SCHED_S_EVENT_TRIGGER");
    case 0x80041309 : return QStringLiteral("SCHED_E_TRIGGER_NOT_FOUND");
    case 0x8004130A : return QStringLiteral("SCHED_E_TASK_NOT_READY");
    case 0x8004130B : return QStringLiteral("SCHED_E_TASK_NOT_RUNNING");
    case 0x8004130C : return QStringLiteral("SCHED_E_SERVICE_NOT_INSTALLED");
    case 0x8004130D : return QStringLiteral("SCHED_E_CANNOT_OPEN_TASK");
    case 0x8004130E : return QStringLiteral("SCHED_E_INVALID_TASK");
    case 0x8004130F : return QStringLiteral("SCHED_E_ACCOUNT_INFORMATION_NOT_SET");
    case 0x80041310 : return QStringLiteral("SCHED_E_ACCOUNT_NAME_NOT_FOUND");
    case 0x80041311 : return QStringLiteral("SCHED_E_ACCOUNT_DBASE_CORRUPT");
    case 0x80041312 : return QStringLiteral("SCHED_E_NO_SECURITY_SERVICES");
    case 0x80041313 : return QStringLiteral("SCHED_E_UNKNOWN_OBJECT_VERSION");
    case 0x80041314 : return QStringLiteral("SCHED_E_UNSUPPORTED_ACCOUNT_OPTION");
    case 0x80041315 : return QStringLiteral("SCHED_E_SERVICE_NOT_RUNNING");
    case 0x80080001 : return QStringLiteral("CO_E_CLASS_CREATE_FAILED");
    case 0x80080002 : return QStringLiteral("CO_E_SCM_ERROR");
    case 0x80080003 : return QStringLiteral("CO_E_SCM_RPC_FAILURE");
    case 0x80080004 : return QStringLiteral("CO_E_BAD_PATH");
    case 0x80080005 : return QStringLiteral("CO_E_SERVER_EXEC_FAILURE");
    case 0x80080006 : return QStringLiteral("CO_E_OBJSRV_RPC_FAILURE");
    case 0x80080007 : return QStringLiteral("MK_E_NO_NORMALIZED");
    case 0x80080008 : return QStringLiteral("CO_E_SERVER_STOPPING");
    case 0x80080009 : return QStringLiteral("MEM_E_INVALID_ROOT");
    case 0x80080010 : return QStringLiteral("MEM_E_INVALID_LINK");
    case 0x80080011 : return QStringLiteral("MEM_E_INVALID_SIZE");
    case 0x00080012 : return QStringLiteral("CO_S_NOTALLINTERFACES");
    case 0x00080013 : return QStringLiteral("CO_S_MACHINENAMENOTFOUND");
    case 0x80020001 : return QStringLiteral("DISP_E_UNKNOWNINTERFACE");
    case 0x80020003 : return QStringLiteral("DISP_E_MEMBERNOTFOUND");
    case 0x80020004 : return QStringLiteral("DISP_E_PARAMNOTFOUND");
    case 0x80020005 : return QStringLiteral("DISP_E_TYPEMISMATCH");
    case 0x80020006 : return QStringLiteral("DISP_E_UNKNOWNNAME");
    case 0x80020007 : return QStringLiteral("DISP_E_NONAMEDARGS");
    case 0x80020008 : return QStringLiteral("DISP_E_BADVARTYPE");
    case 0x80020009 : return QStringLiteral("DISP_E_EXCEPTION");
    case 0x8002000A : return QStringLiteral("DISP_E_OVERFLOW");
    case 0x8002000B : return QStringLiteral("DISP_E_BADINDEX");
    case 0x8002000C : return QStringLiteral("DISP_E_UNKNOWNLCID");
    case 0x8002000D : return QStringLiteral("DISP_E_ARRAYISLOCKED");
    case 0x8002000E : return QStringLiteral("DISP_E_BADPARAMCOUNT");
    case 0x8002000F : return QStringLiteral("DISP_E_PARAMNOTOPTIONAL");
    case 0x80020010 : return QStringLiteral("DISP_E_BADCALLEE");
    case 0x80020011 : return QStringLiteral("DISP_E_NOTACOLLECTION");
    case 0x80020012 : return QStringLiteral("DISP_E_DIVBYZERO");
    case 0x80020013 : return QStringLiteral("DISP_E_BUFFERTOOSMALL");
    case 0x80028016 : return QStringLiteral("TYPE_E_BUFFERTOOSMALL");
    case 0x80028017 : return QStringLiteral("TYPE_E_FIELDNOTFOUND");
    case 0x80028018 : return QStringLiteral("TYPE_E_INVDATAREAD");
    case 0x80028019 : return QStringLiteral("TYPE_E_UNSUPFORMAT");
    case 0x8002801C : return QStringLiteral("TYPE_E_REGISTRYACCESS");
    case 0x8002801D : return QStringLiteral("TYPE_E_LIBNOTREGISTERED");
    case 0x80028027 : return QStringLiteral("TYPE_E_UNDEFINEDTYPE");
    case 0x80028028 : return QStringLiteral("TYPE_E_QUALIFIEDNAMEDISALLOWED");
    case 0x80028029 : return QStringLiteral("TYPE_E_INVALIDSTATE");
    case 0x8002802A : return QStringLiteral("TYPE_E_WRONGTYPEKIND");
    case 0x8002802B : return QStringLiteral("TYPE_E_ELEMENTNOTFOUND");
    case 0x8002802C : return QStringLiteral("TYPE_E_AMBIGUOUSNAME");
    case 0x8002802D : return QStringLiteral("TYPE_E_NAMECONFLICT");
    case 0x8002802E : return QStringLiteral("TYPE_E_UNKNOWNLCID");
    case 0x8002802F : return QStringLiteral("TYPE_E_DLLFUNCTIONNOTFOUND");
    case 0x800288BD : return QStringLiteral("TYPE_E_BADMODULEKIND");
    case 0x800288C5 : return QStringLiteral("TYPE_E_SIZETOOBIG");
    case 0x800288C6 : return QStringLiteral("TYPE_E_DUPLICATEID");
    case 0x800288CF : return QStringLiteral("TYPE_E_INVALIDID");
    case 0x80028CA0 : return QStringLiteral("TYPE_E_TYPEMISMATCH");
    case 0x80028CA1 : return QStringLiteral("TYPE_E_OUTOFBOUNDS");
    case 0x80028CA2 : return QStringLiteral("TYPE_E_IOERROR");
    case 0x80028CA3 : return QStringLiteral("TYPE_E_CANTCREATETMPFILE");
    case 0x80029C4A : return QStringLiteral("TYPE_E_CANTLOADLIBRARY");
    case 0x80029C83 : return QStringLiteral("TYPE_E_INCONSISTENTPROPFUNCS");
    case 0x80029C84 : return QStringLiteral("TYPE_E_CIRCULARTYPE");
    case 0x80030001 : return QStringLiteral("STG_E_INVALIDFUNCTION");
    case 0x80030002 : return QStringLiteral("STG_E_FILENOTFOUND");
    case 0x80030003 : return QStringLiteral("STG_E_PATHNOTFOUND");
    case 0x80030004 : return QStringLiteral("STG_E_TOOMANYOPENFILES");
    case 0x80030005 : return QStringLiteral("STG_E_ACCESSDENIED");
    case 0x80030006 : return QStringLiteral("STG_E_INVALIDHANDLE");
    case 0x80030008 : return QStringLiteral("STG_E_INSUFFICIENTMEMORY");
    case 0x80030009 : return QStringLiteral("STG_E_INVALIDPOINTER");
    case 0x80030012 : return QStringLiteral("STG_E_NOMOREFILES");
    case 0x80030013 : return QStringLiteral("STG_E_DISKISWRITEPROTECTED");
    case 0x80030019 : return QStringLiteral("STG_E_SEEKERROR");
    case 0x8003001D : return QStringLiteral("STG_E_WRITEFAULT");
    case 0x8003001E : return QStringLiteral("STG_E_READFAULT");
    case 0x80030020 : return QStringLiteral("STG_E_SHAREVIOLATION");
    case 0x80030021 : return QStringLiteral("STG_E_LOCKVIOLATION");
    case 0x80030050 : return QStringLiteral("STG_E_FILEALREADYEXISTS");
    case 0x80030057 : return QStringLiteral("STG_E_INVALIDPARAMETER");
    case 0x80030070 : return QStringLiteral("STG_E_MEDIUMFULL");
    case 0x800300F0 : return QStringLiteral("STG_E_PROPSETMISMATCHED");
    case 0x800300FA : return QStringLiteral("STG_E_ABNORMALAPIEXIT");
    case 0x800300FB : return QStringLiteral("STG_E_INVALIDHEADER");
    case 0x800300FC : return QStringLiteral("STG_E_INVALIDNAME");
    case 0x800300FD : return QStringLiteral("STG_E_UNKNOWN");
    case 0x800300FE : return QStringLiteral("STG_E_UNIMPLEMENTEDFUNCTION");
    case 0x800300FF : return QStringLiteral("STG_E_INVALIDFLAG");
    case 0x80030100 : return QStringLiteral("STG_E_INUSE");
    case 0x80030101 : return QStringLiteral("STG_E_NOTCURRENT");
    case 0x80030102 : return QStringLiteral("STG_E_REVERTED");
    case 0x80030103 : return QStringLiteral("STG_E_CANTSAVE");
    case 0x80030104 : return QStringLiteral("STG_E_OLDFORMAT");
    case 0x80030105 : return QStringLiteral("STG_E_OLDDLL");
    case 0x80030106 : return QStringLiteral("STG_E_SHAREREQUIRED");
    case 0x80030107 : return QStringLiteral("STG_E_NOTFILEBASEDSTORAGE");
    case 0x80030108 : return QStringLiteral("STG_E_EXTANTMARSHALLINGS");
    case 0x80030109 : return QStringLiteral("STG_E_DOCFILECORRUPT");
    case 0x80030110 : return QStringLiteral("STG_E_BADBASEADDRESS");
    case 0x80030111 : return QStringLiteral("STG_E_DOCFILETOOLARGE");
    case 0x80030112 : return QStringLiteral("STG_E_NOTSIMPLEFORMAT");
    case 0x80030201 : return QStringLiteral("STG_E_INCOMPLETE");
    case 0x80030202 : return QStringLiteral("STG_E_TERMINATED");
    case 0x00030200 : return QStringLiteral("STG_S_CONVERTED");
    case 0x00030201 : return QStringLiteral("STG_S_BLOCK");
    case 0x00030202 : return QStringLiteral("STG_S_RETRYNOW");
    case 0x00030203 : return QStringLiteral("STG_S_MONITORING");
    case 0x00030204 : return QStringLiteral("STG_S_MULTIPLEOPENS");
    case 0x00030205 : return QStringLiteral("STG_S_CONSOLIDATIONFAILED");
    case 0x00030206 : return QStringLiteral("STG_S_CANNOTCONSOLIDATE");
    case 0x80030305 : return QStringLiteral("STG_E_STATUS_COPY_PROTECTION_FAILURE");
    case 0x80030306 : return QStringLiteral("STG_E_CSS_AUTHENTICATION_FAILURE");
    case 0x80030307 : return QStringLiteral("STG_E_CSS_KEY_NOT_PRESENT");
    case 0x80030308 : return QStringLiteral("STG_E_CSS_KEY_NOT_ESTABLISHED");
    case 0x80030309 : return QStringLiteral("STG_E_CSS_SCRAMBLED_SECTOR");
    case 0x8003030A : return QStringLiteral("STG_E_CSS_REGION_MISMATCH");
    case 0x8003030B : return QStringLiteral("STG_E_RESETS_EXHAUSTED");
    case 0x80010001 : return QStringLiteral("RPC_E_CALL_REJECTED");
    case 0x80010002 : return QStringLiteral("RPC_E_CALL_CANCELED");
    case 0x80010003 : return QStringLiteral("RPC_E_CANTPOST_INSENDCALL");
    case 0x80010004 : return QStringLiteral("RPC_E_CANTCALLOUT_INASYNCCALL");
    case 0x80010005 : return QStringLiteral("RPC_E_CANTCALLOUT_INEXTERNALCALL");
    case 0x80010006 : return QStringLiteral("RPC_E_CONNECTION_TERMINATED");
    case 0x80010007 : return QStringLiteral("RPC_E_SERVER_DIED");
    case 0x80010008 : return QStringLiteral("RPC_E_CLIENT_DIED");
    case 0x80010009 : return QStringLiteral("RPC_E_INVALID_DATAPACKET");
    case 0x8001000A : return QStringLiteral("RPC_E_CANTTRANSMIT_CALL");
    case 0x8001000B : return QStringLiteral("RPC_E_CLIENT_CANTMARSHAL_DATA");
    case 0x8001000C : return QStringLiteral("RPC_E_CLIENT_CANTUNMARSHAL_DATA");
    case 0x8001000D : return QStringLiteral("RPC_E_SERVER_CANTMARSHAL_DATA");
    case 0x8001000E : return QStringLiteral("RPC_E_SERVER_CANTUNMARSHAL_DATA");
    case 0x8001000F : return QStringLiteral("RPC_E_INVALID_DATA");
    case 0x80010010 : return QStringLiteral("RPC_E_INVALID_PARAMETER");
    case 0x80010011 : return QStringLiteral("RPC_E_CANTCALLOUT_AGAIN");
    case 0x80010012 : return QStringLiteral("RPC_E_SERVER_DIED_DNE");
    case 0x80010100 : return QStringLiteral("RPC_E_SYS_CALL_FAILED");
    case 0x80010101 : return QStringLiteral("RPC_E_OUT_OF_RESOURCES");
    case 0x80010102 : return QStringLiteral("RPC_E_ATTEMPTED_MULTITHREAD");
    case 0x80010103 : return QStringLiteral("RPC_E_NOT_REGISTERED");
    case 0x80010104 : return QStringLiteral("RPC_E_FAULT");
    case 0x80010105 : return QStringLiteral("RPC_E_SERVERFAULT");
    case 0x80010106 : return QStringLiteral("RPC_E_CHANGED_MODE");
    case 0x80010107 : return QStringLiteral("RPC_E_INVALIDMETHOD");
    case 0x80010108 : return QStringLiteral("RPC_E_DISCONNECTED");
    case 0x80010109 : return QStringLiteral("RPC_E_RETRY");
    case 0x8001010A : return QStringLiteral("RPC_E_SERVERCALL_RETRYLATER");
    case 0x8001010B : return QStringLiteral("RPC_E_SERVERCALL_REJECTED");
    case 0x8001010C : return QStringLiteral("RPC_E_INVALID_CALLDATA");
    case 0x8001010D : return QStringLiteral("RPC_E_CANTCALLOUT_ININPUTSYNCCALL");
    case 0x8001010E : return QStringLiteral("RPC_E_WRONG_THREAD");
    case 0x8001010F : return QStringLiteral("RPC_E_THREAD_NOT_INIT");
    case 0x80010110 : return QStringLiteral("RPC_E_VERSION_MISMATCH");
    case 0x80010111 : return QStringLiteral("RPC_E_INVALID_HEADER");
    case 0x80010112 : return QStringLiteral("RPC_E_INVALID_EXTENSION");
    case 0x80010113 : return QStringLiteral("RPC_E_INVALID_IPID");
    case 0x80010114 : return QStringLiteral("RPC_E_INVALID_OBJECT");
    case 0x80010115 : return QStringLiteral("RPC_S_CALLPENDING");
    case 0x80010116 : return QStringLiteral("RPC_S_WAITONTIMER");
    case 0x80010117 : return QStringLiteral("RPC_E_CALL_COMPLETE");
    case 0x80010118 : return QStringLiteral("RPC_E_UNSECURE_CALL");
    case 0x80010119 : return QStringLiteral("RPC_E_TOO_LATE");
    case 0x8001011A : return QStringLiteral("RPC_E_NO_GOOD_SECURITY_PACKAGES");
    case 0x8001011B : return QStringLiteral("RPC_E_ACCESS_DENIED");
    case 0x8001011C : return QStringLiteral("RPC_E_REMOTE_DISABLED");
    case 0x8001011D : return QStringLiteral("RPC_E_INVALID_OBJREF");
    case 0x8001011E : return QStringLiteral("RPC_E_NO_CONTEXT");
    case 0x8001011F : return QStringLiteral("RPC_E_TIMEOUT");
    case 0x80010120 : return QStringLiteral("RPC_E_NO_SYNC");
    case 0x80010121 : return QStringLiteral("RPC_E_FULLSIC_REQUIRED");
    case 0x80010122 : return QStringLiteral("RPC_E_INVALID_STD_NAME");
    case 0x80010123 : return QStringLiteral("CO_E_FAILEDTOIMPERSONATE");
    case 0x80010124 : return QStringLiteral("CO_E_FAILEDTOGETSECCTX");
    case 0x80010125 : return QStringLiteral("CO_E_FAILEDTOOPENTHREADTOKEN");
    case 0x80010126 : return QStringLiteral("CO_E_FAILEDTOGETTOKENINFO");
    case 0x80010127 : return QStringLiteral("CO_E_TRUSTEEDOESNTMATCHCLIENT");
    case 0x80010128 : return QStringLiteral("CO_E_FAILEDTOQUERYCLIENTBLANKET");
    case 0x80010129 : return QStringLiteral("CO_E_FAILEDTOSETDACL");
    case 0x8001012A : return QStringLiteral("CO_E_ACCESSCHECKFAILED");
    case 0x8001012B : return QStringLiteral("CO_E_NETACCESSAPIFAILED");
    case 0x8001012C : return QStringLiteral("CO_E_WRONGTRUSTEENAMESYNTAX");
    case 0x8001012D : return QStringLiteral("CO_E_INVALIDSID");
    case 0x8001012E : return QStringLiteral("CO_E_CONVERSIONFAILED");
    case 0x8001012F : return QStringLiteral("CO_E_NOMATCHINGSIDFOUND");
    case 0x80010130 : return QStringLiteral("CO_E_LOOKUPACCSIDFAILED");
    case 0x80010131 : return QStringLiteral("CO_E_NOMATCHINGNAMEFOUND");
    case 0x80010132 : return QStringLiteral("CO_E_LOOKUPACCNAMEFAILED");
    case 0x80010133 : return QStringLiteral("CO_E_SETSERLHNDLFAILED");
    case 0x80010134 : return QStringLiteral("CO_E_FAILEDTOGETWINDIR");
    case 0x80010135 : return QStringLiteral("CO_E_PATHTOOLONG");
    case 0x80010136 : return QStringLiteral("CO_E_FAILEDTOGENUUID");
    case 0x80010137 : return QStringLiteral("CO_E_FAILEDTOCREATEFILE");
    case 0x80010138 : return QStringLiteral("CO_E_FAILEDTOCLOSEHANDLE");
    case 0x80010139 : return QStringLiteral("CO_E_EXCEEDSYSACLLIMIT");
    case 0x8001013A : return QStringLiteral("CO_E_ACESINWRONGORDER");
    case 0x8001013B : return QStringLiteral("CO_E_INCOMPATIBLESTREAMVERSION");
    case 0x8001013C : return QStringLiteral("CO_E_FAILEDTOOPENPROCESSTOKEN");
    case 0x8001013D : return QStringLiteral("CO_E_DECODEFAILED");
    case 0x8001013F : return QStringLiteral("CO_E_ACNOTINITIALIZED");
    case 0x80010140 : return QStringLiteral("CO_E_CANCEL_DISABLED");
    case 0x8001FFFF : return QStringLiteral("RPC_E_UNEXPECTED");
    case 0xC0090001 : return QStringLiteral("ERROR_AUDITING_DISABLED");
    case 0xC0090002 : return QStringLiteral("ERROR_ALL_SIDS_FILTERED");
    case 0x80090001 : return QStringLiteral("NTE_BAD_UID");
    case 0x80090002 : return QStringLiteral("NTE_BAD_HASH");
    case 0x80090003 : return QStringLiteral("NTE_BAD_KEY");
    case 0x80090004 : return QStringLiteral("NTE_BAD_LEN");
    case 0x80090005 : return QStringLiteral("NTE_BAD_DATA");
    case 0x80090006 : return QStringLiteral("NTE_BAD_SIGNATURE");
    case 0x80090007 : return QStringLiteral("NTE_BAD_VER");
    case 0x80090008 : return QStringLiteral("NTE_BAD_ALGID");
    case 0x80090009 : return QStringLiteral("NTE_BAD_FLAGS");
    case 0x8009000A : return QStringLiteral("NTE_BAD_TYPE");
    case 0x8009000B : return QStringLiteral("NTE_BAD_KEY_STATE");
    case 0x8009000C : return QStringLiteral("NTE_BAD_HASH_STATE");
    case 0x8009000D : return QStringLiteral("NTE_NO_KEY");
    case 0x8009000E : return QStringLiteral("NTE_NO_MEMORY");
    case 0x8009000F : return QStringLiteral("NTE_EXISTS");
    case 0x80090010 : return QStringLiteral("NTE_PERM");
    case 0x80090011 : return QStringLiteral("NTE_NOT_FOUND");
    case 0x80090012 : return QStringLiteral("NTE_DOUBLE_ENCRYPT");
    case 0x80090013 : return QStringLiteral("NTE_BAD_PROVIDER");
    case 0x80090014 : return QStringLiteral("NTE_BAD_PROV_TYPE");
    case 0x80090015 : return QStringLiteral("NTE_BAD_PUBLIC_KEY");
    case 0x80090016 : return QStringLiteral("NTE_BAD_KEYSET");
    case 0x80090017 : return QStringLiteral("NTE_PROV_TYPE_NOT_DEF");
    case 0x80090018 : return QStringLiteral("NTE_PROV_TYPE_ENTRY_BAD");
    case 0x80090019 : return QStringLiteral("NTE_KEYSET_NOT_DEF");
    case 0x8009001A : return QStringLiteral("NTE_KEYSET_ENTRY_BAD");
    case 0x8009001B : return QStringLiteral("NTE_PROV_TYPE_NO_MATCH");
    case 0x8009001C : return QStringLiteral("NTE_SIGNATURE_FILE_BAD");
    case 0x8009001D : return QStringLiteral("NTE_PROVIDER_DLL_FAIL");
    case 0x8009001E : return QStringLiteral("NTE_PROV_DLL_NOT_FOUND");
    case 0x8009001F : return QStringLiteral("NTE_BAD_KEYSET_PARAM");
    case 0x80090020 : return QStringLiteral("NTE_FAIL");
    case 0x80090021 : return QStringLiteral("NTE_SYS_ERR");
    case 0x80090022 : return QStringLiteral("NTE_SILENT_CONTEXT");
    case 0x80090023 : return QStringLiteral("NTE_TOKEN_KEYSET_STORAGE_FULL");
    case 0x80090024 : return QStringLiteral("NTE_TEMPORARY_PROFILE");
    case 0x80090025 : return QStringLiteral("NTE_FIXEDPARAMETER");
    case 0x80090300 : return QStringLiteral("SEC_E_INSUFFICIENT_MEMORY");
    case 0x80090301 : return QStringLiteral("SEC_E_INVALID_HANDLE");
    case 0x80090302 : return QStringLiteral("SEC_E_UNSUPPORTED_FUNCTION");
    case 0x80090303 : return QStringLiteral("SEC_E_TARGET_UNKNOWN");
    case 0x80090304 : return QStringLiteral("SEC_E_INTERNAL_ERROR");
    case 0x80090305 : return QStringLiteral("SEC_E_SECPKG_NOT_FOUND");
    case 0x80090306 : return QStringLiteral("SEC_E_NOT_OWNER");
    case 0x80090307 : return QStringLiteral("SEC_E_CANNOT_INSTALL");
    case 0x80090308 : return QStringLiteral("SEC_E_INVALID_TOKEN");
    case 0x80090309 : return QStringLiteral("SEC_E_CANNOT_PACK");
    case 0x8009030A : return QStringLiteral("SEC_E_QOP_NOT_SUPPORTED");
    case 0x8009030B : return QStringLiteral("SEC_E_NO_IMPERSONATION");
    case 0x8009030C : return QStringLiteral("SEC_E_LOGON_DENIED");
    case 0x8009030D : return QStringLiteral("SEC_E_UNKNOWN_CREDENTIALS");
    case 0x8009030E : return QStringLiteral("SEC_E_NO_CREDENTIALS");
    case 0x8009030F : return QStringLiteral("SEC_E_MESSAGE_ALTERED");
    case 0x80090310 : return QStringLiteral("SEC_E_OUT_OF_SEQUENCE");
    case 0x80090311 : return QStringLiteral("SEC_E_NO_AUTHENTICATING_AUTHORITY");
    case 0x00090312 : return QStringLiteral("SEC_I_CONTINUE_NEEDED");
    case 0x00090313 : return QStringLiteral("SEC_I_COMPLETE_NEEDED");
    case 0x00090314 : return QStringLiteral("SEC_I_COMPLETE_AND_CONTINUE");
    case 0x00090315 : return QStringLiteral("SEC_I_LOCAL_LOGON");
    case 0x80090316 : return QStringLiteral("SEC_E_BAD_PKGID");
    case 0x80090317 : return QStringLiteral("SEC_E_CONTEXT_EXPIRED");
    case 0x00090317 : return QStringLiteral("SEC_I_CONTEXT_EXPIRED");
    case 0x80090318 : return QStringLiteral("SEC_E_INCOMPLETE_MESSAGE");
    case 0x80090320 : return QStringLiteral("SEC_E_INCOMPLETE_CREDENTIALS");
    case 0x80090321 : return QStringLiteral("SEC_E_BUFFER_TOO_SMALL");
    case 0x00090320 : return QStringLiteral("SEC_I_INCOMPLETE_CREDENTIALS");
    case 0x00090321 : return QStringLiteral("SEC_I_RENEGOTIATE");
    case 0x80090322 : return QStringLiteral("SEC_E_WRONG_PRINCIPAL");
    case 0x00090323 : return QStringLiteral("SEC_I_NO_LSA_CONTEXT");
    case 0x80090324 : return QStringLiteral("SEC_E_TIME_SKEW");
    case 0x80090325 : return QStringLiteral("SEC_E_UNTRUSTED_ROOT");
    case 0x80090326 : return QStringLiteral("SEC_E_ILLEGAL_MESSAGE");
    case 0x80090327 : return QStringLiteral("SEC_E_CERT_UNKNOWN");
    case 0x80090328 : return QStringLiteral("SEC_E_CERT_EXPIRED");
    case 0x80090329 : return QStringLiteral("SEC_E_ENCRYPT_FAILURE");
    case 0x80090330 : return QStringLiteral("SEC_E_DECRYPT_FAILURE");
    case 0x80090331 : return QStringLiteral("SEC_E_ALGORITHM_MISMATCH");
    case 0x80090332 : return QStringLiteral("SEC_E_SECURITY_QOS_FAILED");
    case 0x80090333 : return QStringLiteral("SEC_E_UNFINISHED_CONTEXT_DELETED");
    case 0x80090334 : return QStringLiteral("SEC_E_NO_TGT_REPLY");
    case 0x80090335 : return QStringLiteral("SEC_E_NO_IP_ADDRESSES");
    case 0x80090336 : return QStringLiteral("SEC_E_WRONG_CREDENTIAL_HANDLE");
    case 0x80090337 : return QStringLiteral("SEC_E_CRYPTO_SYSTEM_INVALID");
    case 0x80090338 : return QStringLiteral("SEC_E_MAX_REFERRALS_EXCEEDED");
    case 0x80090339 : return QStringLiteral("SEC_E_MUST_BE_KDC");
    case 0x8009033A : return QStringLiteral("SEC_E_STRONG_CRYPTO_NOT_SUPPORTED");
    case 0x8009033B : return QStringLiteral("SEC_E_TOO_MANY_PRINCIPALS");
    case 0x8009033C : return QStringLiteral("SEC_E_NO_PA_DATA");
    case 0x8009033D : return QStringLiteral("SEC_E_PKINIT_NAME_MISMATCH");
    case 0x8009033E : return QStringLiteral("SEC_E_SMARTCARD_LOGON_REQUIRED");
    case 0x8009033F : return QStringLiteral("SEC_E_SHUTDOWN_IN_PROGRESS");
    case 0x80090340 : return QStringLiteral("SEC_E_KDC_INVALID_REQUEST");
    case 0x80090341 : return QStringLiteral("SEC_E_KDC_UNABLE_TO_REFER");
    case 0x80090342 : return QStringLiteral("SEC_E_KDC_UNKNOWN_ETYPE");
    case 0x80090343 : return QStringLiteral("SEC_E_UNSUPPORTED_PREAUTH");
    case 0x80090345 : return QStringLiteral("SEC_E_DELEGATION_REQUIRED");
    case 0x80090346 : return QStringLiteral("SEC_E_BAD_BINDINGS");
    case 0x80090347 : return QStringLiteral("SEC_E_MULTIPLE_ACCOUNTS");
    case 0x80090348 : return QStringLiteral("SEC_E_NO_KERB_KEY");
    case 0x80090349 : return QStringLiteral("SEC_E_CERT_WRONG_USAGE");
    case 0x80090350 : return QStringLiteral("SEC_E_DOWNGRADE_DETECTED");
    case 0x80090351 : return QStringLiteral("SEC_E_SMARTCARD_CERT_REVOKED");
    case 0x80090352 : return QStringLiteral("SEC_E_ISSUING_CA_UNTRUSTED");
    case 0x80090353 : return QStringLiteral("SEC_E_REVOCATION_OFFLINE_C");
    case 0x80090354 : return QStringLiteral("SEC_E_PKINIT_CLIENT_FAILURE");
    case 0x80090355 : return QStringLiteral("SEC_E_SMARTCARD_CERT_EXPIRED");
    case 0x80090356 : return QStringLiteral("SEC_E_NO_S4U_PROT_SUPPORT");
    case 0x80090357 : return QStringLiteral("SEC_E_CROSSREALM_DELEGATION_FAILURE");
    case 0x80090358 : return QStringLiteral("SEC_E_REVOCATION_OFFLINE_KDC");
    case 0x80090359 : return QStringLiteral("SEC_E_ISSUING_CA_UNTRUSTED_KDC");
    case 0x8009035A : return QStringLiteral("SEC_E_KDC_CERT_EXPIRED");
    case 0x8009035B : return QStringLiteral("SEC_E_KDC_CERT_REVOKED");
    case 0x80091001 : return QStringLiteral("CRYPT_E_MSG_ERROR");
    case 0x80091002 : return QStringLiteral("CRYPT_E_UNKNOWN_ALGO");
    case 0x80091003 : return QStringLiteral("CRYPT_E_OID_FORMAT");
    case 0x80091004 : return QStringLiteral("CRYPT_E_INVALID_MSG_TYPE");
    case 0x80091005 : return QStringLiteral("CRYPT_E_UNEXPECTED_ENCODING");
    case 0x80091006 : return QStringLiteral("CRYPT_E_AUTH_ATTR_MISSING");
    case 0x80091007 : return QStringLiteral("CRYPT_E_HASH_VALUE");
    case 0x80091008 : return QStringLiteral("CRYPT_E_INVALID_INDEX");
    case 0x80091009 : return QStringLiteral("CRYPT_E_ALREADY_DECRYPTED");
    case 0x8009100A : return QStringLiteral("CRYPT_E_NOT_DECRYPTED");
    case 0x8009100B : return QStringLiteral("CRYPT_E_RECIPIENT_NOT_FOUND");
    case 0x8009100C : return QStringLiteral("CRYPT_E_CONTROL_TYPE");
    case 0x8009100D : return QStringLiteral("CRYPT_E_ISSUER_SERIALNUMBER");
    case 0x8009100E : return QStringLiteral("CRYPT_E_SIGNER_NOT_FOUND");
    case 0x8009100F : return QStringLiteral("CRYPT_E_ATTRIBUTES_MISSING");
    case 0x80091010 : return QStringLiteral("CRYPT_E_STREAM_MSG_NOT_READY");
    case 0x80091011 : return QStringLiteral("CRYPT_E_STREAM_INSUFFICIENT_DATA");
    case 0x00091012 : return QStringLiteral("CRYPT_I_NEW_PROTECTION_REQUIRED");
    case 0x80092001 : return QStringLiteral("CRYPT_E_BAD_LEN");
    case 0x80092002 : return QStringLiteral("CRYPT_E_BAD_ENCODE");
    case 0x80092003 : return QStringLiteral("CRYPT_E_FILE_ERROR");
    case 0x80092004 : return QStringLiteral("CRYPT_E_NOT_FOUND");
    case 0x80092005 : return QStringLiteral("CRYPT_E_EXISTS");
    case 0x80092006 : return QStringLiteral("CRYPT_E_NO_PROVIDER");
    case 0x80092007 : return QStringLiteral("CRYPT_E_SELF_SIGNED");
    case 0x80092008 : return QStringLiteral("CRYPT_E_DELETED_PREV");
    case 0x80092009 : return QStringLiteral("CRYPT_E_NO_MATCH");
    case 0x8009200A : return QStringLiteral("CRYPT_E_UNEXPECTED_MSG_TYPE");
    case 0x8009200B : return QStringLiteral("CRYPT_E_NO_KEY_PROPERTY");
    case 0x8009200C : return QStringLiteral("CRYPT_E_NO_DECRYPT_CERT");
    case 0x8009200D : return QStringLiteral("CRYPT_E_BAD_MSG");
    case 0x8009200E : return QStringLiteral("CRYPT_E_NO_SIGNER");
    case 0x8009200F : return QStringLiteral("CRYPT_E_PENDING_CLOSE");
    case 0x80092010 : return QStringLiteral("CRYPT_E_REVOKED");
    case 0x80092011 : return QStringLiteral("CRYPT_E_NO_REVOCATION_DLL");
    case 0x80092012 : return QStringLiteral("CRYPT_E_NO_REVOCATION_CHECK");
    case 0x80092013 : return QStringLiteral("CRYPT_E_REVOCATION_OFFLINE");
    case 0x80092014 : return QStringLiteral("CRYPT_E_NOT_IN_REVOCATION_DATABASE");
    case 0x80092020 : return QStringLiteral("CRYPT_E_INVALID_NUMERIC_STRING");
    case 0x80092021 : return QStringLiteral("CRYPT_E_INVALID_PRINTABLE_STRING");
    case 0x80092022 : return QStringLiteral("CRYPT_E_INVALID_IA5_STRING");
    case 0x80092023 : return QStringLiteral("CRYPT_E_INVALID_X500_STRING");
    case 0x80092024 : return QStringLiteral("CRYPT_E_NOT_CHAR_STRING");
    case 0x80092025 : return QStringLiteral("CRYPT_E_FILERESIZED");
    case 0x80092026 : return QStringLiteral("CRYPT_E_SECURITY_SETTINGS");
    case 0x80092027 : return QStringLiteral("CRYPT_E_NO_VERIFY_USAGE_DLL");
    case 0x80092028 : return QStringLiteral("CRYPT_E_NO_VERIFY_USAGE_CHECK");
    case 0x80092029 : return QStringLiteral("CRYPT_E_VERIFY_USAGE_OFFLINE");
    case 0x8009202A : return QStringLiteral("CRYPT_E_NOT_IN_CTL");
    case 0x8009202B : return QStringLiteral("CRYPT_E_NO_TRUSTED_SIGNER");
    case 0x8009202C : return QStringLiteral("CRYPT_E_MISSING_PUBKEY_PARA");
    case 0x80093000 : return QStringLiteral("CRYPT_E_OSS_ERROR");
    case 0x80093001 : return QStringLiteral("OSS_MORE_BUF");
    case 0x80093002 : return QStringLiteral("OSS_NEGATIVE_UINTEGER");
    case 0x80093003 : return QStringLiteral("OSS_PDU_RANGE");
    case 0x80093004 : return QStringLiteral("OSS_MORE_INPUT");
    case 0x80093005 : return QStringLiteral("OSS_DATA_ERROR");
    case 0x80093006 : return QStringLiteral("OSS_BAD_ARG");
    case 0x80093007 : return QStringLiteral("OSS_BAD_VERSION");
    case 0x80093008 : return QStringLiteral("OSS_OUT_MEMORY");
    case 0x80093009 : return QStringLiteral("OSS_PDU_MISMATCH");
    case 0x8009300A : return QStringLiteral("OSS_LIMITED");
    case 0x8009300B : return QStringLiteral("OSS_BAD_PTR");
    case 0x8009300C : return QStringLiteral("OSS_BAD_TIME");
    case 0x8009300D : return QStringLiteral("OSS_INDEFINITE_NOT_SUPPORTED");
    case 0x8009300E : return QStringLiteral("OSS_MEM_ERROR");
    case 0x8009300F : return QStringLiteral("OSS_BAD_TABLE");
    case 0x80093010 : return QStringLiteral("OSS_TOO_LONG");
    case 0x80093011 : return QStringLiteral("OSS_CONSTRAINT_VIOLATED");
    case 0x80093012 : return QStringLiteral("OSS_FATAL_ERROR");
    case 0x80093013 : return QStringLiteral("OSS_ACCESS_SERIALIZATION_ERROR");
    case 0x80093014 : return QStringLiteral("OSS_NULL_TBL");
    case 0x80093015 : return QStringLiteral("OSS_NULL_FCN");
    case 0x80093016 : return QStringLiteral("OSS_BAD_ENCRULES");
    case 0x80093017 : return QStringLiteral("OSS_UNAVAIL_ENCRULES");
    case 0x80093018 : return QStringLiteral("OSS_CANT_OPEN_TRACE_WINDOW");
    case 0x80093019 : return QStringLiteral("OSS_UNIMPLEMENTED");
    case 0x8009301A : return QStringLiteral("OSS_OID_DLL_NOT_LINKED");
    case 0x8009301B : return QStringLiteral("OSS_CANT_OPEN_TRACE_FILE");
    case 0x8009301C : return QStringLiteral("OSS_TRACE_FILE_ALREADY_OPEN");
    case 0x8009301D : return QStringLiteral("OSS_TABLE_MISMATCH");
    case 0x8009301E : return QStringLiteral("OSS_TYPE_NOT_SUPPORTED");
    case 0x8009301F : return QStringLiteral("OSS_REAL_DLL_NOT_LINKED");
    case 0x80093020 : return QStringLiteral("OSS_REAL_CODE_NOT_LINKED");
    case 0x80093021 : return QStringLiteral("OSS_OUT_OF_RANGE");
    case 0x80093022 : return QStringLiteral("OSS_COPIER_DLL_NOT_LINKED");
    case 0x80093023 : return QStringLiteral("OSS_CONSTRAINT_DLL_NOT_LINKED");
    case 0x80093024 : return QStringLiteral("OSS_COMPARATOR_DLL_NOT_LINKED");
    case 0x80093025 : return QStringLiteral("OSS_COMPARATOR_CODE_NOT_LINKED");
    case 0x80093026 : return QStringLiteral("OSS_MEM_MGR_DLL_NOT_LINKED");
    case 0x80093027 : return QStringLiteral("OSS_PDV_DLL_NOT_LINKED");
    case 0x80093028 : return QStringLiteral("OSS_PDV_CODE_NOT_LINKED");
    case 0x80093029 : return QStringLiteral("OSS_API_DLL_NOT_LINKED");
    case 0x8009302A : return QStringLiteral("OSS_BERDER_DLL_NOT_LINKED");
    case 0x8009302B : return QStringLiteral("OSS_PER_DLL_NOT_LINKED");
    case 0x8009302C : return QStringLiteral("OSS_OPEN_TYPE_ERROR");
    case 0x8009302D : return QStringLiteral("OSS_MUTEX_NOT_CREATED");
    case 0x8009302E : return QStringLiteral("OSS_CANT_CLOSE_TRACE_FILE");
    case 0x80093100 : return QStringLiteral("CRYPT_E_ASN1_ERROR");
    case 0x80093101 : return QStringLiteral("CRYPT_E_ASN1_INTERNAL");
    case 0x80093102 : return QStringLiteral("CRYPT_E_ASN1_EOD");
    case 0x80093103 : return QStringLiteral("CRYPT_E_ASN1_CORRUPT");
    case 0x80093104 : return QStringLiteral("CRYPT_E_ASN1_LARGE");
    case 0x80093105 : return QStringLiteral("CRYPT_E_ASN1_CONSTRAINT");
    case 0x80093106 : return QStringLiteral("CRYPT_E_ASN1_MEMORY");
    case 0x80093107 : return QStringLiteral("CRYPT_E_ASN1_OVERFLOW");
    case 0x80093108 : return QStringLiteral("CRYPT_E_ASN1_BADPDU");
    case 0x80093109 : return QStringLiteral("CRYPT_E_ASN1_BADARGS");
    case 0x8009310A : return QStringLiteral("CRYPT_E_ASN1_BADREAL");
    case 0x8009310B : return QStringLiteral("CRYPT_E_ASN1_BADTAG");
    case 0x8009310C : return QStringLiteral("CRYPT_E_ASN1_CHOICE");
    case 0x8009310D : return QStringLiteral("CRYPT_E_ASN1_RULE");
    case 0x8009310E : return QStringLiteral("CRYPT_E_ASN1_UTF8");
    case 0x80093133 : return QStringLiteral("CRYPT_E_ASN1_PDU_TYPE");
    case 0x80093134 : return QStringLiteral("CRYPT_E_ASN1_NYI");
    case 0x80093201 : return QStringLiteral("CRYPT_E_ASN1_EXTENDED");
    case 0x80093202 : return QStringLiteral("CRYPT_E_ASN1_NOEOD");
    case 0x80094001 : return QStringLiteral("CERTSRV_E_BAD_REQUESTSUBJECT");
    case 0x80094002 : return QStringLiteral("CERTSRV_E_NO_REQUEST");
    case 0x80094003 : return QStringLiteral("CERTSRV_E_BAD_REQUESTSTATUS");
    case 0x80094004 : return QStringLiteral("CERTSRV_E_PROPERTY_EMPTY");
    case 0x80094005 : return QStringLiteral("CERTSRV_E_INVALID_CA_CERTIFICATE");
    case 0x80094006 : return QStringLiteral("CERTSRV_E_SERVER_SUSPENDED");
    case 0x80094007 : return QStringLiteral("CERTSRV_E_ENCODING_LENGTH");
    case 0x80094008 : return QStringLiteral("CERTSRV_E_ROLECONFLICT");
    case 0x80094009 : return QStringLiteral("CERTSRV_E_RESTRICTEDOFFICER");
    case 0x8009400A : return QStringLiteral("CERTSRV_E_KEY_ARCHIVAL_NOT_CONFIGURED");
    case 0x8009400B : return QStringLiteral("CERTSRV_E_NO_VALID_KRA");
    case 0x8009400C : return QStringLiteral("CERTSRV_E_BAD_REQUEST_KEY_ARCHIVAL");
    case 0x8009400D : return QStringLiteral("CERTSRV_E_NO_CAADMIN_DEFINED");
    case 0x8009400E : return QStringLiteral("CERTSRV_E_BAD_RENEWAL_CERT_ATTRIBUTE");
    case 0x8009400F : return QStringLiteral("CERTSRV_E_NO_DB_SESSIONS");
    case 0x80094010 : return QStringLiteral("CERTSRV_E_ALIGNMENT_FAULT");
    case 0x80094011 : return QStringLiteral("CERTSRV_E_ENROLL_DENIED");
    case 0x80094012 : return QStringLiteral("CERTSRV_E_TEMPLATE_DENIED");
    case 0x80094013 : return QStringLiteral("CERTSRV_E_DOWNLEVEL_DC_SSL_OR_UPGRADE");
    case 0x80094800 : return QStringLiteral("CERTSRV_E_UNSUPPORTED_CERT_TYPE");
    case 0x80094801 : return QStringLiteral("CERTSRV_E_NO_CERT_TYPE");
    case 0x80094802 : return QStringLiteral("CERTSRV_E_TEMPLATE_CONFLICT");
    case 0x80094803 : return QStringLiteral("CERTSRV_E_SUBJECT_ALT_NAME_REQUIRED");
    case 0x80094804 : return QStringLiteral("CERTSRV_E_ARCHIVED_KEY_REQUIRED");
    case 0x80094805 : return QStringLiteral("CERTSRV_E_SMIME_REQUIRED");
    case 0x80094806 : return QStringLiteral("CERTSRV_E_BAD_RENEWAL_SUBJECT");
    case 0x80094807 : return QStringLiteral("CERTSRV_E_BAD_TEMPLATE_VERSION");
    case 0x80094808 : return QStringLiteral("CERTSRV_E_TEMPLATE_POLICY_REQUIRED");
    case 0x80094809 : return QStringLiteral("CERTSRV_E_SIGNATURE_POLICY_REQUIRED");
    case 0x8009480A : return QStringLiteral("CERTSRV_E_SIGNATURE_COUNT");
    case 0x8009480B : return QStringLiteral("CERTSRV_E_SIGNATURE_REJECTED");
    case 0x8009480C : return QStringLiteral("CERTSRV_E_ISSUANCE_POLICY_REQUIRED");
    case 0x8009480D : return QStringLiteral("CERTSRV_E_SUBJECT_UPN_REQUIRED");
    case 0x8009480E : return QStringLiteral("CERTSRV_E_SUBJECT_DIRECTORY_GUID_REQUIRED");
    case 0x8009480F : return QStringLiteral("CERTSRV_E_SUBJECT_DNS_REQUIRED");
    case 0x80094810 : return QStringLiteral("CERTSRV_E_ARCHIVED_KEY_UNEXPECTED");
    case 0x80094811 : return QStringLiteral("CERTSRV_E_KEY_LENGTH");
    case 0x80094812 : return QStringLiteral("CERTSRV_E_SUBJECT_EMAIL_REQUIRED");
    case 0x80094813 : return QStringLiteral("CERTSRV_E_UNKNOWN_CERT_TYPE");
    case 0x80094814 : return QStringLiteral("CERTSRV_E_CERT_TYPE_OVERLAP");
    case 0x80095000 : return QStringLiteral("XENROLL_E_KEY_NOT_EXPORTABLE");
    case 0x80095001 : return QStringLiteral("XENROLL_E_CANNOT_ADD_ROOT_CERT");
    case 0x80095002 : return QStringLiteral("XENROLL_E_RESPONSE_KA_HASH_NOT_FOUND");
    case 0x80095003 : return QStringLiteral("XENROLL_E_RESPONSE_UNEXPECTED_KA_HASH");
    case 0x80095004 : return QStringLiteral("XENROLL_E_RESPONSE_KA_HASH_MISMATCH");
    case 0x80095005 : return QStringLiteral("XENROLL_E_KEYSPEC_SMIME_MISMATCH");
    case 0x80096001 : return QStringLiteral("TRUST_E_SYSTEM_ERROR");
    case 0x80096002 : return QStringLiteral("TRUST_E_NO_SIGNER_CERT");
    case 0x80096003 : return QStringLiteral("TRUST_E_COUNTER_SIGNER");
    case 0x80096004 : return QStringLiteral("TRUST_E_CERT_SIGNATURE");
    case 0x80096005 : return QStringLiteral("TRUST_E_TIME_STAMP");
    case 0x80096010 : return QStringLiteral("TRUST_E_BAD_DIGEST");
    case 0x80096019 : return QStringLiteral("TRUST_E_BASIC_CONSTRAINTS");
    case 0x8009601E : return QStringLiteral("TRUST_E_FINANCIAL_CRITERIA");
    case 0x80097001 : return QStringLiteral("MSSIPOTF_E_OUTOFMEMRANGE");
    case 0x80097002 : return QStringLiteral("MSSIPOTF_E_CANTGETOBJECT");
    case 0x80097003 : return QStringLiteral("MSSIPOTF_E_NOHEADTABLE");
    case 0x80097004 : return QStringLiteral("MSSIPOTF_E_BAD_MAGICNUMBER");
    case 0x80097005 : return QStringLiteral("MSSIPOTF_E_BAD_OFFSET_TABLE");
    case 0x80097006 : return QStringLiteral("MSSIPOTF_E_TABLE_TAGORDER");
    case 0x80097007 : return QStringLiteral("MSSIPOTF_E_TABLE_LONGWORD");
    case 0x80097008 : return QStringLiteral("MSSIPOTF_E_BAD_FIRST_TABLE_PLACEMENT");
    case 0x80097009 : return QStringLiteral("MSSIPOTF_E_TABLES_OVERLAP");
    case 0x8009700A : return QStringLiteral("MSSIPOTF_E_TABLE_PADBYTES");
    case 0x8009700B : return QStringLiteral("MSSIPOTF_E_FILETOOSMALL");
    case 0x8009700C : return QStringLiteral("MSSIPOTF_E_TABLE_CHECKSUM");
    case 0x8009700D : return QStringLiteral("MSSIPOTF_E_FILE_CHECKSUM");
    case 0x80097010 : return QStringLiteral("MSSIPOTF_E_FAILED_POLICY");
    case 0x80097011 : return QStringLiteral("MSSIPOTF_E_FAILED_HINTS_CHECK");
    case 0x80097012 : return QStringLiteral("MSSIPOTF_E_NOT_OPENTYPE");
    case 0x80097013 : return QStringLiteral("MSSIPOTF_E_FILE");
    case 0x80097014 : return QStringLiteral("MSSIPOTF_E_CRYPT");
    case 0x80097015 : return QStringLiteral("MSSIPOTF_E_BADVERSION");
    case 0x80097016 : return QStringLiteral("MSSIPOTF_E_DSIG_STRUCTURE");
    case 0x80097017 : return QStringLiteral("MSSIPOTF_E_PCONST_CHECK");
    case 0x80097018 : return QStringLiteral("MSSIPOTF_E_STRUCTURE");
    case 0x800B0001 : return QStringLiteral("TRUST_E_PROVIDER_UNKNOWN");
    case 0x800B0002 : return QStringLiteral("TRUST_E_ACTION_UNKNOWN");
    case 0x800B0003 : return QStringLiteral("TRUST_E_SUBJECT_FORM_UNKNOWN");
    case 0x800B0004 : return QStringLiteral("TRUST_E_SUBJECT_NOT_TRUSTED");
    case 0x800B0005 : return QStringLiteral("DIGSIG_E_ENCODE");
    case 0x800B0006 : return QStringLiteral("DIGSIG_E_DECODE");
    case 0x800B0007 : return QStringLiteral("DIGSIG_E_EXTENSIBILITY");
    case 0x800B0008 : return QStringLiteral("DIGSIG_E_CRYPTO");
    case 0x800B0009 : return QStringLiteral("PERSIST_E_SIZEDEFINITE");
    case 0x800B000A : return QStringLiteral("PERSIST_E_SIZEINDEFINITE");
    case 0x800B000B : return QStringLiteral("PERSIST_E_NOTSELFSIZING");
    case 0x800B0100 : return QStringLiteral("TRUST_E_NOSIGNATURE");
    case 0x800B0101 : return QStringLiteral("CERT_E_EXPIRED");
    case 0x800B0102 : return QStringLiteral("CERT_E_VALIDITYPERIODNESTING");
    case 0x800B0103 : return QStringLiteral("CERT_E_ROLE");
    case 0x800B0104 : return QStringLiteral("CERT_E_PATHLENCONST");
    case 0x800B0105 : return QStringLiteral("CERT_E_CRITICAL");
    case 0x800B0106 : return QStringLiteral("CERT_E_PURPOSE");
    case 0x800B0107 : return QStringLiteral("CERT_E_ISSUERCHAINING");
    case 0x800B0108 : return QStringLiteral("CERT_E_MALFORMED");
    case 0x800B0109 : return QStringLiteral("CERT_E_UNTRUSTEDROOT");
    case 0x800B010A : return QStringLiteral("CERT_E_CHAINING");
    case 0x800B010B : return QStringLiteral("TRUST_E_FAIL");
    case 0x800B010C : return QStringLiteral("CERT_E_REVOKED");
    case 0x800B010D : return QStringLiteral("CERT_E_UNTRUSTEDTESTROOT");
    case 0x800B010E : return QStringLiteral("CERT_E_REVOCATION_FAILURE");
    case 0x800B010F : return QStringLiteral("CERT_E_CN_NO_MATCH");
    case 0x800B0110 : return QStringLiteral("CERT_E_WRONG_USAGE");
    case 0x800B0111 : return QStringLiteral("TRUST_E_EXPLICIT_DISTRUST");
    case 0x800B0112 : return QStringLiteral("CERT_E_UNTRUSTEDCA");
    case 0x800B0113 : return QStringLiteral("CERT_E_INVALID_POLICY");
    case 0x800B0114 : return QStringLiteral("CERT_E_INVALID_NAME");
    case 0x800F0000 : return QStringLiteral("SPAPI_E_EXPECTED_SECTION_NAME");
    case 0x800F0001 : return QStringLiteral("SPAPI_E_BAD_SECTION_NAME_LINE");
    case 0x800F0002 : return QStringLiteral("SPAPI_E_SECTION_NAME_TOO_LONG");
    case 0x800F0003 : return QStringLiteral("SPAPI_E_GENERAL_SYNTAX");
    case 0x800F0100 : return QStringLiteral("SPAPI_E_WRONG_INF_STYLE");
    case 0x800F0101 : return QStringLiteral("SPAPI_E_SECTION_NOT_FOUND");
    case 0x800F0102 : return QStringLiteral("SPAPI_E_LINE_NOT_FOUND");
    case 0x800F0103 : return QStringLiteral("SPAPI_E_NO_BACKUP");
    case 0x800F0200 : return QStringLiteral("SPAPI_E_NO_ASSOCIATED_CLASS");
    case 0x800F0201 : return QStringLiteral("SPAPI_E_CLASS_MISMATCH");
    case 0x800F0202 : return QStringLiteral("SPAPI_E_DUPLICATE_FOUND");
    case 0x800F0203 : return QStringLiteral("SPAPI_E_NO_DRIVER_SELECTED");
    case 0x800F0204 : return QStringLiteral("SPAPI_E_KEY_DOES_NOT_EXIST");
    case 0x800F0205 : return QStringLiteral("SPAPI_E_INVALID_DEVINST_NAME");
    case 0x800F0206 : return QStringLiteral("SPAPI_E_INVALID_CLASS");
    case 0x800F0207 : return QStringLiteral("SPAPI_E_DEVINST_ALREADY_EXISTS");
    case 0x800F0208 : return QStringLiteral("SPAPI_E_DEVINFO_NOT_REGISTERED");
    case 0x800F0209 : return QStringLiteral("SPAPI_E_INVALID_REG_PROPERTY");
    case 0x800F020A : return QStringLiteral("SPAPI_E_NO_INF");
    case 0x800F020B : return QStringLiteral("SPAPI_E_NO_SUCH_DEVINST");
    case 0x800F020C : return QStringLiteral("SPAPI_E_CANT_LOAD_CLASS_ICON");
    case 0x800F020D : return QStringLiteral("SPAPI_E_INVALID_CLASS_INSTALLER");
    case 0x800F020E : return QStringLiteral("SPAPI_E_DI_DO_DEFAULT");
    case 0x800F020F : return QStringLiteral("SPAPI_E_DI_NOFILECOPY");
    case 0x800F0210 : return QStringLiteral("SPAPI_E_INVALID_HWPROFILE");
    case 0x800F0211 : return QStringLiteral("SPAPI_E_NO_DEVICE_SELECTED");
    case 0x800F0212 : return QStringLiteral("SPAPI_E_DEVINFO_LIST_LOCKED");
    case 0x800F0213 : return QStringLiteral("SPAPI_E_DEVINFO_DATA_LOCKED");
    case 0x800F0214 : return QStringLiteral("SPAPI_E_DI_BAD_PATH");
    case 0x800F0215 : return QStringLiteral("SPAPI_E_NO_CLASSINSTALL_PARAMS");
    case 0x800F0216 : return QStringLiteral("SPAPI_E_FILEQUEUE_LOCKED");
    case 0x800F0217 : return QStringLiteral("SPAPI_E_BAD_SERVICE_INSTALLSECT");
    case 0x800F0218 : return QStringLiteral("SPAPI_E_NO_CLASS_DRIVER_LIST");
    case 0x800F0219 : return QStringLiteral("SPAPI_E_NO_ASSOCIATED_SERVICE");
    case 0x800F021A : return QStringLiteral("SPAPI_E_NO_DEFAULT_DEVICE_INTERFACE");
    case 0x800F021B : return QStringLiteral("SPAPI_E_DEVICE_INTERFACE_ACTIVE");
    case 0x800F021C : return QStringLiteral("SPAPI_E_DEVICE_INTERFACE_REMOVED");
    case 0x800F021D : return QStringLiteral("SPAPI_E_BAD_INTERFACE_INSTALLSECT");
    case 0x800F021E : return QStringLiteral("SPAPI_E_NO_SUCH_INTERFACE_CLASS");
    case 0x800F021F : return QStringLiteral("SPAPI_E_INVALID_REFERENCE_STRING");
    case 0x800F0220 : return QStringLiteral("SPAPI_E_INVALID_MACHINENAME");
    case 0x800F0221 : return QStringLiteral("SPAPI_E_REMOTE_COMM_FAILURE");
    case 0x800F0222 : return QStringLiteral("SPAPI_E_MACHINE_UNAVAILABLE");
    case 0x800F0223 : return QStringLiteral("SPAPI_E_NO_CONFIGMGR_SERVICES");
    case 0x800F0224 : return QStringLiteral("SPAPI_E_INVALID_PROPPAGE_PROVIDER");
    case 0x800F0225 : return QStringLiteral("SPAPI_E_NO_SUCH_DEVICE_INTERFACE");
    case 0x800F0226 : return QStringLiteral("SPAPI_E_DI_POSTPROCESSING_REQUIRED");
    case 0x800F0227 : return QStringLiteral("SPAPI_E_INVALID_COINSTALLER");
    case 0x800F0228 : return QStringLiteral("SPAPI_E_NO_COMPAT_DRIVERS");
    case 0x800F0229 : return QStringLiteral("SPAPI_E_NO_DEVICE_ICON");
    case 0x800F022A : return QStringLiteral("SPAPI_E_INVALID_INF_LOGCONFIG");
    case 0x800F022B : return QStringLiteral("SPAPI_E_DI_DONT_INSTALL");
    case 0x800F022C : return QStringLiteral("SPAPI_E_INVALID_FILTER_DRIVER");
    case 0x800F022D : return QStringLiteral("SPAPI_E_NON_WINDOWS_NT_DRIVER");
    case 0x800F022E : return QStringLiteral("SPAPI_E_NON_WINDOWS_DRIVER");
    case 0x800F022F : return QStringLiteral("SPAPI_E_NO_CATALOG_FOR_OEM_INF");
    case 0x800F0230 : return QStringLiteral("SPAPI_E_DEVINSTALL_QUEUE_NONNATIVE");
    case 0x800F0231 : return QStringLiteral("SPAPI_E_NOT_DISABLEABLE");
    case 0x800F0232 : return QStringLiteral("SPAPI_E_CANT_REMOVE_DEVINST");
    case 0x800F0233 : return QStringLiteral("SPAPI_E_INVALID_TARGET");
    case 0x800F0234 : return QStringLiteral("SPAPI_E_DRIVER_NONNATIVE");
    case 0x800F0235 : return QStringLiteral("SPAPI_E_IN_WOW64");
    case 0x800F0236 : return QStringLiteral("SPAPI_E_SET_SYSTEM_RESTORE_POINT");
    case 0x800F0237 : return QStringLiteral("SPAPI_E_INCORRECTLY_COPIED_INF");
    case 0x800F0238 : return QStringLiteral("SPAPI_E_SCE_DISABLED");
    case 0x800F0239 : return QStringLiteral("SPAPI_E_UNKNOWN_EXCEPTION");
    case 0x800F023A : return QStringLiteral("SPAPI_E_PNP_REGISTRY_ERROR");
    case 0x800F023B : return QStringLiteral("SPAPI_E_REMOTE_REQUEST_UNSUPPORTED");
    case 0x800F023C : return QStringLiteral("SPAPI_E_NOT_AN_INSTALLED_OEM_INF");
    case 0x800F023D : return QStringLiteral("SPAPI_E_INF_IN_USE_BY_DEVICES");
    case 0x800F023E : return QStringLiteral("SPAPI_E_DI_FUNCTION_OBSOLETE");
    case 0x800F023F : return QStringLiteral("SPAPI_E_NO_AUTHENTICODE_CATALOG");
    case 0x800F0240 : return QStringLiteral("SPAPI_E_AUTHENTICODE_DISALLOWED");
    case 0x800F0241 : return QStringLiteral("SPAPI_E_AUTHENTICODE_TRUSTED_PUBLISHER");
    case 0x800F0242 : return QStringLiteral("SPAPI_E_AUTHENTICODE_TRUST_NOT_ESTABLISHED");
    case 0x800F0243 : return QStringLiteral("SPAPI_E_AUTHENTICODE_PUBLISHER_NOT_TRUSTED");
    case 0x800F0244 : return QStringLiteral("SPAPI_E_SIGNATURE_OSATTRIBUTE_MISMATCH");
    case 0x800F0245 : return QStringLiteral("SPAPI_E_ONLY_VALIDATE_VIA_AUTHENTICODE");
    case 0x800F0300 : return QStringLiteral("SPAPI_E_UNRECOVERABLE_STACK_OVERFLOW");
    case 0x800F1000 : return QStringLiteral("SPAPI_E_ERROR_NOT_INSTALLED");
    case 0x80100001 : return QStringLiteral("SCARD_F_INTERNAL_ERROR");
    case 0x80100002 : return QStringLiteral("SCARD_E_CANCELLED");
    case 0x80100003 : return QStringLiteral("SCARD_E_INVALID_HANDLE");
    case 0x80100004 : return QStringLiteral("SCARD_E_INVALID_PARAMETER");
    case 0x80100005 : return QStringLiteral("SCARD_E_INVALID_TARGET");
    case 0x80100006 : return QStringLiteral("SCARD_E_NO_MEMORY");
    case 0x80100007 : return QStringLiteral("SCARD_F_WAITED_TOO_LONG");
    case 0x80100008 : return QStringLiteral("SCARD_E_INSUFFICIENT_BUFFER");
    case 0x80100009 : return QStringLiteral("SCARD_E_UNKNOWN_READER");
    case 0x8010000A : return QStringLiteral("SCARD_E_TIMEOUT");
    case 0x8010000B : return QStringLiteral("SCARD_E_SHARING_VIOLATION");
    case 0x8010000C : return QStringLiteral("SCARD_E_NO_SMARTCARD");
    case 0x8010000D : return QStringLiteral("SCARD_E_UNKNOWN_CARD");
    case 0x8010000E : return QStringLiteral("SCARD_E_CANT_DISPOSE");
    case 0x8010000F : return QStringLiteral("SCARD_E_PROTO_MISMATCH");
    case 0x80100010 : return QStringLiteral("SCARD_E_NOT_READY");
    case 0x80100011 : return QStringLiteral("SCARD_E_INVALID_VALUE");
    case 0x80100012 : return QStringLiteral("SCARD_E_SYSTEM_CANCELLED");
    case 0x80100013 : return QStringLiteral("SCARD_F_COMM_ERROR");
    case 0x80100014 : return QStringLiteral("SCARD_F_UNKNOWN_ERROR");
    case 0x80100015 : return QStringLiteral("SCARD_E_INVALID_ATR");
    case 0x80100016 : return QStringLiteral("SCARD_E_NOT_TRANSACTED");
    case 0x80100017 : return QStringLiteral("SCARD_E_READER_UNAVAILABLE");
    case 0x80100018 : return QStringLiteral("SCARD_P_SHUTDOWN");
    case 0x80100019 : return QStringLiteral("SCARD_E_PCI_TOO_SMALL");
    case 0x8010001A : return QStringLiteral("SCARD_E_READER_UNSUPPORTED");
    case 0x8010001B : return QStringLiteral("SCARD_E_DUPLICATE_READER");
    case 0x8010001C : return QStringLiteral("SCARD_E_CARD_UNSUPPORTED");
    case 0x8010001D : return QStringLiteral("SCARD_E_NO_SERVICE");
    case 0x8010001E : return QStringLiteral("SCARD_E_SERVICE_STOPPED");
    case 0x8010001F : return QStringLiteral("SCARD_E_UNEXPECTED");
    case 0x80100020 : return QStringLiteral("SCARD_E_ICC_INSTALLATION");
    case 0x80100021 : return QStringLiteral("SCARD_E_ICC_CREATEORDER");
    case 0x80100022 : return QStringLiteral("SCARD_E_UNSUPPORTED_FEATURE");
    case 0x80100023 : return QStringLiteral("SCARD_E_DIR_NOT_FOUND");
    case 0x80100024 : return QStringLiteral("SCARD_E_FILE_NOT_FOUND");
    case 0x80100025 : return QStringLiteral("SCARD_E_NO_DIR");
    case 0x80100026 : return QStringLiteral("SCARD_E_NO_FILE");
    case 0x80100027 : return QStringLiteral("SCARD_E_NO_ACCESS");
    case 0x80100028 : return QStringLiteral("SCARD_E_WRITE_TOO_MANY");
    case 0x80100029 : return QStringLiteral("SCARD_E_BAD_SEEK");
    case 0x8010002A : return QStringLiteral("SCARD_E_INVALID_CHV");
    case 0x8010002B : return QStringLiteral("SCARD_E_UNKNOWN_RES_MNG");
    case 0x8010002C : return QStringLiteral("SCARD_E_NO_SUCH_CERTIFICATE");
    case 0x8010002D : return QStringLiteral("SCARD_E_CERTIFICATE_UNAVAILABLE");
    case 0x8010002E : return QStringLiteral("SCARD_E_NO_READERS_AVAILABLE");
    case 0x8010002F : return QStringLiteral("SCARD_E_COMM_DATA_LOST");
    case 0x80100030 : return QStringLiteral("SCARD_E_NO_KEY_CONTAINER");
    case 0x80100031 : return QStringLiteral("SCARD_E_SERVER_TOO_BUSY");
    case 0x80100065 : return QStringLiteral("SCARD_W_UNSUPPORTED_CARD");
    case 0x80100066 : return QStringLiteral("SCARD_W_UNRESPONSIVE_CARD");
    case 0x80100067 : return QStringLiteral("SCARD_W_UNPOWERED_CARD");
    case 0x80100068 : return QStringLiteral("SCARD_W_RESET_CARD");
    case 0x80100069 : return QStringLiteral("SCARD_W_REMOVED_CARD");
    case 0x8010006A : return QStringLiteral("SCARD_W_SECURITY_VIOLATION");
    case 0x8010006B : return QStringLiteral("SCARD_W_WRONG_CHV");
    case 0x8010006C : return QStringLiteral("SCARD_W_CHV_BLOCKED");
    case 0x8010006D : return QStringLiteral("SCARD_W_EOF");
    case 0x8010006E : return QStringLiteral("SCARD_W_CANCELLED_BY_USER");
    case 0x8010006F : return QStringLiteral("SCARD_W_CARD_NOT_AUTHENTICATED");
    case 0x80100070 : return QStringLiteral("SCARD_W_CACHE_ITEM_NOT_FOUND");
    case 0x80100071 : return QStringLiteral("SCARD_W_CACHE_ITEM_STALE");
    case 0x80110401 : return QStringLiteral("COMADMIN_E_OBJECTERRORS");
    case 0x80110402 : return QStringLiteral("COMADMIN_E_OBJECTINVALID");
    case 0x80110403 : return QStringLiteral("COMADMIN_E_KEYMISSING");
    case 0x80110404 : return QStringLiteral("COMADMIN_E_ALREADYINSTALLED");
    case 0x80110407 : return QStringLiteral("COMADMIN_E_APP_FILE_WRITEFAIL");
    case 0x80110408 : return QStringLiteral("COMADMIN_E_APP_FILE_READFAIL");
    case 0x80110409 : return QStringLiteral("COMADMIN_E_APP_FILE_VERSION");
    case 0x8011040A : return QStringLiteral("COMADMIN_E_BADPATH");
    case 0x8011040B : return QStringLiteral("COMADMIN_E_APPLICATIONEXISTS");
    case 0x8011040C : return QStringLiteral("COMADMIN_E_ROLEEXISTS");
    case 0x8011040D : return QStringLiteral("COMADMIN_E_CANTCOPYFILE");
    case 0x8011040F : return QStringLiteral("COMADMIN_E_NOUSER");
    case 0x80110410 : return QStringLiteral("COMADMIN_E_INVALIDUSERIDS");
    case 0x80110411 : return QStringLiteral("COMADMIN_E_NOREGISTRYCLSID");
    case 0x80110412 : return QStringLiteral("COMADMIN_E_BADREGISTRYPROGID");
    case 0x80110413 : return QStringLiteral("COMADMIN_E_AUTHENTICATIONLEVEL");
    case 0x80110414 : return QStringLiteral("COMADMIN_E_USERPASSWDNOTVALID");
    case 0x80110418 : return QStringLiteral("COMADMIN_E_CLSIDORIIDMISMATCH");
    case 0x80110419 : return QStringLiteral("COMADMIN_E_REMOTEINTERFACE");
    case 0x8011041A : return QStringLiteral("COMADMIN_E_DLLREGISTERSERVER");
    case 0x8011041B : return QStringLiteral("COMADMIN_E_NOSERVERSHARE");
    case 0x8011041D : return QStringLiteral("COMADMIN_E_DLLLOADFAILED");
    case 0x8011041E : return QStringLiteral("COMADMIN_E_BADREGISTRYLIBID");
    case 0x8011041F : return QStringLiteral("COMADMIN_E_APPDIRNOTFOUND");
    case 0x80110423 : return QStringLiteral("COMADMIN_E_REGISTRARFAILED");
    case 0x80110424 : return QStringLiteral("COMADMIN_E_COMPFILE_DOESNOTEXIST");
    case 0x80110425 : return QStringLiteral("COMADMIN_E_COMPFILE_LOADDLLFAIL");
    case 0x80110426 : return QStringLiteral("COMADMIN_E_COMPFILE_GETCLASSOBJ");
    case 0x80110427 : return QStringLiteral("COMADMIN_E_COMPFILE_CLASSNOTAVAIL");
    case 0x80110428 : return QStringLiteral("COMADMIN_E_COMPFILE_BADTLB");
    case 0x80110429 : return QStringLiteral("COMADMIN_E_COMPFILE_NOTINSTALLABLE");
    case 0x8011042A : return QStringLiteral("COMADMIN_E_NOTCHANGEABLE");
    case 0x8011042B : return QStringLiteral("COMADMIN_E_NOTDELETEABLE");
    case 0x8011042C : return QStringLiteral("COMADMIN_E_SESSION");
    case 0x8011042D : return QStringLiteral("COMADMIN_E_COMP_MOVE_LOCKED");
    case 0x8011042E : return QStringLiteral("COMADMIN_E_COMP_MOVE_BAD_DEST");
    case 0x80110430 : return QStringLiteral("COMADMIN_E_REGISTERTLB");
    case 0x80110433 : return QStringLiteral("COMADMIN_E_SYSTEMAPP");
    case 0x80110434 : return QStringLiteral("COMADMIN_E_COMPFILE_NOREGISTRAR");
    case 0x80110435 : return QStringLiteral("COMADMIN_E_COREQCOMPINSTALLED");
    case 0x80110436 : return QStringLiteral("COMADMIN_E_SERVICENOTINSTALLED");
    case 0x80110437 : return QStringLiteral("COMADMIN_E_PROPERTYSAVEFAILED");
    case 0x80110438 : return QStringLiteral("COMADMIN_E_OBJECTEXISTS");
    case 0x80110439 : return QStringLiteral("COMADMIN_E_COMPONENTEXISTS");
    case 0x8011043B : return QStringLiteral("COMADMIN_E_REGFILE_CORRUPT");
    case 0x8011043C : return QStringLiteral("COMADMIN_E_PROPERTY_OVERFLOW");
    case 0x8011043E : return QStringLiteral("COMADMIN_E_NOTINREGISTRY");
    case 0x8011043F : return QStringLiteral("COMADMIN_E_OBJECTNOTPOOLABLE");
    case 0x80110446 : return QStringLiteral("COMADMIN_E_APPLID_MATCHES_CLSID");
    case 0x80110447 : return QStringLiteral("COMADMIN_E_ROLE_DOES_NOT_EXIST");
    case 0x80110448 : return QStringLiteral("COMADMIN_E_START_APP_NEEDS_COMPONENTS");
    case 0x80110449 : return QStringLiteral("COMADMIN_E_REQUIRES_DIFFERENT_PLATFORM");
    case 0x8011044A : return QStringLiteral("COMADMIN_E_CAN_NOT_EXPORT_APP_PROXY");
    case 0x8011044B : return QStringLiteral("COMADMIN_E_CAN_NOT_START_APP");
    case 0x8011044C : return QStringLiteral("COMADMIN_E_CAN_NOT_EXPORT_SYS_APP");
    case 0x8011044D : return QStringLiteral("COMADMIN_E_CANT_SUBSCRIBE_TO_COMPONENT");
    case 0x8011044E : return QStringLiteral("COMADMIN_E_EVENTCLASS_CANT_BE_SUBSCRIBER");
    case 0x8011044F : return QStringLiteral("COMADMIN_E_LIB_APP_PROXY_INCOMPATIBLE");
    case 0x80110450 : return QStringLiteral("COMADMIN_E_BASE_PARTITION_ONLY");
    case 0x80110451 : return QStringLiteral("COMADMIN_E_START_APP_DISABLED");
    case 0x80110457 : return QStringLiteral("COMADMIN_E_CAT_DUPLICATE_PARTITION_NAME");
    case 0x80110458 : return QStringLiteral("COMADMIN_E_CAT_INVALID_PARTITION_NAME");
    case 0x80110459 : return QStringLiteral("COMADMIN_E_CAT_PARTITION_IN_USE");
    case 0x8011045A : return QStringLiteral("COMADMIN_E_FILE_PARTITION_DUPLICATE_FILES");
    case 0x8011045B : return QStringLiteral("COMADMIN_E_CAT_IMPORTED_COMPONENTS_NOT_ALLOWED");
    case 0x8011045C : return QStringLiteral("COMADMIN_E_AMBIGUOUS_APPLICATION_NAME");
    case 0x8011045D : return QStringLiteral("COMADMIN_E_AMBIGUOUS_PARTITION_NAME");
    case 0x80110472 : return QStringLiteral("COMADMIN_E_REGDB_NOTINITIALIZED");
    case 0x80110473 : return QStringLiteral("COMADMIN_E_REGDB_NOTOPEN");
    case 0x80110474 : return QStringLiteral("COMADMIN_E_REGDB_SYSTEMERR");
    case 0x80110475 : return QStringLiteral("COMADMIN_E_REGDB_ALREADYRUNNING");
    case 0x80110480 : return QStringLiteral("COMADMIN_E_MIG_VERSIONNOTSUPPORTED");
    case 0x80110481 : return QStringLiteral("COMADMIN_E_MIG_SCHEMANOTFOUND");
    case 0x80110482 : return QStringLiteral("COMADMIN_E_CAT_BITNESSMISMATCH");
    case 0x80110483 : return QStringLiteral("COMADMIN_E_CAT_UNACCEPTABLEBITNESS");
    case 0x80110484 : return QStringLiteral("COMADMIN_E_CAT_WRONGAPPBITNESS");
    case 0x80110485 : return QStringLiteral("COMADMIN_E_CAT_PAUSE_RESUME_NOT_SUPPORTED");
    case 0x80110486 : return QStringLiteral("COMADMIN_E_CAT_SERVERFAULT");
    case 0x80110600 : return QStringLiteral("COMQC_E_APPLICATION_NOT_QUEUED");
    case 0x80110601 : return QStringLiteral("COMQC_E_NO_QUEUEABLE_INTERFACES");
    case 0x80110602 : return QStringLiteral("COMQC_E_QUEUING_SERVICE_NOT_AVAILABLE");
    case 0x80110603 : return QStringLiteral("COMQC_E_NO_IPERSISTSTREAM");
    case 0x80110604 : return QStringLiteral("COMQC_E_BAD_MESSAGE");
    case 0x80110605 : return QStringLiteral("COMQC_E_UNAUTHENTICATED");
    case 0x80110606 : return QStringLiteral("COMQC_E_UNTRUSTED_ENQUEUER");
    case 0x80110701 : return QStringLiteral("MSDTC_E_DUPLICATE_RESOURCE");
    case 0x80110808 : return QStringLiteral("COMADMIN_E_OBJECT_PARENT_MISSING");
    case 0x80110809 : return QStringLiteral("COMADMIN_E_OBJECT_DOES_NOT_EXIST");
    case 0x8011080A : return QStringLiteral("COMADMIN_E_APP_NOT_RUNNING");
    case 0x8011080B : return QStringLiteral("COMADMIN_E_INVALID_PARTITION");
    case 0x8011080D : return QStringLiteral("COMADMIN_E_SVCAPP_NOT_POOLABLE_OR_RECYCLABLE");
    case 0x8011080E : return QStringLiteral("COMADMIN_E_USER_IN_SET");
    case 0x8011080F : return QStringLiteral("COMADMIN_E_CANTRECYCLELIBRARYAPPS");
    case 0x80110811 : return QStringLiteral("COMADMIN_E_CANTRECYCLESERVICEAPPS");
    case 0x80110812 : return QStringLiteral("COMADMIN_E_PROCESSALREADYRECYCLED");
    case 0x80110813 : return QStringLiteral("COMADMIN_E_PAUSEDPROCESSMAYNOTBERECYCLED");
    case 0x80110814 : return QStringLiteral("COMADMIN_E_CANTMAKEINPROCSERVICE");
    case 0x80110815 : return QStringLiteral("COMADMIN_E_PROGIDINUSEBYCLSID");
    case 0x80110816 : return QStringLiteral("COMADMIN_E_DEFAULT_PARTITION_NOT_IN_SET");
    case 0x80110817 : return QStringLiteral("COMADMIN_E_RECYCLEDPROCESSMAYNOTBEPAUSED");
    case 0x80110818 : return QStringLiteral("COMADMIN_E_PARTITION_ACCESSDENIED");
    case 0x80110819 : return QStringLiteral("COMADMIN_E_PARTITION_MSI_ONLY");
    case 0x8011081A : return QStringLiteral("COMADMIN_E_LEGACYCOMPS_NOT_ALLOWED_IN_1_0_FORMAT");
    case 0x8011081B : return QStringLiteral("COMADMIN_E_LEGACYCOMPS_NOT_ALLOWED_IN_NONBASE_PARTITIONS");
    case 0x8011081C : return QStringLiteral("COMADMIN_E_COMP_MOVE_SOURCE");
    case 0x8011081D : return QStringLiteral("COMADMIN_E_COMP_MOVE_DEST");
    case 0x8011081E : return QStringLiteral("COMADMIN_E_COMP_MOVE_PRIVATE");
    case 0x8011081F : return QStringLiteral("COMADMIN_E_BASEPARTITION_REQUIRED_IN_SET");
    case 0x80110820 : return QStringLiteral("COMADMIN_E_CANNOT_ALIAS_EVENTCLASS");
    case 0x80110821 : return QStringLiteral("COMADMIN_E_PRIVATE_ACCESSDENIED");
    case 0x80110822 : return QStringLiteral("COMADMIN_E_SAFERINVALID");
    case 0x80110823 : return QStringLiteral("COMADMIN_E_REGISTRY_ACCESSDENIED");
    case 0x80110824 : return QStringLiteral("COMADMIN_E_PARTITIONS_DISABLED");
    case 0x80042301 : return QStringLiteral("VSS_E_BAD_STATE");
    case 0x800423F7 : return QStringLiteral("VSS_E_LEGACY_PROVIDER");
    case 0x800423FF : return QStringLiteral("VSS_E_RESYNC_IN_PROGRESS");
    case 0x8004232B : return QStringLiteral("VSS_E_SNAPSHOT_NOT_IN_SET");
    case 0x80042312 : return QStringLiteral("VSS_E_MAXIMUM_NUMBER_OF_VOLUMES_REACHED");
    case 0x80042317 : return QStringLiteral("VSS_E_MAXIMUM_NUMBER_OF_SNAPSHOTS_REACHED");
    case 0x8004232C : return QStringLiteral("VSS_E_NESTED_VOLUME_LIMIT");
    case 0x80042308 : return QStringLiteral("VSS_E_OBJECT_NOT_FOUND");
    case 0x80042304 : return QStringLiteral("VSS_E_PROVIDER_NOT_REGISTERED");
    case 0x80042306 : return QStringLiteral("VSS_E_PROVIDER_VETO");
    case 0x8004230C : return QStringLiteral("VSS_E_VOLUME_NOT_SUPPORTED");
    case 0x8004230E : return QStringLiteral("VSS_E_VOLUME_NOT_SUPPORTED_BY_PROVIDER");
    case 0x80042302 : return QStringLiteral("VSS_E_UNEXPECTED");
    case 0x8004230F : return QStringLiteral("VSS_E_UNEXPECTED_PROVIDER_ERROR");
    case 0x8004232A : return QStringLiteral("VSS_E_UNSELECTED_VOLUME");
    case 0x800423FE : return QStringLiteral("VSS_E_CANNOT_REVERT_DISKID");
    case 0x80042311 : return QStringLiteral("VSS_E_INVALID_XML_DOCUMENT");
    case 0x8004230D : return QStringLiteral("VSS_E_OBJECT_ALREADY_EXISTS");
    case 0x80284001 : return QStringLiteral("TBS_E_INTERNAL_ERROR");
    case 0x80284002 : return QStringLiteral("TBS_E_BAD_PARAMETER");
    case 0x80284003 : return QStringLiteral("TBS_E_INVALID_OUTPUT_POINTER");
    case 0x80284005 : return QStringLiteral("TBS_E_INSUFFICIENT_BUFFER");
    case 0x80284006 : return QStringLiteral("TBS_E_IOERROR");
    case 0x80284007 : return QStringLiteral("TBS_E_INVALID_CONTEXT_PARAM");
    case 0x80284008 : return QStringLiteral("TBS_E_SERVICE_NOT_RUNNING");
    case 0x80284009 : return QStringLiteral("TBS_E_TOO_MANY_TBS_CONTEXTS");
    case 0x8028400B : return QStringLiteral("TBS_E_SERVICE_START_PENDING");
    case 0x8028400E : return QStringLiteral("TBS_E_BUFFER_TOO_LARGE");
    case 0x8028400F : return QStringLiteral("TBS_E_TPM_NOT_FOUND");
    case 0x80284010 : return QStringLiteral("TBS_E_SERVICE_DISABLED");
    case 0x80284016 : return QStringLiteral("TBS_E_DEACTIVATED");
    case 0x80320001 : return QStringLiteral("FWP_E_CALLOUT_NOT_FOUND");
    case 0x80320002 : return QStringLiteral("FWP_E_CONDITION_NOT_FOUND");
    case 0x80320003 : return QStringLiteral("FWP_E_FILTER_NOT_FOUND");
    case 0x80320004 : return QStringLiteral("FWP_E_LAYER_NOT_FOUND");
    case 0x80320005 : return QStringLiteral("FWP_E_PROVIDER_NOT_FOUND");
    case 0x80320006 : return QStringLiteral("FWP_E_PROVIDER_CONTEXT_NOT_FOUND");
    case 0x80320007 : return QStringLiteral("FWP_E_SUBLAYER_NOT_FOUND");
    case 0x80320008 : return QStringLiteral("FWP_E_NOT_FOUND");
    case 0x80320009 : return QStringLiteral("FWP_E_ALREADY_EXISTS");
    case 0x8032000A : return QStringLiteral("FWP_E_IN_USE");
    case 0x8032000B : return QStringLiteral("FWP_E_DYNAMIC_SESSION_IN_PROGRESS");
    case 0x8032000C : return QStringLiteral("FWP_E_WRONG_SESSION");
    case 0x8032000D : return QStringLiteral("FWP_E_NO_TXN_IN_PROGRESS");
    case 0x8032000E : return QStringLiteral("FWP_E_TXN_IN_PROGRESS");
    case 0x8032000F : return QStringLiteral("FWP_E_TXN_ABORTED");
    case 0x80320010 : return QStringLiteral("FWP_E_SESSION_ABORTED");
    case 0x80320011 : return QStringLiteral("FWP_E_INCOMPATIBLE_TXN");
    case 0x80320012 : return QStringLiteral("FWP_E_TIMEOUT");
    case 0x80320013 : return QStringLiteral("FWP_E_NET_EVENTS_DISABLED");
    case 0x80320014 : return QStringLiteral("FWP_E_INCOMPATIBLE_LAYER");
    case 0x80320015 : return QStringLiteral("FWP_E_KM_CLIENTS_ONLY");
    case 0x80320016 : return QStringLiteral("FWP_E_LIFETIME_MISMATCH");
    case 0x80320017 : return QStringLiteral("FWP_E_BUILTIN_OBJECT");
    case 0x80320018 : return QStringLiteral("FWP_E_TOO_MANY_CALLOUTS");
    case 0x80320019 : return QStringLiteral("FWP_E_NOTIFICATION_DROPPED");
    case 0x8032001A : return QStringLiteral("FWP_E_TRAFFIC_MISMATCH");
    case 0x8032001B : return QStringLiteral("FWP_E_INCOMPATIBLE_SA_STATE");
    case 0x8032001C : return QStringLiteral("FWP_E_NULL_POINTER");
    case 0x8032001D : return QStringLiteral("FWP_E_INVALID_ENUMERATOR");
    case 0x8032001E : return QStringLiteral("FWP_E_INVALID_FLAGS");
    case 0x8032001F : return QStringLiteral("FWP_E_INVALID_NET_MASK");
    case 0x80320020 : return QStringLiteral("FWP_E_INVALID_RANGE");
    case 0x80320021 : return QStringLiteral("FWP_E_INVALID_INTERVAL");
    case 0x80320022 : return QStringLiteral("FWP_E_ZERO_LENGTH_ARRAY");
    case 0x80320023 : return QStringLiteral("FWP_E_NULL_DISPLAY_NAME");
    case 0x80320024 : return QStringLiteral("FWP_E_INVALID_ACTION_TYPE");
    case 0x80320025 : return QStringLiteral("FWP_E_INVALID_WEIGHT");
    case 0x80320026 : return QStringLiteral("FWP_E_MATCH_TYPE_MISMATCH");
    case 0x80320027 : return QStringLiteral("FWP_E_TYPE_MISMATCH");
    case 0x80320028 : return QStringLiteral("FWP_E_OUT_OF_BOUNDS");
    case 0x80320029 : return QStringLiteral("FWP_E_RESERVED");
    case 0x8032002A : return QStringLiteral("FWP_E_DUPLICATE_CONDITION");
    case 0x8032002B : return QStringLiteral("FWP_E_DUPLICATE_KEYMOD");
    case 0x8032002C : return QStringLiteral("FWP_E_ACTION_INCOMPATIBLE_WITH_LAYER");
    case 0x8032002D : return QStringLiteral("FWP_E_ACTION_INCOMPATIBLE_WITH_SUBLAYER");
    case 0x8032002E : return QStringLiteral("FWP_E_CONTEXT_INCOMPATIBLE_WITH_LAYER");
    case 0x8032002F : return QStringLiteral("FWP_E_CONTEXT_INCOMPATIBLE_WITH_CALLOUT");
    case 0x80320030 : return QStringLiteral("FWP_E_INCOMPATIBLE_AUTH_METHOD");
    case 0x80320031 : return QStringLiteral("FWP_E_INCOMPATIBLE_DH_GROUP");
    case 0x80320032 : return QStringLiteral("FWP_E_EM_NOT_SUPPORTED");
    case 0x80320033 : return QStringLiteral("FWP_E_NEVER_MATCH");
    case 0x80320034 : return QStringLiteral("FWP_E_PROVIDER_CONTEXT_MISMATCH");
    case 0x80320035 : return QStringLiteral("FWP_E_INVALID_PARAMETER");
    case 0x80320036 : return QStringLiteral("FWP_E_TOO_MANY_SUBLAYERS");
    case 0x80320037 : return QStringLiteral("FWP_E_CALLOUT_NOTIFICATION_FAILED");
    case 0x80320038 : return QStringLiteral("FWP_E_INVALID_AUTH_TRANSFORM");
    case 0x80320039 : return QStringLiteral("FWP_E_INVALID_CIPHER_TRANSFORM");
    default : return QString();
    }
}

/*!
    \since 5.2

    Returns the DWM colorization color. After the function returns, the optional
    \a opaqueBlend will contain true if the color is an opaque blend and false
    otherwise.
 */
QColor QtWin::colorizationColor(bool *opaqueBlend)
{
    QWinEventFilter::setup();

    DWORD colorization = 0;
    BOOL dummy = false;
    qtDwmApiDll.init();
    if (qtDwmApiDll.dwmGetColorizationColor)
        qtDwmApiDll.dwmGetColorizationColor(&colorization, &dummy);
    if (opaqueBlend)
        *opaqueBlend = dummy;
    return QColor::fromRgba(colorization);
}

/*!
    \since 5.2

    Returns the real colorization color, set by the user, using an undocumented
    registry key. The API-based function \c getColorizationColor() returns an
    alpha-blended color which often turns out a semitransparent gray rather
    than something similar to what is chosen by the user.

    \sa colorizationColor()
 */
QColor QtWin::realColorizationColor()
{
    QWinEventFilter::setup();

    bool ok = false;
    const QLatin1String path("HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\DWM");
    QSettings registry(path, QSettings::NativeFormat);
    QRgb color = registry.value(QLatin1String("ColorizationColor")).toUInt(&ok);
    if (!ok)
        qDebug("Failed to read colorization color.");
    return ok ? QColor::fromRgba(color) : QColor();
}

/*!
    \fn QtWin::setWindowExcludedFromPeek(QWidget *window, bool exclude)
    \since 5.2
    \overload QtWin::setWindowExcludedFromPeek()
*/

/*!
    \since 5.2

    Excludes the specified \a window from Aero Peek if \a exclude is true.
 */
void QtWin::setWindowExcludedFromPeek(QWindow *window, bool exclude)
{
    Q_ASSERT_X(window, Q_FUNC_INFO, "window is null");
    QtDwmApiDll::setBooleanWindowAttribute(reinterpret_cast<HWND>(window->winId()), qt_DWMWA_EXCLUDED_FROM_PEEK, exclude);
}

/*!
    \fn bool QtWin::isWindowExcludedFromPeek(QWidget *window)
    \since 5.2
    \overload QtWin::isWindowExcludedFromPeek()
*/

/*!
    \since 5.2

    Returns true if the specified \a window is excluded from Aero Peek.
 */
bool QtWin::isWindowExcludedFromPeek(QWindow *window)
{
    Q_ASSERT_X(window, Q_FUNC_INFO, "window is null");
    return QtDwmApiDll::booleanWindowAttribute(reinterpret_cast<HWND>(window->winId()), qt_DWMWA_EXCLUDED_FROM_PEEK);
}

/*!
    \fn void QtWin::setWindowDisallowPeek(QWidget *window, bool disallow)
    \since 5.2
    \overload QtWin::setWindowDisallowPeek()
*/

/*!
    \since 5.2

    Disables Aero Peek for the specified \a window when hovering over the
    taskbar thumbnail of the window with the mouse pointer if \a disallow is
    true; otherwise allows it.

    The default is false.
 */
void QtWin::setWindowDisallowPeek(QWindow *window, bool disallow)
{
    Q_ASSERT_X(window, Q_FUNC_INFO, "window is null");
    QtDwmApiDll::setBooleanWindowAttribute(reinterpret_cast<HWND>(window->winId()), qt_DWMWA_DISALLOW_PEEK, disallow);
}

/*!
    \fn bool QtWin::isWindowPeekDisallowed(QWidget *window)
    \since 5.2
    \overload QtWin::isWindowPeekDisallowed()
*/

/*!
    \since 5.2

    Returns true if Aero Peek is disallowed on the thumbnail of the specified
    \a window.
 */
bool QtWin::isWindowPeekDisallowed(QWindow *window)
{
    Q_ASSERT_X(window, Q_FUNC_INFO, "window is null");
    return QtDwmApiDll::booleanWindowAttribute(reinterpret_cast<HWND>(window->winId()), qt_DWMWA_DISALLOW_PEEK);
}

/*!
    \fn void QtWin::setWindowFlip3DPolicy(QWidget *window, QtWin::WindowFlip3DPolicy policy)
    \since 5.2
    \overload QtWin::setWindowFlip3DPolicy()
*/

/*!
    \since 5.2

    Sets the Flip3D policy \a policy for the specified \a window.
 */
void QtWin::setWindowFlip3DPolicy(QWindow *window, QtWin::WindowFlip3DPolicy policy)
{
    Q_ASSERT_X(window, Q_FUNC_INFO, "window is null");

    HWND handle = reinterpret_cast<HWND>(window->winId());

    // Policy should be defaulted first, bug or smth.
    DWORD value = qt_DWMFLIP3D_DEFAULT;
    QtDwmApiDll::setWindowAttribute(handle, qt_DWMWA_FLIP3D_POLICY, value);

    switch (policy) {
    default :
    case FlipDefault :
        value = -1;
        break;
    case FlipExcludeBelow :
        value = qt_DWMFLIP3D_EXCLUDEBELOW;
        break;
    case FlipExcludeAbove :
        value = qt_DWMFLIP3D_EXCLUDEABOVE;
        break;
    }

    if (qt_DWMFLIP3D_DEFAULT != value)
        QtDwmApiDll::setWindowAttribute(handle, qt_DWMWA_FLIP3D_POLICY, value);
}

/*!
    \fn QtWin::WindowFlip3DPolicy QtWin::windowFlip3DPolicy(QWidget *window)
    \since 5.2
    \overload QtWin::windowFlip3DPolicy()
 */

/*!
    \since 5.2

    Returns the current Flip3D policy for the specified \a window.
 */
QtWin::WindowFlip3DPolicy QtWin::windowFlip3DPolicy(QWindow *window)
{
    Q_ASSERT_X(window, Q_FUNC_INFO, "window is null");

    const DWORD value =
        QtDwmApiDll::windowAttribute<DWORD>(reinterpret_cast<HWND>(window->winId()),
                                            qt_DWMWA_FLIP3D_POLICY, DWORD(qt_DWMFLIP3D_DEFAULT));
    QtWin::WindowFlip3DPolicy policy = QtWin::FlipDefault;
    switch (value) {
    case qt_DWMFLIP3D_EXCLUDEABOVE :
        policy = QtWin::FlipExcludeAbove;
        break;
    case qt_DWMFLIP3D_EXCLUDEBELOW :
        policy = QtWin::FlipExcludeBelow;
        break;
    default :
        break;
    }
    return policy;
}

void qt_ExtendFrameIntoClientArea(QWindow *window, int left, int top, int right, int bottom)
{
    QWinEventFilter::setup();

    qtDwmApiDll.init();
    if (qtDwmApiDll.dwmExtendFrameIntoClientArea) {
        MARGINS margins = {left, right, top, bottom};
        qtDwmApiDll.dwmExtendFrameIntoClientArea(reinterpret_cast<HWND>(window->winId()), &margins);
    }
}

/*! \fn void QtWin::extendFrameIntoClientArea(QWidget *window, int left, int top, int right, int bottom)
    \since 5.2
    \overload QtWin::extendFrameIntoClientArea()
 */

/*!
    \since 5.2

    Extends the glass frame into the client area of the specified \a window
    using the \a left, \a top, \a right, and \a bottom margin values.

    Pass -1 as values for any of the four margins to fully extend the frame,
    creating a \e {sheet of glass} effect.

    If you want the extended frame to act like a standard window border, you
    should handle that yourself.

    \note If \a window is a QWidget handle, set the
    Qt::WA_NoSystemBackground attribute for your widget.

    \sa resetExtendedFrame()
 */
void QtWin::extendFrameIntoClientArea(QWindow *window, int left, int top, int right, int bottom)
{
    Q_ASSERT_X(window, Q_FUNC_INFO, "window is null");
    qt_ExtendFrameIntoClientArea(window, left, top, right, bottom);
}

/*! \fn void QtWin::extendFrameIntoClientArea(QWidget *window, const QMargins &margins)
    \since 5.2
    \overload QtWin::extendFrameIntoClientArea()

    Convenience overload that allows passing frame sizes in a \a margins
    structure.
 */

/*!
    \since 5.2
    \overload QtWin::extendFrameIntoClientArea()

    Extends the glass frame into the client area of the specified \a window
    using the specified \a margins.
 */
void QtWin::extendFrameIntoClientArea(QWindow *window, const QMargins &margins)
{
    QtWin::extendFrameIntoClientArea(window, margins.left(), margins.top(), margins.right(), margins.bottom());
}

/*!
    \fn void QtWin::resetExtendedFrame(QWidget *window)
    \since 5.2
    \overload QtWin::resetExtendedFrame()
 */

/*!
    \since 5.2

    Resets the glass frame and restores the \a window attributes.

    This convenience function calls extendFrameIntoClientArea() with margins set
    to 0.

    \note You must unset the Qt::WA_NoSystemBackground attribute for
    extendFrameIntoClientArea() to work.

    \sa extendFrameIntoClientArea()
 */
void QtWin::resetExtendedFrame(QWindow *window)
{
    Q_ASSERT_X(window, Q_FUNC_INFO, "window is null");
    qt_ExtendFrameIntoClientArea(window, 0, 0, 0, 0);
}

/*!
    \fn void QtWin::enableBlurBehindWindow(QWidget *window, const QRegion &region)
    \since 5.2
    \overload QtWin::enableBlurBehindWindow()
 */

/*!
    \since 5.2

    Enables the blur effect for the specified \a region of the specified
    \a window.

    \sa disableBlurBehindWindow()
 */
void QtWin::enableBlurBehindWindow(QWindow *window, const QRegion &region)
{
    Q_ASSERT_X(window, Q_FUNC_INFO, "window is null");

    qtDwmApiDll.init();
    if (!qtDwmApiDll.dwmEnableBlurBehindWindow)
        return;

    qt_DWM_BLURBEHIND dwmbb = {0, 0, 0, 0};
    dwmbb.dwFlags = qt_DWM_BB_ENABLE;
    dwmbb.fEnable = TRUE;
    HRGN rgn = 0;
    if (!region.isNull()) {
        rgn = toHRGN(region);
        if (rgn) {
            dwmbb.hRgnBlur = rgn;
            dwmbb.dwFlags |= qt_DWM_BB_BLURREGION;
        }
    }
    qtDwmApiDll.dwmEnableBlurBehindWindow(reinterpret_cast<HWND>(window->winId()), &dwmbb);
    if (rgn)
        DeleteObject(rgn);
}

/*!
    \fn void QtWin::enableBlurBehindWindow(QWidget *window)
    \since 5.2
    \overload QtWin::enableBlurBehindWindow()
 */

/*!
    \since 5.2

    Enables the blur effect for the specified \a window.

    \sa disableBlurBehindWindow()
 */
void QtWin::enableBlurBehindWindow(QWindow *window)
{
    QtWin::enableBlurBehindWindow(window, QRegion());
}

/*!
    \fn void QtWin::disableBlurBehindWindow(QWidget *window)
    \since 5.2
    \overload QtWin::disableBlurBehindWindow()
 */

/*!
    \since 5.2

    Disables the previously enabled blur effect for the specified \a window.

    \sa enableBlurBehindWindow()
 */
void QtWin::disableBlurBehindWindow(QWindow *window)
{
    Q_ASSERT_X(window, Q_FUNC_INFO, "window is null");
    qt_DWM_BLURBEHIND dwmbb = {0, 0, 0, 0};
    dwmbb.dwFlags = qt_DWM_BB_ENABLE;
    dwmbb.fEnable = FALSE;
    qtDwmApiDll.init();
    if (qtDwmApiDll.dwmEnableBlurBehindWindow)
        qtDwmApiDll.dwmEnableBlurBehindWindow(reinterpret_cast<HWND>(window->winId()), &dwmbb);
}

/*!
    \since 5.2

    Returns the DWM composition state.
 */
bool QtWin::isCompositionEnabled()
{
    QWinEventFilter::setup();

    BOOL enabled = FALSE;
    qtDwmApiDll.init();
    if (qtDwmApiDll.dwmIsCompositionEnabled)
        qtDwmApiDll.dwmIsCompositionEnabled(&enabled);
    return enabled;
}

/*!
    \since 5.2

    Sets whether the Windows Desktop composition is \a enabled.

    \note The underlying function was declared deprecated as of Windows 8 and
    takes no effect.
 */
void QtWin::setCompositionEnabled(bool enabled)
{
    QWinEventFilter::setup();

    UINT compositionEnabled = enabled;
    qtDwmApiDll.init();
    if (qtDwmApiDll.dwmEnableComposition)
        qtDwmApiDll.dwmEnableComposition(compositionEnabled);
}

/*!
    \since 5.2

    Returns whether the colorization color is an opaque blend.
 */
bool QtWin::isCompositionOpaque()
{
    bool opaque;
    colorizationColor(&opaque);
    return opaque;
}

/*!
    \since 5.2

    Sets the Application User Model ID \a id.

    For more information, see
    \l{http://msdn.microsoft.com/en-us/library/windows/desktop/dd378459.aspx}
    {Application User Model IDs}.
 */
void QtWin::setCurrentProcessExplicitAppUserModelID(const QString &id)
{
    qtShell32Dll.init();
    if (qtShell32Dll.setCurrentProcessExplicitAppUserModelID) {
        QScopedArrayPointer<wchar_t> wid(qt_qstringToNullTerminated(id));
        qtShell32Dll.setCurrentProcessExplicitAppUserModelID(wid.data());
    }
}

/*!
    \internal
 */
ITaskbarList3 *qt_createITaskbarList3()
{
    ITaskbarList3 *pTbList = 0;
    HRESULT result = CoCreateInstance(CLSID_TaskbarList, 0, CLSCTX_INPROC_SERVER, qIID_ITaskbarList3, reinterpret_cast<void **>(&pTbList));
    if (SUCCEEDED(result)) {
        if (FAILED(pTbList->HrInit())) {
            pTbList->Release();
            pTbList = 0;
        }
    }
    return pTbList;
}

/*!
    \internal
 */
ITaskbarList2 *qt_createITaskbarList2()
{
    ITaskbarList3 *pTbList = 0;
    HRESULT result = CoCreateInstance(CLSID_TaskbarList, 0, CLSCTX_INPROC_SERVER, qIID_ITaskbarList2, reinterpret_cast<void **>(&pTbList));
    if (SUCCEEDED(result)) {
        if (FAILED(pTbList->HrInit())) {
            pTbList->Release();
            pTbList = 0;
        }
    }
    return pTbList;
}

/*!
    \fn void QtWin::markFullscreenWindow(QWidget *window, bool fullscreen)
    \since 5.2
    \overload QtWin::markFullscreenWindow()
 */

/*!
    \since 5.2

    Marks the specified \a window as running in the full-screen mode if
    \a fullscreen is true, so that the shell handles it correctly. Otherwise,
    removes the mark.

    \note You do not usually need to call this function, because the Windows
    taskbar always tries to determine whether a window is running in the
    full-screen mode.
 */
void QtWin::markFullscreenWindow(QWindow *window, bool fullscreen)
{
    ITaskbarList2 *pTbList = qt_createITaskbarList2();
    if (pTbList) {
        pTbList->MarkFullscreenWindow(reinterpret_cast<HWND>(window->winId()), fullscreen);
        pTbList->Release();
    }
}

/*!
    \fn void QtWin::taskbarActivateTab(QWidget *window)
    \since 5.2
    \overload QtWin::taskbarActivateTab()
 */

/*!
    \since 5.2

    Activates an item on the taskbar without activating the \a window itself.
 */
void QtWin::taskbarActivateTab(QWindow *window)
{
    ITaskbarList *pTbList = qt_createITaskbarList2();
    if (pTbList) {
        pTbList->ActivateTab(reinterpret_cast<HWND>(window->winId()));
        pTbList->Release();
    }
}

/*!
    \fn void QtWin::taskbarActivateTabAlt(QWidget *window)
    \since 5.2
    \overload QtWin::taskbarActivateTabAlt()
 */

/*!
    \since 5.2

    Marks the item that represents the specified \a window on the taskbar
    as active, but does not activate it visually.
 */
void QtWin::taskbarActivateTabAlt(QWindow *window)
{
    ITaskbarList *pTbList = qt_createITaskbarList2();
    if (pTbList) {
        pTbList->SetActiveAlt(reinterpret_cast<HWND>(window->winId()));
        pTbList->Release();
    }
}

/*!
    \fn void QtWin::taskbarAddTab(QWidget *window)
    \since 5.2
    \overload QtWin::taskbarAddTab()
 */

/*!
    \since 5.2

    Adds an item for the specified \a window to the taskbar.
 */
void QtWin::taskbarAddTab(QWindow *window)
{
    ITaskbarList *pTbList = qt_createITaskbarList2();
    if (pTbList) {
        pTbList->AddTab(reinterpret_cast<HWND>(window->winId()));
        pTbList->Release();
    }
}

/*!
    \fn void QtWin::taskbarDeleteTab(QWidget *window)
    \since 5.2
    \overload QtWin::taskbarDeleteTab()
 */

/*!
    \since 5.2

    Removes the specified \a window from the taskbar.
 */
void QtWin::taskbarDeleteTab(QWindow *window)
{
    ITaskbarList *pTbList = qt_createITaskbarList3();
    if (pTbList) {
        pTbList->DeleteTab(reinterpret_cast<HWND>(window->winId()));
        pTbList->Release();
    }
}

/*!
    \enum QtWin::HBitmapFormat

    \since 5.2

    This enum defines how the conversion between \c
    HBITMAP and QPixmap is performed.

    \value  HBitmapNoAlpha
    The alpha channel is ignored and always treated as
    being set to fully opaque. This is preferred if the \c HBITMAP is
    used with standard GDI calls, such as \c BitBlt().

    \value HBitmapPremultipliedAlpha
    The \c HBITMAP is treated as having an
    alpha channel and premultiplied colors. This is preferred if the
    \c HBITMAP is accessed through the \c AlphaBlend() GDI function.

    \value HBitmapAlpha
    The \c HBITMAP is treated as having a plain alpha
    channel. This is the preferred format if the \c HBITMAP is going
    to be used as an application icon or a systray icon.

    \sa fromHBITMAP(), toHBITMAP()
*/

/*!
    \enum QtWin::WindowFlip3DPolicy

    \since 5.2

    This enum type specifies the Flip3D window policy.

    \value  FlipDefault
            Let the OS decide whether to include the window in the Flip3D
            rendering.

    \value  FlipExcludeAbove
            Exclude the window from Flip3D and display it above the Flip3D
            rendering.

    \value  FlipExcludeBelow
            Exclude the window from Flip3D and display it below the Flip3D
            rendering.

    \sa setWindowFlip3DPolicy()
 */

QT_END_NAMESPACE
