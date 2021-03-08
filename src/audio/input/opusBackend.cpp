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

#include "opusBackend.h"

#include "settings.h"
#include "utils.h"

#include <string.h>

#include <QDebug>
#include <QComboBox>
#include <QLabel>

extern const unsigned char iconOpus[952] =
{
    0x47, 0x49, 0x46, 0x38, 0x39, 0x61, 0x10, 0x00, 0x10, 0x00, 0xf5, 0x3f, 0x00, 0x8c, 0x8c, 0x8c,
    0x92, 0x92, 0x92, 0x93, 0x93, 0x93, 0x94, 0x94, 0x94, 0x95, 0x95, 0x95, 0x96, 0x96, 0x96, 0x97,
    0x97, 0x97, 0x99, 0x99, 0x99, 0x9a, 0x9a, 0x9a, 0x9b, 0x9b, 0x9b, 0x9c, 0x9c, 0x9c, 0x9d, 0x9d,
    0x9d, 0x9e, 0x9e, 0x9e, 0x9f, 0x9f, 0x9f, 0xa0, 0xa0, 0xa0, 0xa1, 0xa1, 0xa1, 0xa2, 0xa2, 0xa2,
    0xa3, 0xa3, 0xa3, 0xa4, 0xa4, 0xa4, 0xa5, 0xa5, 0xa5, 0xa6, 0xa6, 0xa6, 0xa8, 0xa8, 0xa8, 0xa9,
    0xa9, 0xa9, 0xaa, 0xaa, 0xaa, 0xab, 0xab, 0xab, 0xac, 0xac, 0xac, 0xad, 0xad, 0xad, 0xb0, 0xb0,
    0xb0, 0xb1, 0xb1, 0xb1, 0xb4, 0xb4, 0xb4, 0xb8, 0xb8, 0xb8, 0xbc, 0xbc, 0xbc, 0xbd, 0xbd, 0xbd,
    0xbe, 0xbe, 0xbe, 0xc0, 0xc0, 0xc0, 0xc1, 0xc1, 0xc1, 0xc2, 0xc2, 0xc2, 0xc5, 0xc5, 0xc5, 0xc7,
    0xc7, 0xc7, 0xc9, 0xc9, 0xc9, 0xce, 0xce, 0xce, 0xd1, 0xd1, 0xd1, 0xd3, 0xd3, 0xd3, 0xd5, 0xd5,
    0xd5, 0xd7, 0xd7, 0xd7, 0xdc, 0xdc, 0xdc, 0xdf, 0xdf, 0xdf, 0xe2, 0xe2, 0xe2, 0xe5, 0xe5, 0xe5,
    0xe6, 0xe6, 0xe6, 0xe7, 0xe7, 0xe7, 0xeb, 0xeb, 0xeb, 0xee, 0xee, 0xee, 0xf0, 0xf0, 0xf0, 0xf1,
    0xf1, 0xf1, 0xf2, 0xf2, 0xf2, 0xf4, 0xf4, 0xf4, 0xf5, 0xf5, 0xf5, 0xf7, 0xf7, 0xf7, 0xf9, 0xf9,
    0xf9, 0xfa, 0xfa, 0xfa, 0xfd, 0xfd, 0xfd, 0xfe, 0xfe, 0xfe, 0xff, 0xff, 0xff, 0x21, 0xf9, 0x04,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x21, 0xff, 0x0b, 0x49, 0x43, 0x43, 0x52, 0x47, 0x42, 0x47, 0x31,
    0x30, 0x31, 0x32, 0xff, 0x00, 0x00, 0x02, 0x54, 0x6c, 0x63, 0x6d, 0x73, 0x04, 0x30, 0x00, 0x00,
    0x6d, 0x6e, 0x74, 0x72, 0x52, 0x47, 0x42, 0x20, 0x58, 0x59, 0x5a, 0x20, 0x07, 0xe1, 0x00, 0x06,
    0x00, 0x16, 0x00, 0x06, 0x00, 0x1b, 0x00, 0x10, 0x61, 0x63, 0x73, 0x70, 0x41, 0x50, 0x50, 0x4c,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xf6, 0xd6, 0x00, 0x01, 0x00, 0x00,
    0x00, 0x00, 0xd3, 0x2d, 0x6c, 0x63, 0x6d, 0x73, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0b, 0x64, 0x65, 0x73, 0x63, 0x00, 0x00, 0x01, 0x08,
    0x00, 0x00, 0x00, 0x3e, 0x63, 0x70, 0x72, 0x74, 0x00, 0x00, 0x01, 0x48, 0x00, 0x00, 0x00, 0x4c,
    0x77, 0x74, 0x70, 0x74, 0x00, 0x00, 0x01, 0x94, 0x00, 0x00, 0x00, 0x14, 0x63, 0x68, 0x61, 0x64,
    0x00, 0x00, 0x01, 0xa8, 0x00, 0x00, 0x00, 0x2c, 0x72, 0x58, 0x59, 0x5a, 0x00, 0x00, 0x01, 0xd4,
    0x00, 0x00, 0x00, 0x14, 0x62, 0x58, 0x59, 0x5a, 0x00, 0x00, 0x01, 0xe8, 0x00, 0x00, 0x00, 0x14,
    0x67, 0x58, 0x59, 0x5a, 0x00, 0x00, 0x01, 0xfc, 0x00, 0x00, 0x00, 0x14, 0x72, 0x54, 0x52, 0x43,
    0x00, 0x00, 0x02, 0x10, 0x00, 0x00, 0x00, 0x20, 0x67, 0x54, 0x52, 0x43, 0x00, 0x00, 0x02, 0x10,
    0x00, 0x00, 0x00, 0x20, 0x62, 0x54, 0x52, 0x43, 0x00, 0x00, 0x02, 0x10, 0x00, 0x00, 0x00, 0x20,
    0x63, 0x68, 0x72, 0xff, 0x6d, 0x00, 0x00, 0x02, 0x30, 0x00, 0x00, 0x00, 0x24, 0x6d, 0x6c, 0x75,
    0x63, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x0c, 0x65, 0x6e, 0x55,
    0x53, 0x00, 0x00, 0x00, 0x22, 0x00, 0x00, 0x00, 0x1c, 0x00, 0x73, 0x00, 0x52, 0x00, 0x47, 0x00,
    0x42, 0x00, 0x20, 0x00, 0x49, 0x00, 0x45, 0x00, 0x43, 0x00, 0x36, 0x00, 0x31, 0x00, 0x39, 0x00,
    0x36, 0x00, 0x36, 0x00, 0x2d, 0x00, 0x32, 0x00, 0x2e, 0x00, 0x31, 0x00, 0x00, 0x6d, 0x6c, 0x75,
    0x63, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x0c, 0x65, 0x6e, 0x55,
    0x53, 0x00, 0x00, 0x00, 0x30, 0x00, 0x00, 0x00, 0x1c, 0x00, 0x4e, 0x00, 0x6f, 0x00, 0x20, 0x00,
    0x63, 0x00, 0x6f, 0x00, 0x70, 0x00, 0x79, 0x00, 0x72, 0x00, 0x69, 0x00, 0x67, 0x00, 0x68, 0x00,
    0x74, 0x00, 0x2c, 0x00, 0x20, 0x00, 0x75, 0x00, 0x73, 0x00, 0x65, 0x00, 0x20, 0x00, 0x66, 0x00,
    0x72, 0x00, 0x65, 0x00, 0x65, 0x00, 0x6c, 0x00, 0x79, 0x58, 0x59, 0x5a, 0x20, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0xf6, 0xd6, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0xd3, 0x2d, 0x73, 0x66, 0x33,
    0x32, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x0c, 0x42, 0x00, 0x00, 0x05, 0xde, 0xff, 0xff, 0xf3,
    0x25, 0x00, 0x00, 0x07, 0x93, 0x00, 0x00, 0xfd, 0x90, 0xff, 0xff, 0xfb, 0xa1, 0xff, 0xff, 0xfd,
    0xa2, 0x00, 0x00, 0x03, 0xdc, 0x00, 0x00, 0xc0, 0x6e, 0x58, 0x59, 0x5a, 0x20, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x6f, 0xa0, 0x00, 0x00, 0x38, 0xf5, 0x00, 0x00, 0x03, 0x90, 0x58, 0x59, 0x5a,
    0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x24, 0x9f, 0x00, 0x00, 0x0f, 0x84, 0x00, 0x00, 0xb6,
    0xc3, 0x58, 0x59, 0x56, 0x5a, 0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x62, 0x97, 0x00, 0x00,
    0xb7, 0x87, 0x00, 0x00, 0x18, 0xd9, 0x70, 0x61, 0x72, 0x61, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03,
    0x00, 0x00, 0x00, 0x02, 0x66, 0x66, 0x00, 0x00, 0xf2, 0xa7, 0x00, 0x00, 0x0d, 0x59, 0x00, 0x00,
    0x13, 0xd0, 0x00, 0x00, 0x0a, 0x5b, 0x63, 0x68, 0x72, 0x6d, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03,
    0x00, 0x00, 0x00, 0x00, 0xa3, 0xd7, 0x00, 0x00, 0x54, 0x7b, 0x00, 0x00, 0x4c, 0xcd, 0x00, 0x00,
    0x99, 0x9a, 0x00, 0x00, 0x26, 0x66, 0x00, 0x00, 0x0f, 0x5c, 0x00, 0x2c, 0x00, 0x00, 0x00, 0x00,
    0x10, 0x00, 0x10, 0x00, 0x00, 0x06, 0x6f, 0xc0, 0x9f, 0x70, 0x48, 0x2c, 0x1a, 0x8f, 0xc8, 0x24,
    0x52, 0x07, 0x73, 0xb5, 0x5e, 0x37, 0xda, 0x11, 0x26, 0xe2, 0x68, 0x34, 0x13, 0xd1, 0xca, 0xf2,
    0x71, 0x11, 0x53, 0x13, 0x8b, 0xc6, 0x62, 0x6d, 0x7d, 0x2c, 0x96, 0x87, 0xf7, 0xf7, 0x7a, 0xa0,
    0xb3, 0xba, 0x1d, 0x2c, 0x9c, 0xfe, 0x08, 0x45, 0x8c, 0xc9, 0x63, 0x72, 0x13, 0xa2, 0xf2, 0x13,
    0x09, 0x27, 0x42, 0x1d, 0x09, 0x0c, 0x87, 0x2b, 0x34, 0x2b, 0x0f, 0x0c, 0x0f, 0x09, 0x0f, 0x7d,
    0x3f, 0x26, 0x02, 0x0c, 0x09, 0x06, 0x02, 0x97, 0x06, 0x97, 0x13, 0x30, 0x43, 0x34, 0x97, 0x79,
    0x95, 0x8d, 0x1d, 0x2b, 0x3b, 0x45, 0x2e, 0x00, 0x26, 0x3f, 0x37, 0x3a, 0x3a, 0x4a, 0x33, 0x52,
    0x4a, 0xb2, 0xb3, 0xb4, 0xb3, 0x41, 0x00, 0x3b
};

