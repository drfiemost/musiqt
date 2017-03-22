/*
 *  Copyright (C) 2007-2017 Leandro Nini
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
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include "wvBackend.h"

#include "settings.h"
#include "utils.h"

#include <QDebug>

/* created by reswrap from file wavpack.gif */
extern const unsigned char iconWv[375] =
{
    0x47,0x49,0x46,0x38,0x39,0x61,0x10,0x00,0x10,0x00,0xf5,0x00,0x00,0x00,0x00,0x00,
    0x00,0x01,0x00,0x03,0x04,0x06,0x04,0x06,0x08,0x05,0x06,0x08,0x07,0x09,0x0d,0x07,
    0x0a,0x0e,0x07,0x0a,0x0f,0x07,0x0b,0x0f,0x07,0x0b,0x10,0x08,0x0b,0x10,0x0a,0x0c,
    0x14,0x0b,0x10,0x17,0x0e,0x14,0x1d,0x0d,0x15,0x1f,0x0e,0x16,0x1f,0x10,0x17,0x22,
    0x13,0x1c,0x28,0x14,0x1e,0x2b,0x15,0x1e,0x2b,0x17,0x21,0x30,0x19,0x25,0x37,0x1a,
    0x24,0x36,0x1e,0x2c,0x41,0x20,0x2f,0x45,0x21,0x2f,0x45,0x20,0x30,0x45,0x26,0x38,
    0x51,0x2a,0x3c,0x57,0x2e,0x42,0x61,0x30,0x45,0x65,0x34,0x4d,0x70,0x36,0x4f,0x72,
    0x38,0x51,0x77,0x39,0x52,0x77,0x3e,0x5a,0x83,0x3e,0x5c,0x87,0x41,0x5f,0x8b,0x44,
    0x64,0x91,0x45,0x67,0x95,0x47,0x69,0x98,0x48,0x6a,0x99,0x4a,0x6c,0x9e,0x4b,0x6d,
    0x9f,0x4b,0x6e,0xa2,0x4d,0x72,0xa5,0x4f,0x73,0xa6,0x4f,0x74,0xaa,0x50,0x76,0xab,
    0x52,0x77,0xad,0x52,0x77,0xae,0x52,0x79,0xae,0x56,0x7f,0xb9,0x5e,0x87,0xc4,0x61,
    0x8e,0xce,0x61,0x8f,0xd0,0x62,0x8f,0xd0,0x61,0x90,0xd0,0x62,0x90,0xd1,0x63,0x90,
    0xd2,0x62,0x91,0xd2,0x63,0x91,0xd3,0x64,0x93,0xd4,0x00,0x00,0x00,0x21,0xf9,0x04,
    0x01,0x00,0x00,0x3f,0x00,0x2c,0x00,0x00,0x00,0x00,0x10,0x00,0x10,0x00,0x00,0x06,
    0x94,0xc0,0x9f,0xc1,0xa0,0x50,0x24,0x0c,0x89,0x64,0x51,0x31,0x14,0xda,0x6c,0xa1,
    0xd6,0x13,0xf5,0xc4,0x7d,0x64,0x4f,0xe2,0x53,0x10,0xb1,0xe1,0x02,0x2a,0x5b,0x29,
    0xf0,0xb4,0x25,0x14,0x4f,0x0d,0xf9,0x13,0x18,0xd9,0x02,0x96,0xa7,0x4f,0x0b,0x25,
    0x07,0x02,0x9c,0x19,0xd9,0xd6,0xf3,0x9d,0x9f,0x2a,0x01,0x1d,0x0b,0x1e,0x0e,0x13,
    0x18,0x36,0x3a,0x59,0x68,0x4f,0x77,0x30,0x29,0x77,0x2c,0x65,0x66,0x8c,0x6f,0x0d,
    0x36,0x35,0x77,0x77,0x24,0x59,0x09,0x65,0x01,0x58,0x34,0x7b,0x04,0x0b,0x4f,0x7f,
    0x8a,0x26,0x4f,0x17,0x04,0x8d,0x88,0x36,0x5a,0x8a,0x38,0x8d,0x1b,0x32,0x14,0x7b,
    0x94,0x93,0x36,0x28,0x9a,0x71,0x65,0x74,0x5e,0x36,0x86,0xc0,0xc3,0x7f,0x65,0x5f,
    0x27,0x5e,0x3d,0x3a,0x3d,0x59,0x06,0x72,0x36,0x2d,0x10,0xb9,0xbe,0x42,0x43,0x48,
    0x4c,0xd6,0xd9,0x3f,0x41,0x00,0x3b
};

#define EXT "wv"

#define	BUFFER_LENGTH 4096

#define CREDITS "WAVPACK\nHybrid Lossless Wavefile Compressor\nCopyright \302\251 Conifer Software."
#define LINK    "http://www.wavpack.com/"

const char wvBackend::name[] = "Wavpack";

/*****************************************************************/

size_t wvBackend::fillBuffer(void* buffer, const size_t bufferSize, const unsigned int seconds)
{
    size_t n = 0;
    const unsigned int sampleSize =
        (_precision == sample_t::U8) ? 1 : (_precision == sample_t::S16) ? 2 : 4;

    do {
        if (_bufOffset >= _bufSize)
        {
            _bufSize = WavpackUnpackSamples(_wvContext, _decodeBuf, BUFFER_LENGTH) * _channels;
            if (_bufSize == 0)
            {
                return n;
            }
            _bufOffset = 0;
        }
        const size_t size = qMin((size_t)(_bufSize-_bufOffset), (bufferSize-n) / sampleSize);
        copyBuffer((char*)buffer+n, _decodeBuf+_bufOffset, size);
        _bufOffset += size;
        n += size*sampleSize;
    } while (n < bufferSize);

    return bufferSize;
}

