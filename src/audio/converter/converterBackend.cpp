/*
 *  Copyright (C) 2009-2020 Leandro Nini
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

#include "converterBackend.h"

#include <QDebug>

resamplerBackend::resamplerBackend(unsigned int srIn, unsigned int srOut, size_t size,
        unsigned int channels, unsigned int inputPrecision, unsigned int outputPrecision) :
    converter(channels, inputPrecision, outputPrecision),
    _data(0)
{
    qDebug() << "Conversion ratio " << (float)(srIn/srOut);

    _rate = (((unsigned int)srIn)<<16)/srOut;
    qDebug() << "_rate " << _rate;

    size_t frames = size / (outputPrecision*channels);
    unsigned long tmp = ((unsigned long)frames * (unsigned long)_rate);
    if (tmp & 0xFFFFll)
        tmp += 0x10000ll;

    bufferSize = (tmp>>16) * (inputPrecision*channels);
    qDebug() << "converter buffer size: " << bufferSize;
    _buffer = new char[bufferSize];
}

resamplerBackend::~resamplerBackend()
{
    delete [] _buffer;
}

/******************************************************************************/

converterBackend::converterBackend(size_t size, unsigned int channels,
                                   unsigned int inputPrecision, unsigned int outputPrecision) :
    converter(channels, inputPrecision, outputPrecision)
{
    bufferSize = size * frameRatio;
    qDebug() << "converter buffer size: " << bufferSize;
    _buffer = new char[bufferSize];
}

converterBackend::~converterBackend()
{
    delete [] _buffer;
}
