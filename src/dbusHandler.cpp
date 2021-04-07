/*
 *  Copyright (C) 2021 Leandro Nini
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include "dbusHandler.h"

#include "mediaplayer2adaptor.h"

#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif

static QStringList mimeTypes = QStringList()
#if defined HAVE_MPG123 || defined HAVE_LIBAVFORMAT_AVFORMAT_H
    << "audio/mpeg"
    << "audio/mpeg3"
    << "audio/x-mpeg-3"
#endif
#if defined HAVE_VORBIS || defined HAVE_LIBAVFORMAT_AVFORMAT_H
    << "audio/ogg"
    << "audio/vorbis"
#endif
#if defined HAVE_OPUS || defined HAVE_LIBAVFORMAT_AVFORMAT_H
    << "audio/opus"
#endif
#ifdef HAVE_OPENMPT
    << "audio/it"
    << "audio/s3m"
    << "audio/xm"
#endif
#ifdef HAVE_SIDPLAYFP
    << "audio/prs.sid"
    << "audio/x-psid"
#endif
#ifdef HAVE_WAVPACK
    << "audio/x-wv"
#endif
#ifdef HAVE_LIBMPCDEC
    << "audio/musepack"
    << "audio/x-musepack"
#endif
#if defined HAVE_SNDFILE || defined HAVE_LIBAVFORMAT_AVFORMAT_H
    << "audio/wav"
    << "audio/aiff"
    << "audio/x-aiff"
    << "audio/x-au"
    << "audio/x-wav"
#endif
#ifdef HAVE_LIBAVFORMAT_AVFORMAT_H
    << "audio/x-realaudio"
    << "audio/x-pn-realaudio"
#endif
;

// MediaPlayer2 properties
bool dbusHandler::getCanQuit() const { return true; }
bool dbusHandler::getCanRaise() const { return true; }
bool dbusHandler::getHasTrackList() const { return false; }
QString dbusHandler::getIdentity() const { return QApplication::applicationName(); }
QStringList dbusHandler::getSupportedUriSchemes() const { return QStringList("file"); }
QStringList dbusHandler::getSupportedMimeTypes() const { return mimeTypes; }

// MediaPlayer2 methods
dbusHandler::dbusHandler(QObject* parent) :
    QObject(parent)
{
    new MediaPlayer2Adaptor(this);

    QDBusConnection dbus = QDBusConnection::sessionBus();
    dbus.registerService("org.mpris.MediaPlayer2.musiqt");
    dbus.registerObject("/org/mpris/MediaPlayer2", this);
}

void dbusHandler::Raise()
{
    const QWidgetList topLevelWidgets = QApplication::topLevelWidgets();
    for (QWidget *widget : topLevelWidgets)
    {
        if (widget->isWindow())
        {
            widget->setWindowState(Qt::WindowActive);
            widget->raise();
        }
    }
}

void dbusHandler::Quit() { QApplication::instance()->quit(); }