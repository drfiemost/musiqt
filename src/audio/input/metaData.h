/*
 *  Copyright (C) 2010-2021 Leandro Nini
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

#ifndef METADATA_H
#define METADATA_H

#include "utils.h"

#include <QHash>

class metaData
{
public:
    // https://www.freedesktop.org/wiki/Specifications/mpris-spec/metadata/
    enum mpris_t
    {
        // MPRIS-specific
        TRACK_ID,
        LENGTH,
        ART_URL,
        // Common Xesam properties
        ALBUM,
        ALBUM_ARTIST,
        ARTIST,
        AS_TEXT,
        AUDIO_BPM,
        AUTO_RATING,
        COMMENT,
        COMPOSER,
        CONTENT_CREATED,
        DISC_NUMBER,
        FIRST_USED,
        GENRE,
        LAST_USED,
        LYRICIST,
        TITLE,
        TRACK_NUMBER,
        URL,
        USER_COUNT,
        USER_RATING,
        LAST_ID
    };

public:
    virtual int moreInfo(const int i) const =0;
    virtual QString getKey(const unsigned int num) const =0;
    virtual QString getKey(const mpris_t info) const =0;
    virtual QString getInfo(unsigned int num) const =0;
    virtual QString getInfo(const char* info) const =0;
    virtual QString getInfo(const mpris_t info) const =0;
    virtual QByteArray* getImage(void) const =0;

protected:
    ~metaData() {}
};

/*****************************************************************/

class metaDataImpl : public metaData
{
private:
    typedef QHash<QString, QString> StringDict;
private:
    static const char* mprisTags[LAST_ID];

private:
    StringDict m_infos;

    QByteArray *m_img;

public:
    metaDataImpl() : m_img(nullptr) {}
    ~metaDataImpl() { delete m_img; }

    /// Append song info
    void addInfo(QString type, QString info);
    void addInfo(QString type, unsigned int info);
    void addInfo(const mpris_t type, QString info);
    void addInfo(const mpris_t type, unsigned int info);
    void addInfo(QByteArray* img) { m_img = img; }

    /// Remove all info
    void clearInfo() { m_infos.clear(); utils::delPtr(m_img); }

    /// Find song info
    QString findInfo(QString type) const { return m_infos.find(type).value(); }

    /// Get song info
    int moreInfo(const int i) const;

    QString getKey(const unsigned int num) const { return m_infos.keys()[num]; }
    QString getKey(const mpris_t info) const { return mprisTags[info]; }
    QString getInfo(unsigned int num) const { return m_infos.values()[num]; }
    QString getInfo(const char* info) const
    {
        auto it = m_infos.find(info);
        return it != m_infos.end() ? it.value() : QString();
    }
    QString getInfo(const mpris_t info) const { return getInfo(mprisTags[info]); }
    QByteArray* getImage() const { return m_img; }
};

#endif
