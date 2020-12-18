/*
 *  Copyright (C) 2006-2018 Leandro Nini
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

#include "audio.h"

#include "settings.h"
#include "InputWrapper.h"
#include "converter/converterFactory.h"
#include "input/input.h"
#include "output/qaudioBackend.h"

#include <QDebug>
#include <QLabel>
#include <QComboBox>
#include <QStringList>

//#define PROFILE

#if defined(PROFILE) && QT_VERSION >= 0x040700
#  include <QElapsedTimer>

#  define PROFILE_START QElapsedTimer timer; \
    timer.start();
#  define PROFILE_END   qDebug() << "Elapsed " << timer.elapsed();
#else
#  define PROFILE_START
#  define PROFILE_END
#endif

/*****************************************************************/
/*
void audioThread::run()
{
    switch (_audio->outputPrecision())
    {
    case sample_t::U8:
        _audio->loop<quint8>();
        break;
    case sample_t::S16:
        _audio->loop<qint16>();
        break;
    case sample_t::S24:
    case sample_t::S32:
        qWarning() << "Not supported yet";
        break;
    }
}
*/
/*****************************************************************/

/*
template<> inline void audio::process<quint8>(size_t size)
{
#if defined HAVE_BS2B && BS2B_VERSION_MAJOR == 3
    if (_bs2bdp)
    {
        uint8_t *buf = (uint8_t*)_output->buffer();
        bs2b_cross_feed_u8(_bs2bdp, buf, size/2);
    }
#endif
}

template<> inline void audio::process<short>(size_t size)
{
#if (Q_BYTE_ORDER == Q_BIG_ENDIAN)
    {
        //Swap bytes on big endian machines
        quint16 *buf = (quint16*)_output->buffer();
        const quint16 *end = buf+(size/2);
        do {
            const quint16 tmp = *buf;
            *buf++ = ((tmp & 0x00FF)<<8) & ((tmp & 0xFF00)>>8);
        } while (buf<end);
    }
#endif

#ifdef HAVE_BS2B
    if (_bs2bdp)
    {
#if BS2B_VERSION_MAJOR == 3
        int16_t *buf = (int16_t*)_output->buffer();
        bs2b_cross_feed_s16le(_bs2bdp, buf, size/4);
#else
        short *buf = (short*)_output->buffer();
        const short *end = buf+(size/2);
        do {
            bs2b_cross_feed_16(_bs2bdp, buf);
            buf += 2;
        } while (buf < end);
#endif
    }
#endif
}

template<typename T>
void audio::loop()
{
    qDebug() << "loop " << static_cast<int>(sizeof(T)<<3) << " bit";
    while (isPlaying)
    {
PROFILE_START
        size_t size = i->fillBuffer(
            _converter != nullptr ? _converter->buffer() : _output->buffer(),
            _converter != nullptr ? _converter->bufSize() : _bufferSize,
            _seconds);

        if (!size)
        {
            emit songEnded();
            if (_preload)
            {
                i = _preload;
                _preload = nullptr;
                _seconds = 0;
                continue;
            }
            else
                break;
        }

        if (_converter != nullptr)
            size = _converter->convert(_output->buffer(), _bufferSize);

        process<T>(size);
PROFILE_END

        if (!_output->write(_output->buffer(), _bufferSize) && isPlaying)
        {
            qWarning() << "Output error";
            emit outputError();
            isPlaying = false;
            break;
        }

        _buffers -= (1<<DECIMALS);
        if (_buffers < 0)
        {
            do {
                _buffers += _bufPerSec;
                _seconds++;
            } while (_buffers < 0);
            if (_seconds != i->time()-5)
                emit updateTime();
            else
                emit preloadSong();
        }
    }
}
*/
/*****************************************************************/

audio::audio() :
    _state(state_t::STOP),
    isPlaying(false)
#ifdef HAVE_BS2B
    ,_bs2bdp(0)
#endif
{
    _volume = settings.value("Audio Settings/volume", 50).toInt();

    _output = new qaudioBackend();
}

audio::~audio()
{
    stop();

    settings.setValue("Audio Settings/volume", _volume);

    delete _output;
}