OpusFileCallbacks opusBackend::opus_callbacks =
{
    opusBackend::read_func,
    opusBackend::seek_func,
    opusBackend::tell_func,
    opusBackend::close_func
};

#define EXT "opus"

#define CREDITS "Opus<br>Copyright \302\251 Xiph.org Foundation"
#define LINK    "http://opus-codec.org/"

const char opusBackend::name[] = "Opus";

opusConfig_t opusBackend::_settings;

/*****************************************************************/

size_t opusBackend::fillBuffer(void* buffer, const size_t bufferSize, const unsigned int milliSeconds)
{
    size_t n = 0;
    int read;
    do {
        read = op_read_stereo(_of, (opus_int16*)buffer+n/2, (bufferSize-n)/2);
        if (read < 0)
        {
            qWarning() << "Decoding error: " << read;
            return 0;
        }
        n += read*4;
    } while (read && (n < bufferSize));

    return n;
}

/*****************************************************************/

QStringList opusBackend::ext() { return QStringList(EXT); }

opusBackend::opusBackend() :
    inputBackend(name, iconOpus, 952),
    _of(nullptr)
{}

opusBackend::~opusBackend()
{
    close();
}

bool opusBackend::compareTag(const char* orig, const char* tag)
{
    int n = qstrlen(tag);
    return qstrnicmp(orig, tag, n);
}

