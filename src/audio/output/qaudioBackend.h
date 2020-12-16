/*
 *  Copyright (C) 2006-2020 Leandro Nini
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

#include <QAudioOutput>

#include <memory>

/*****************************************************************/

/**
 * QAudio output backend
 */
class qaudioBackend
{
private:
    QAudioOutput *_audioOutput;

public:
    qaudioBackend();

    virtual ~qaudioBackend() {};

    static QStringList devices();

    /// Open
    size_t open(const unsigned int card, unsigned int &sampleRate, const unsigned int channels, const unsigned int prec, QIODevice* device);

    /// Close
    void close();

    /// Pause
    void pause();
    void unpause();

    /// Stop
    void stop();

    /// Set volume
    void volume(int vol);

    /// Get volume
    int volume();
};

#endif