bool audio::play(input* i, int pos)
{
    if ((i->songLoaded().isEmpty()) || (_state == state_t::PLAY))
        return false;

    qDebug() << "audio::play";

    unsigned int selectedCard = 0;
    QString cardName = SETTINGS->card();
    const QStringList devices = qaudioBackend::devices();
    for (int i=0; i<devices.size(); i++)
    {
        if (!cardName.compare(devices[i]))
        {
            selectedCard = i;
            break;
        }
    }

    unsigned int sampleRate = i->samplerate();

    sample_t sampleType;
    switch (i->precision())
    {
    case sample_t::U8:
    case sample_t::S16:
    case sample_t::S24:
    case sample_t::S32:
        sampleType = i->precision();
        break;
    case sample_t::SAMPLE_FLOAT:
    case sample_t::SAMPLE_FIXED:
        switch (SETTINGS->bits())
        {
        case 8:
            sampleType = sample_t::U8;
            break;
        case 16:
            sampleType = sample_t::S16;
            break;
        default:
            qWarning() << "Unhandled sample type " << SETTINGS->bits();
            return false;
        }
    }

    // FIXME only supports 8/16 bits
    const unsigned int precision = (sampleType == sample_t::U8) ? 1 : 2;
    qDebug() << "Setting parameters " << sampleRate << ":" << i->channels() << ":" << precision;
    iw = new InputWrapper(i);
    connect(_output, SIGNAL(songEnded()), this, SIGNAL(songEnded()));
    connect(iw, SIGNAL(switchSong()), this, SIGNAL(songEnded()));
    connect(iw, SIGNAL(updateTime()), this, SIGNAL(updateTime()));
    connect(iw, SIGNAL(preloadSong()), this, SIGNAL(preloadSong()));
    size_t bufferSize = _output->open(selectedCard, sampleRate, i->channels(), precision, iw);
    if (!bufferSize)
        return false;

    qDebug() << "Output samplerate " << sampleRate;

    // Check if soundcard supports requested samplerate
    _converter = CFACTORY->get(i->samplerate(), sampleRate, bufferSize,
        i->channels(), i->precision(), sampleType, i->fract());

    int bytePerSec = sampleRate * i->channels() * precision;
    iw->setBPS(bytePerSec);

#ifdef HAVE_BS2B
    if (SETTINGS->bs2b() && (i->channels() == 2)
#if BS2B_VERSION_MAJOR == 2
        && (sampleType == sample_t::S16)
#endif
    )
    {
        _bs2bdp = bs2b_open();
        if (_bs2bdp)
        {
            qDebug() << "bs2b enabled";
            bs2b_set_srate(_bs2bdp, sampleRate);
            bs2b_set_level(_bs2bdp, BS2B_DEFAULT_CLEVEL);
        }
    }
#endif
    _output->volume(_volume);

    i->seek(pos);

    isPlaying = true;
    _state = state_t::PLAY;

    return true;
}

void audio::pause()
{
    switch (_state)
    {
    case state_t::PLAY:
        qDebug() << "Pause";
        isPlaying = false;
        _output->pause();
        _state = state_t::PAUSE;
        break;
    case state_t::PAUSE:
        qDebug() << "Unpause";
        isPlaying = true;
        _output->unpause();
        _state = state_t::PLAY;
        break;
    case state_t::STOP:
        break;
    }
}

bool audio::stop()
{
    if (_state == state_t::STOP)
        return false;

    qDebug() << "audio::stop";

    isPlaying = false;

    _output->stop();

    iw->close();

    _output->close();

    _state = state_t::STOP;
#ifdef HAVE_BS2B
    if (_bs2bdp)
    {
        bs2b_close(_bs2bdp);
        _bs2bdp = 0;
    }
#endif
    if (_converter != nullptr)
        delete _converter;

    delete iw;

    return true;
}

void audio::volume(const int vol)
{
    _volume = vol;
    _output->volume(_volume);
}

bool audio::gapless(input* const i)
{
    return iw->tryPreload(i);
}

int audio::seconds() const { return iw->getSeconds(); }

void audio::unload() { iw->unload(); }

/*****************************************************************/

audioConfig::audioConfig(QWidget* win) :
    configFrame(win, "Audio")
{
    matrix()->addWidget(new QLabel(tr("Card"), this));
    QComboBox* cardList = new QComboBox(this);
    matrix()->addWidget(cardList);

    {
        const QStringList deviceNames = qaudioBackend::devices();
        int devices = deviceNames.size();
        cardList->addItems(deviceNames);
        cardList->setMaxVisibleItems((devices > 5) ? 5 : devices);

        int val = cardList->findText(SETTINGS->card());
        if (val >= 0)
            cardList->setCurrentIndex(val);
    }

    connect(cardList, SIGNAL(currentIndexChanged(const QString &)), this, SLOT(onCmdCard(const QString &)));

    matrix()->addWidget(new QLabel(tr("Default bitdepth"), this));
    QComboBox *bitBox = new QComboBox(this);
    matrix()->addWidget(bitBox);
    QStringList items;
    items << "8" << "16"; // TODO get supported values from device QAudioDeviceInfo::supportedSampleSizes() 
    bitBox->addItems(items);
    bitBox->setMaxVisibleItems(items.size());

    {
        unsigned int val;
        switch (SETTINGS->bits())
        {
        case 8:
            val = 0;
            break;
        default:
        case 16:
            val = 1;
            break;
        }

        bitBox->setCurrentIndex(val);
    }

    connect(bitBox, SIGNAL(currentIndexChanged(int)), this, SLOT(onCmdBits(int)));
}

void audioConfig::onCmdCard(const QString &card)
{
    qDebug() << "onCmdCard" << card;

    SETTINGS->_card = card;
}

void audioConfig::onCmdBits(int val)
{
    switch (val)
    {
    case 0:
        SETTINGS->_bits = 8;
        break;
    case 1:
        SETTINGS->_bits = 16;
        break;
    }
}
