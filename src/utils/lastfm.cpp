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

#include "lastfm.h"

#include "player.h"
#include "input/metaData.h"
#include "utils/xdg.h"

#include <lastfm5/misc.h>
#include <lastfm5/ws.h>
#include <lastfm5/Track.h>
#include <lastfm5/XmlQuery.h>

#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QSettings>
#include <QDebug>

QString signature(QString method)
{
    QString sig = QString("api_key%1method%2token%3").arg(lastfm::ws::ApiKey, method, lastfm::ws::SharedSecret);
    return lastfm::md5(sig.toUtf8());
}

/*****************************************************************/

lastfmScrobbler::lastfmScrobbler(player* p, QObject* parent) :
    QObject(parent),
    m_scrobbler("mqt"),
    m_player(p)
{
    QSettings settings;
    QString userName = settings.value("Last.fm Settings/User Name", QString()).toString();
    QString sessionKey = settings.value("Last.fm Settings/Session Key", QString()).toString();

    m_timer.setSingleShot(true);
    connect(&m_timer, &QTimer::timeout, this, &lastfmScrobbler::scrobble);

    lastfm::ws::ApiKey          = "5cfc6d1c438c0bb0d9f2e1d6d2abd9fd";
    lastfm::ws::SharedSecret    = "5785cfb120eee3142c18bd3901494d8b";
    lastfm::ws::Username        = userName;
    lastfm::ws::SessionKey      = sessionKey;

    connect(m_player, &player::stateChanged, this, &lastfmScrobbler::stateChanged);
    //connect(m_player, &player::songEnded, this, &lastfmScrobbler::songEnded);
}

lastfmScrobbler::~lastfmScrobbler()
{
    // submit any pending scrobble
    m_scrobbler.submit();
}

void lastfmScrobbler::stateChanged()
{
    switch (m_player->state())
    {
    case state_t::PLAY:
        // FIXME this is not called on gapless playback
        nowPlaying();
        break;
    case state_t::PAUSE:
        m_timer.stop();
        break;
    case state_t::STOP:
        // TODO if past scrobblePoint do scrobble
        m_timer.stop();
        m_track.reset(nullptr);
        break;
    default:
        break;
    }
}

void lastfmScrobbler::nowPlaying()
{
    if (lastfm::ws::SessionKey.isEmpty())
        return;

    // submit any pending scrobble
    m_scrobbler.submit();

    int songDuration = m_player->songDuration()/1000;
    // The track must be longer than 30 seconds.
    if (songDuration < 30)
        return;

    int scrobblePoint = m_player->songDuration()/2;
    // The track has been played for at least half its duration, or for 4 minutes
    constexpr int limit = 4 * 60 * 1000;
    if (scrobblePoint > limit)
            scrobblePoint = limit;

    if (m_player->seconds() == 0)
    {
        // playback is starting
        const metaData* data = m_player->getMetaData();
        lastfm::MutableTrack *track = new lastfm::MutableTrack;
        track->setArtist(data->getInfo(metaData::ARTIST));
        track->setAlbum(data->getInfo(metaData::ALBUM));
        track->setTitle(data->getInfo(metaData::TITLE));
        track->setDuration(songDuration);
        track->stamp(); //sets track start time
        m_track.reset(track);

        m_scrobbler.nowPlaying(*m_track);

        m_timer.setInterval(scrobblePoint);
        m_timer.start();
    }
    else
    {
        // unpausing
        if (m_track.data() != nullptr)
        {
            m_timer.setInterval(scrobblePoint - m_player->seconds());
            m_timer.start();
        }
    }
}

void lastfmScrobbler::scrobble()
{
    qDebug() << "Caching scrobble";
    m_scrobbler.cache(*m_track);
    m_track.reset(nullptr);
}

/*****************************************************************/

lastfmConfig::lastfmConfig(QWidget* win) :
    configFrame(win)
{
    QLabel* label = new QLabel(tr("Session:"), this);
    matrix()->addWidget(label);
    QLineEdit* lineEdit = new QLineEdit(tr("none/active"), this); // FIXME
    lineEdit->setReadOnly(true);
    matrix()->addWidget(lineEdit, 0, 1);
    QPushButton* button = new QPushButton(tr("Authenticate"), this);
    button->setToolTip("Get session key from Last.fm");
    matrix()->addWidget(button, 1, 1);
    connect(button, &QPushButton::clicked, this, &lastfmConfig::auth);
}

void lastfmConfig::auth()
{
    // Fetch a request token
    QMap<QString, QString> params;
    params["method"] = "auth.getToken";
    params["api_key"] = lastfm::ws::ApiKey;
    params["api_sig "] = signature("auth.getToken");

    QNetworkReply* reply = lastfm::ws::post(params);
    QObject::connect(reply, &QNetworkReply::finished, this, &lastfmConfig::gotToken);
}

void lastfmConfig::gotToken()
{
    QNetworkReply* reply = static_cast<QNetworkReply*>(sender());
    QString token;

    lastfm::XmlQuery query;
    if (!query.parse(reply))
    {
        qWarning() << query.parseError().message();
        return;
    }
    token = query["token"].text();
    qDebug() << token;

    // Request authorization from the user
    QString link = QString("http://www.last.fm/api/auth/?api_key=%1&token=%2").arg(lastfm::ws::ApiKey, token);
    xdg::open(link);

    // Wait before asking for session key
    QMessageBox::information(this, tr("Info"), tr("Authorize app on Last.fm site before proceeding"));

    // Fetch A Web Service Session
    QMap<QString, QString> params;
    params["method"] = "auth.getSession";
    params["api_key"] = lastfm::ws::ApiKey;
    params["token"] = token;
    params["api_sig "] = signature("auth.getSession");

    reply = lastfm::ws::post(params);
    QObject::connect(reply, &QNetworkReply::finished,
        [reply]()
        {
            lastfm::XmlQuery query;
            if (query.parse(reply))
            {
                QString userName = query["session"]["name"].text();
                QString sessionKey = query["session"]["key"].text();
                lastfm::ws::Username = userName;
                lastfm::ws::SessionKey = sessionKey;
                qDebug() << userName << " - " << sessionKey;

                QSettings settings;
                settings.setValue("Last.fm Settings/User Name", userName);
                settings.setValue("Last.fm Settings/Session Key", sessionKey);
            }
            else
            {
                qWarning() << query.parseError().message();
            }
        }
    );
}