bool opusBackend::open(const QString& fileName)
{
    close();

    _file.setFileName(fileName);
    if (!_file.open(QIODevice::ReadOnly))
        return false;

    int error;
    _of = op_open_callbacks(&_file, &opus_callbacks, nullptr, 0, &error);
    if (_of == nullptr)
    {
        qDebug() << "Error code: " << error;
        _file.close();
        return false;
    }

    time(static_cast<unsigned int>(op_pcm_total(_of, -1)/48));

    QString title;
    QString artist;
    QString year;
    QString album;
    QString genre;
    QString comment;
    QString mime;
    QByteArray image;

    char **ptr = op_tags(_of, -1)->user_comments;
    while (*ptr)
    {
        qDebug() << *ptr;
        if (!getComment(*ptr, &title, "title"))
        if (!getComment(*ptr, &artist, "artist"))
        if (!getComment(*ptr, &year, "date"))
        if (!getComment(*ptr, &album, "album"))
        if (!getComment(*ptr, &genre, "genre"))
        if (!getComment(*ptr, &comment, "comment"))
        {
            if (!compareTag(*ptr, "tracknumber"))
            {
                m_metaData.addInfo(metaData::TRACK, QString(*ptr).mid(12));
            }
            else if (!compareTag(*ptr, "METADATA_BLOCK_PICTURE"))
            {
                qDebug() << "METADATA_BLOCK_PICTURE";
                QByteArray flac_picture = QByteArray::fromBase64(*ptr+23);
            }
            else if (!compareTag(*ptr, "COVERARTMIME"))
            {
                qDebug() << "COVERARTMIME";
                mime = QString(*ptr+13);
            }
            else if (!compareTag(*ptr, "COVERART"))
            {
                qDebug() << "COVERART";
                image = QByteArray::fromBase64(*ptr+9);
            }
            else if (!compareTag(*ptr, "BINARY_COVERART"))
            {
                qDebug() << "BINARY_COVERART";
                QByteArray flac_picture(*ptr+16); // FIXME
            }
        }
        ++ptr;
    }

//METADATA_BLOCK_PICTURE
/*
<32>   The picture type according to the ID3v2 APIC frame:
<32>   The length of the MIME type string in bytes.
<n*8>  The MIME type string
<32>   The length of the description string in bytes.
<n*8>  The description of the picture, in UTF-8.
<32>   The width of the picture in pixels.
<32>   The height of the picture in pixels.
<32>   The color depth of the picture in bits-per-pixel.
<32>   For indexed-color pictures (e.g. GIF), the number of colors used, or 0 for non-indexed pictures.
<32>   The length of the picture data in bytes.
<n*8>  The binary picture data.
*/

//deprecated:
//COVERARTMIME
//COVERART (base64 encoded)

    m_metaData.addInfo(metaData::TITLE, title);
    m_metaData.addInfo(metaData::ARTIST, artist);
    m_metaData.addInfo(metaData::ALBUM, album);
    m_metaData.addInfo(metaData::GENRE, genre);
    m_metaData.addInfo(metaData::YEAR, year);
    m_metaData.addInfo(metaData::COMMENT, comment);

    if (!mime.isNull())
        m_metaData.addInfo(new QByteArray((char*)image.data(), image.size()));

    songLoaded(fileName);
    return true;
}

