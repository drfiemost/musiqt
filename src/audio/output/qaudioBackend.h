/*
 *  Copyright (C) 2006-2021 Leandro Nini
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

#ifndef QAUDIOBACKEND_H
#define QAUDIOBACKEND_H

#include "inputTypes.h"
#include "AudioOutputWrapper.h"

#include <QAudio>
#include <QPointer>
#include <QRunnable>
#include <QThread>

/*****************************************************************/

class deviceLoader : public QRunnable
{
public:
    void run() override;
};

/*****************************************************************/

/**
 * QAudio output backend
 */
class qaudioBackend : public QObject
{
    Q_OBJECT

private:
    QPointer<AudioOutputWrapper> m_audioOutput;

    // audio thread
    QThread *m_thread;

signals:
    void songEnded();

private:
    void onStateChange(QAudio::State newState);

public:
    qaudioBackend();

    virtual ~qaudioBackend();

    static QStringList getDevices();

    /// Open
    size_t open(int card, audioFormat_t format, QIODevice* device, audioFormat_t& outputFormat);

    /// Close
    void close();

    /// Pause
    void pause();
    void unpause();

    /// Stop
    void stop();

    /// Set volume
    void setVolume(int vol);

    /// Get volume
    int getVolume();
};

#endif