void wvBackend::copyBuffer(char* dest, const int* src, size_t length)
{
    switch (_precision)
    {
    case sample_t::U8:
        for (size_t i=0; i<length; i++)
            dest[i] = 128 + (char)(src[i]);
        break;
    case sample_t::S16:
        for (size_t i=0; i<length; i++)
        {
            dest[i<<1] = (char)(src[i]);
            dest[(i<<1)+1] = (char)(src[i]>>8);
        }
        break;
    default:
        memcpy(dest, src, length*4);
    }
}

/*****************************************************************/

bool wvBackend::supports(const QString& fileName)
{
    QRegExp rx("*." EXT);
    rx.setPatternSyntax(QRegExp::Wildcard);
    return rx.exactMatch(fileName);
}

inline QStringList wvBackend::ext() const { return QStringList(EXT); }

wvBackend::wvBackend() :
    inputBackend(name, iconWv, 375),
    _wvContext(0) {}

wvBackend::~wvBackend()
{
    close();
}

bool wvBackend::open(const QString& fileName)
{
    close();

    char tmp[255];
    _wvContext = WavpackOpenFileInput(fileName.toLocal8Bit().constData(), tmp, OPEN_WVC|OPEN_TAGS|OPEN_2CH_MAX|OPEN_NORMALIZE, 0);
    if (!_wvContext)
    {
        qWarning() << "Error - " << tmp;
        return false;
    }

    const int mode = WavpackGetMode(_wvContext);
    _bps = WavpackGetBytesPerSample(_wvContext);
    _channels = WavpackGetReducedChannels(_wvContext);

    _decodeBuf = new int[BUFFER_LENGTH*_channels];

    if (mode & MODE_FLOAT)
    {
        _precision = sample_t::SAMPLE_FLOAT;
    }
    else
    {
        switch (_bps)
        {
        case 1:
            _precision = sample_t::U8;
            break;
        case 2:
            _precision = sample_t::S16;
            break;
        default:
            _precision = sample_t::SAMPLE_FIXED;
        }
    }

    if (mode & MODE_VALID_TAG)
    {
        if (mode & MODE_APETAG)
        {
            // TODO APEv2 text tags can have multiple (NULL separated) strings for a single value
            if (WavpackGetTagItem(_wvContext, "Title", tmp, 255))
                _metaData.addInfo(metaData::TITLE, QString::fromUtf8(tmp));
            if (WavpackGetTagItem(_wvContext, "Artist", tmp, 255))
                _metaData.addInfo(metaData::ARTIST, QString::fromUtf8(tmp));
            if (WavpackGetTagItem(_wvContext, "Album", tmp, 255))
                _metaData.addInfo(metaData::ALBUM, QString::fromUtf8(tmp));
            if (WavpackGetTagItem(_wvContext, "Year", tmp, 255))
                _metaData.addInfo(metaData::YEAR, tmp);
            if (WavpackGetTagItem(_wvContext, "Track", tmp, 255))
                _metaData.addInfo(metaData::TRACK, tmp);
            if (WavpackGetTagItem(_wvContext, "Comment", tmp, 255))
                _metaData.addInfo(metaData::COMMENT, QString::fromUtf8(tmp));
            if (WavpackGetTagItem(_wvContext, "Genre", tmp, 255))
                _metaData.addInfo(metaData::GENRE, QString::fromUtf8(tmp));
        }
        else
        {
            // ID3v1
            if (WavpackGetTagItem(_wvContext, "title", tmp, 255))
                _metaData.addInfo(metaData::TITLE, tmp);
            if (WavpackGetTagItem(_wvContext, "artist", tmp, 255))
                _metaData.addInfo(metaData::ARTIST, tmp);
            if (WavpackGetTagItem(_wvContext, "album", tmp, 255))
                _metaData.addInfo(metaData::ALBUM, tmp);
            if (WavpackGetTagItem(_wvContext, "year", tmp, 255))
                _metaData.addInfo(metaData::YEAR, tmp);
            if (WavpackGetTagItem(_wvContext, "track", tmp, 255))
                _metaData.addInfo(metaData::TRACK, tmp);
            if (WavpackGetTagItem(_wvContext, "comment", tmp, 255))
                _metaData.addInfo(metaData::COMMENT, tmp);
        }
    }

    time(WavpackGetNumSamples(_wvContext) / WavpackGetSampleRate(_wvContext));

    _bufOffset = 0;
    _bufSize = 0;

    songLoaded(fileName);
    return true;
}

void wvBackend::close()
{
    if (_wvContext)
    {
        _wvContext = WavpackCloseFile(_wvContext);
        delete [] _decodeBuf;
    }

    songLoaded(QString::null);
}

bool wvBackend::rewind()
{
    if (!_wvContext)
        return false;

    if (!WavpackSeekSample(_wvContext, 0))
    {
        close();
        return false;
    }

    _bufOffset = 0;
    _bufSize = 0;

    return true;
}

/*****************************************************************/

wvConfig::wvConfig(QWidget* win) :
    configFrame(win, wvBackend::name, CREDITS, LINK)
{
    matrix()->addWidget(new QLabel(tr("No settings available"), this));
}