bool opusBackend::getComment(const char* orig, QString* dest, const char* type)
{
    const int n = qstrlen(type);
    if (qstrnicmp(orig, type, n))
        return false;

    if (orig[n] != '=')
        return false;

    if (!dest->isEmpty())
        dest->append(", ");

    dest->append(QString::fromUtf8(orig+n+1));
    return true;
}

void opusBackend::close()
{
    if (_of != nullptr)
    {
        op_free(_of);
        //utils::delPtr(_of);
        _file.close();
    }

    songLoaded(QString());
}

bool opusBackend::seek(int pos)
{
    if (_of == nullptr)
        return false;

    ogg_int64_t length = op_pcm_total(_of, -1);

    if (length < 0)
        return false;

    ogg_int64_t offset = (length * pos) / 100;
    if (op_pcm_seek(_of, offset) < 0)
        return false;

    return true;
}

int opusBackend::read_func(void *_stream, unsigned char *_ptr, int _nbytes)
{
    return ((QFile*)_stream)->read((char*)_ptr, _nbytes);
}

int opusBackend::seek_func(void *_stream, opus_int64 _offset, int _whence)
{
    QFile* file = static_cast<QFile*>(_stream);
    qint64 pos;
    switch (_whence)
    {
    case SEEK_SET:
        pos = 0;
        break;
    case SEEK_CUR:
        pos = file->pos();
        break;
    case SEEK_END:
        pos = file->size();
        break;
    default:
        return -1;
    }
    return file->seek(pos + _offset) ? 0 : -1;
}

opus_int64 opusBackend::tell_func(void *_stream)
{
    return ((QFile*)_stream)->pos();
}

int opusBackend::close_func(void *_stream)
{
    ((QFile*)_stream)->close();
    return 0;
}

/*****************************************************************/

opusConfig::opusConfig(QWidget* win) :
    configFrame(win, opusBackend::name, CREDITS, LINK)
{
    matrix()->addWidget(new QLabel(tr("No settings available"), this));
}
