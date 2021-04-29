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

#ifndef LASTFM_H
#define LASTFM_H

#include <lastfm5/Audioscrobbler.h>

#include <QObject>
#include <QScopedPointer>
#include <QString>
#include <QTimer>

class player;
class metaData;

/*****************************************************************/

class lastfmScrobbler : public QObject
{
    Q_OBJECT

public:
    lastfmScrobbler(player* p, QObject* parent = nullptr);
    ~lastfmScrobbler();

    void nowPlaying();

private:
    void stateChanged();
    void scrobble();

private:
    lastfm::Audioscrobbler m_scrobbler;
    QScopedPointer<lastfm::Track> m_track;
    player *m_player;
    QTimer m_timer;
};

/*****************************************************************/

#include "configFrame.h"

class lastfmConfig : public configFrame
{
private:
    lastfmConfig() {}
    lastfmConfig(const lastfmConfig&);
    lastfmConfig& operator=(const lastfmConfig&);

private:
    void auth();
    void gotToken();

public:
    lastfmConfig(QWidget* win);
    virtual ~lastfmConfig() {}
};

#endif