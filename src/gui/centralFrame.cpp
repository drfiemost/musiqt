/*
 *  Copyright (C) 2013-2021 Leandro Nini
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

#include "centralFrame.h"

#include "iconFactory.h"
#include "bookmark.h"
#include "playlist.h"
#include "playlistModel.h"
#include "proxymodel.h"
#include "input/inputFactory.h"
#include "settings.h"
#include "trackListFactory.h"
#include "xdg.h"

#include <QApplication>
#include <QButtonGroup>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QFileDialog>
#include <QFileInfo>
#include <QFileSystemModel>
#include <QItemSelectionModel>
#include <QMenu>
#include <QMessageBox>
#include <QPushButton>
#include <QSlider>
#include <QStackedWidget>
#include <QTimer>
#include <QTreeView>
#include <QHeaderView>
#include <QWidgetAction>
#include <QDebug>


void loadThread::run()
{
    emit loaded(IFACTORY->get(fileName));
}

/*****************************************************************/

centralFrame::centralFrame(player* p, QWidget *parent) :
    QWidget(parent),
    m_player(p)
{
    //connect(m_audio, &audio::outputError, this, &onPlaybackStopped);
    connect(m_player, &player::playbackStopped, this, &centralFrame::onPlaybackStopped);
    connect(m_player, &player::updateTime,  this, &centralFrame::onUpdateTime);
    connect(m_player, &player::songEnded,   this, &centralFrame::onSongEnded);
    connect(m_player, &player::preloadSong, this, &centralFrame::onPreloadSong);

    // dir view
    m_fsm = new QFileSystemModel(this);
    m_fsm->setFilter(QDir::AllDirs|QDir::Drives|QDir::NoDotAndDotDot|QDir::Files);
    m_fsm->setNameFilterDisables(false);
    m_fsm->setNameFilters(TFACTORY->plExt());

    m_dirlist = new QTreeView(this);
    m_dirlist->setModel(m_fsm);
    m_dirlist->hideColumn(3);
    m_dirlist->hideColumn(2);
    m_dirlist->hideColumn(1);
    m_dirlist->setHeaderHidden(true);
    m_dirlist->setUniformRowHeights(true);
    m_dirlist->setDragEnabled(true);
    m_dirlist->setContextMenuPolicy(Qt::CustomContextMenu);
    m_dirlist->header()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    m_dirlist->header()->setStretchLastSection(false);
    QItemSelectionModel* selectionModel = m_dirlist->selectionModel();

    connect(selectionModel, &QItemSelectionModel::currentChanged, this, &centralFrame::onDirSelected);
    connect(m_dirlist, &QTreeView::customContextMenuRequested, this, &centralFrame::onRgtClkDirList);

    // m_playlist
    m_playlist = new playlist(this);
    m_playlist->setAlternatingRowColors(true);
    m_playlist->setUniformItemSizes(true);
    m_playlist->setDragDropMode(QAbstractItemView::DropOnly);

    m_playlistModel = new playlistModel(this);

    m_proxyModel = new proxymodel(this);
    m_proxyModel->setSourceModel(m_playlistModel);
    m_proxyModel->setDynamicSortFilter(true);
    m_proxyModel->sort(proxymodel::sortMode::Ascending);
    m_playlist->setModel(m_proxyModel);

    m_proxyModel->setFilterRegExp(QRegExp(getFilter(), Qt::CaseInsensitive));
    m_proxyModel->setFilterRole(Qt::UserRole+1);

    selectionModel = m_playlist->selectionModel();
    connect(selectionModel, &QItemSelectionModel::currentRowChanged, this, &centralFrame::onCmdSongSelected);
    connect(m_playlist, &playlist::doubleClicked, this, &centralFrame::onCmdPlayPauseSong);
    connect(m_playlist, &playlist::changed, this, &centralFrame::updateSongs);

    m_playlist->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(m_playlist, &playlist::customContextMenuRequested, this, &centralFrame::onRgtClkPlayList);

    m_bookmarkList = new bookmark(this);
    m_bookmarkList->setAlternatingRowColors(true);
    connect(m_bookmarkList, &bookmark::currentTextChanged, this, &centralFrame::setDir);

    // left view
    QStackedWidget *stackedWidget = new QStackedWidget(this);
    stackedWidget->addWidget(m_playlist);
    stackedWidget->addWidget(m_bookmarkList);
    QButtonGroup *buttonGroup = new QButtonGroup(this);
    connect(buttonGroup, &QButtonGroup::idClicked, stackedWidget, &QStackedWidget::setCurrentIndex);

    QWidget *w = new QWidget(this);
    {
        QHBoxLayout *buttons = new QHBoxLayout(w);
        buttons->setContentsMargins(0,0,0,0);
        QPushButton *b1 = new QPushButton(GET_ICON(icon_playlist), tr("Playlist"), this);
        b1->setCheckable(true);
        b1->setAutoExclusive(true);
        b1->setChecked(true);
        buttonGroup->addButton(b1, 0);
        buttons->addWidget(b1);
        QPushButton *b2 = new QPushButton(GET_ICON(icon_bookmark), tr("Bookmarks"), this);
        b2->setCheckable(true);
        b2->setAutoExclusive(true);
        buttonGroup->addButton(b2, 1);
        buttons->addWidget(b2);
        buttons->setSpacing(0);
    }

    //
    QWidget *wButtons = new QWidget(this);
    {
        QHBoxLayout *buttons = new QHBoxLayout(wButtons);
        buttons->setContentsMargins(0,0,0,0);
        m_editMode = new QPushButton(this);
        m_editMode->setIcon(GET_ICON(icon_editlist));
        m_editMode->setToolTip(tr("Edit"));;
        m_editMode->setStatusTip(tr("Edit playlist"));
        m_editMode->setCheckable(true);
        buttons->addWidget(m_editMode);
        connect(m_editMode, &QPushButton::clicked, this, &centralFrame::onCmdPlEdit);
        QPushButton *b1 = new QPushButton(this);
        b1->setIcon(GET_ICON(icon_documentsave));
        b1->setToolTip(tr("Save"));
        b1->setStatusTip(tr("Save playlist"));
        connect(b1, &QPushButton::clicked, this, &centralFrame::onCmdPlSave);
        buttons->addWidget(b1);
        b1 = new QPushButton(this);
        b1->setIcon(GET_ICON(icon_currentplaylist));
        b1->setToolTip(tr("Current playlist"));
        b1->setStatusTip(tr("Return to current playlist"));
        connect(b1, &QPushButton::clicked, this, &centralFrame::onCmdCurrentDir);
        buttons->addWidget(b1);
        m_home = new QPushButton(this);
        m_home->setIcon(GET_ICON(icon_gohome));
        m_home->setToolTip(tr("Home"));
        m_home->setStatusTip(tr("Home"));
        m_home->setMenu(new QMenu());

        createHomeMenu();

        buttons->addWidget(m_home);
        buttons->setSpacing(0);
    }

    // Left view - file list/bookmarks
    QWidget *w2 = new QWidget(this);
    QVBoxLayout *vbox = new QVBoxLayout(w2);
    vbox->setContentsMargins(0,0,0,0);
    vbox->addWidget(w);
    vbox->addWidget(stackedWidget);
    vbox->addWidget(wButtons);
    vbox->setSpacing(0);

    QWidget *cFrame = new QWidget(this);
    QHBoxLayout *hbox = new QHBoxLayout(cFrame);
    hbox->setContentsMargins(0,0,0,0);
    hbox->addWidget(w2);
    hbox->addWidget(m_dirlist);

    QVBoxLayout *main = new QVBoxLayout(this);
    main->setContentsMargins(0,0,0,0);
    m_slider = new QSlider(Qt::Horizontal);
    m_slider->setMinimum(0);
    m_slider->setMaximum(100);
    m_slider->setTickInterval(10);
    m_slider->setTickPosition(QSlider::TicksBelow);
    m_slider->setTracking(false);
    m_slider->setDisabled(true);
    connect(m_slider, &QSlider::actionTriggered,
        [this]()
        {
            int pos = m_slider->sliderPosition();
            qDebug() << "seek: " << pos;
            m_player->seek(pos);
        }
    );
    connect(this, &centralFrame::updateSlider, m_slider, &QSlider::setValue);
    main->addWidget(m_slider);
    main->addWidget(cFrame);
}

centralFrame::~centralFrame()
{
    m_bookmarkList->save();
}

void centralFrame::createHomeMenu()
{
    QMenu *menu = m_home->menu();
    menu->clear();

    QActionGroup *homeGroup = new QActionGroup(menu);

    QString musicDir = xdg::getMusicDir();
    if (!musicDir.isEmpty())
    {
        QAction *action = menu->addAction(tr("System music location"));
        action->setData(musicDir);
        action->setStatusTip(musicDir);
        homeGroup->addAction(action);
    }

    for (int i=0; i<IFACTORY->num(); i++)
    {
        inputConfig* ic = IFACTORY->getConfig(i);

        QString musicDir = ic->getMusicDir();
        if (!musicDir.isEmpty())
        {
            QString name = IFACTORY->name(i);
            QAction *action = menu->addAction(ic->icon(), name.append(tr(" music location")));
            action->setData(musicDir);
            action->setStatusTip(musicDir);
            homeGroup->addAction(action);
        }

        delete ic;
    }
    connect(homeGroup, &QActionGroup::triggered, this, &centralFrame::onHome);
}

void centralFrame::changeState()
{
    emit stateChanged(m_player->state());
    switch (m_player->state())
    {
    case state_t::STOP:
        m_slider->setDisabled(true);
        break;
    case state_t::PLAY:
        m_slider->setDisabled(!m_player->seekable());
        break;
    case state_t::PAUSE:
        m_slider->setDisabled(true);
        break;
    }
}

bool centralFrame::isPlaylistDirSelected()
{
    QFileInfo fileInfo(m_player->loadedSong());
    QString playDir = fileInfo.absolutePath();
    qDebug() << "playDir " << playDir;
    QString currentDir = m_fsm->filePath(m_dirlist->currentIndex());
    qDebug() << "currentDir " << currentDir;
    return playDir==currentDir;
}

void centralFrame::onDirSelected(const QModelIndex& idx)
{
    qDebug() << "onDirSelected";
    m_bookmarkList->setCurrentRow(-1);

    if (m_editMode->isChecked())
        return;

    const QString curItem = m_fsm->fileName(idx);
    if (curItem.isEmpty())
        return;

    qDebug() << "Set dir " << curItem;

    const QString mPath = m_fsm->fileInfo(idx).absoluteFilePath();
    qDebug() << "Full path: " << mPath;

    m_playlistModel->load(mPath);

    if (m_proxyModel->rowCount() == 0)
    {
        return;
    }

    const QString fileName = m_dirlist->property("UserData").toString();
    if (!fileName.isEmpty())
    {
        qDebug() << "selecting file " << fileName;
        QModelIndexList items = m_proxyModel->match(m_proxyModel->index(0, 0), Qt::DisplayRole,
                QVariant::fromValue(fileName), 1, Qt::MatchExactly|Qt::MatchCaseSensitive);
        if (!items.empty())
        {
            m_playlist->setCurrentIndex(items.at(0));
            m_dirlist->setProperty("UserData", QVariant(QString()));
        }
    }
    else
    {
        QString songLoaded = m_player->loadedSong();
        if (songLoaded.isEmpty())
        {
            m_playlist->setCurrentIndex(m_proxyModel->index(0, 0));
        }
        else
        {
            QFileInfo fileInfo(songLoaded);
            QModelIndexList items = m_proxyModel->match(m_proxyModel->index(0, 0), Qt::DisplayRole,
                    QVariant::fromValue(fileInfo.completeBaseName()), 1, Qt::MatchExactly|Qt::MatchCaseSensitive);
            if (!items.empty())
            {
                m_playlist->setCurrentIndex(items.at(0));
                m_playlist->scrollTo(items.at(0));
            }
            else if (m_player->state() == state_t::STOP)
                m_playlist->setCurrentIndex(m_proxyModel->index(0, 0));
        }
    }
}

void centralFrame::onHome(QAction* action)
{
    QString musicDir = action->data().toString();
    qDebug() << "go to music dir: " << musicDir;
    setDir(musicDir);
}

void centralFrame::onCmdCurrentDir()
{
    if (m_editMode->isChecked())
        return;

    qDebug() << "onCmdCurrentDir";
    QString file = m_player->loadedSong();
    if (file.isEmpty())
        return;

    QFileInfo fileInfo(file);
    setDir(fileInfo.absolutePath());
    QModelIndexList items = m_proxyModel->match(m_proxyModel->index(0, 0),
        Qt::DisplayRole, QVariant::fromValue(fileInfo.completeBaseName()),
        -1, Qt::MatchExactly|Qt::MatchCaseSensitive);

    m_playlist->setCurrentIndex(items.at(0));
}

void centralFrame::setFile(const QString& file, const bool play)
{
    qDebug() << "setFile " << file;

    QModelIndex curItem = m_playlist->currentIndex();

    QFileInfo fileInfo(file);
    // Check if requested file/directory exists
    if (!fileInfo.exists())
    {
        // ???
        //if (curItem.isValid())
        //    onHome();
        return;
    }

    QModelIndexList items = m_playlistModel->match(m_playlistModel->index(0, 0), Qt::DisplayRole, QVariant::fromValue(file), -1, Qt::MatchExactly|Qt::MatchCaseSensitive);

    const bool selected = items.empty() ? false : m_playlist->selectionModel()->isSelected(items.at(0));

    // Check if requested song is already playing
    if ((m_player->state() != state_t::STOP) && selected)
        return;

    QModelIndex val;
    if (!items.empty())
        val = items.at(0);

    QString currentDir = m_fsm->fileName(m_dirlist->currentIndex());
    const bool dirSelected = !currentDir.compare(fileInfo.dir().absolutePath());

    if (fileInfo.isDir() || (TFACTORY->plExt().indexOf(fileInfo.suffix().toLower()) >= 0))
    {
        if (dirSelected)
        {
            if (play && (m_player->state() == state_t::STOP))
            {
                onCmdPlayPauseSong();
            }
            return;
        }
        else
        {
            m_dirlist->setProperty("UserData", QVariant(QString()));
            setDir(file);
            m_playMode = true; // TODO sync GUI
        }
        return;
    }

    {
        const QString fName = fileInfo.fileName();

        if (!fName.isEmpty())
        {
            QRegExp regexp = m_proxyModel->filterRegExp();
            if (!regexp.exactMatch(fName))
            {
                // file is not supported, ignore message
                return;
            }
        }
    }

    if (dirSelected)
    {
        if (val.isValid())
        {
            if (val == curItem)
            {
                onCmdPlayPauseSong();
            }
            else
                m_playlist->setCurrentIndex(val);
        }
    }
    else
    {
        m_dirlist->setProperty("UserData", QVariant(fileInfo.completeBaseName()));
        setDir(fileInfo.dir().absolutePath());
    }
}

void centralFrame::onCmdPlayPauseSong()
{
    if (m_player->state() != state_t::STOP)
    {
        m_player->pause();
    }
    else
    {
        // if loaded song is not the selected one don't play
        QString songLoaded = m_player->loadedSong();
        const QString song = m_proxyModel->data(m_playlist->currentIndex(), Qt::UserRole).toString();
        qDebug() << "Song: " << song;
        if (!song.isEmpty() && song.compare(songLoaded))
        {
            return;
        }

        if (m_player->play())
        {
            QFileInfo fileInfo(songLoaded);
            setDir(fileInfo.absolutePath());
        }
    }

    changeState();
}

void centralFrame::onPlaybackStopped()
{
    qDebug() << "centralFrame::onPlaybackStopped";

    emit updateTime(0);
    emit updateSlider(0);
    changeState();
    if (!isPlaylistDirSelected())
    {
        onDirSelected(m_dirlist->currentIndex());
    }
}

void centralFrame::onCmdPrevSong()
{
    onCmdChangeSong(dir_t::ID_PREV);
}

void centralFrame::onCmdNextSong()
{
    onCmdChangeSong(dir_t::ID_NEXT);
}

void centralFrame::onCmdChangeSong(dir_t dir)
{
    if ((m_player->state() != state_t::STOP) && !isPlaylistDirSelected())
        return;

    int row = m_playlist->currentIndex().row();

    switch (dir)
    {
    case dir_t::ID_PREV:
            row--;
            qDebug("Previous");
            break;
    case dir_t::ID_NEXT:
            row++;
            qDebug("Next");
            break;
    }

    QModelIndex index = m_proxyModel->index(row, 0);

    if (index.isValid())
    {
        m_playlist->setCurrentIndex(index);
    }
}

void centralFrame::load(const QString& filename, bool preload)
{
    qDebug() << "Loading " << filename;

    emit updateSlider(0);

    loadThread* loader = new loadThread(filename);
    if (preload)
        connect(loader, &loadThread::loaded, this, &centralFrame::onCmdSongPreLoaded);
    else
        connect(loader, &loadThread::loaded, this, &centralFrame::onCmdSongLoaded);
    connect(loader, &loadThread::finished, loader, &loadThread::deleteLater);

    QApplication::setOverrideCursor(Qt::WaitCursor);

    loader->start();
}

void centralFrame::onCmdSongLoaded(input* res)
{
    QApplication::restoreOverrideCursor();

    m_player->loaded(res, SETTINGS->subtunes());

    if (res != nullptr)
    {
        emit setDisplay();

        changeState();

        qDebug() << "Song loaded";
    }
    else
    {
        qWarning() << "Error loading song";

        emit clearDisplay(false);

        changeState();
    }
}

void centralFrame::onCmdSongPreLoaded(input* res)
{
    QApplication::restoreOverrideCursor();

    m_player->preloaded(res, SETTINGS->subtunes());

    qDebug() << "Song preloaded";
}

void centralFrame::onCmdSongSelected(const QModelIndex& currentRow)
{
    if (m_editMode->isChecked())
        return;

    qDebug() << "onCmdSongSelected " << currentRow.row();
    if (!currentRow.isValid())
        return;

    QString songLoaded = m_player->loadedSong();
    const QString song = m_proxyModel->data(currentRow, Qt::UserRole).toString();
    if (!songLoaded.isEmpty() && !song.compare(songLoaded))
        return;

    m_playlist->scrollTo(currentRow);

    if (!m_playlist->isVisible())
        updateSongs();

    if (m_player->tryPreload(song))
    {
        emit setDisplay();
    }
    else
    {
        emit clearDisplay(true);

        load(song);
    }
}

void centralFrame::onUpdateTime()
{
    // Playing may have stopped while message was in queue
    if (m_player->state() != state_t::STOP)
    {
        emit updateTime(m_player->seconds());

        if (!m_slider->isSliderDown() && m_player->seekable())
            emit updateSlider(m_player->getPosition());
    }
}

void centralFrame::onPreloadSong()
{
    qDebug("centralFrame::onPreloadSong");
    if (m_playMode && m_player->gapless() && isPlaylistDirSelected())
    {
        const int nextSong = m_playlist->currentIndex().row()+1;
        QModelIndex index = m_proxyModel->index(nextSong, 0);
        if (index.isValid())
        {
            load(m_proxyModel->data(index, Qt::UserRole).toString(), true);
        }
    }
}

void centralFrame::onSongEnded()
{
    qDebug("centralFrame::onSongEnded");
    if (SETTINGS->subtunes() && (m_player->subtune()<m_player->subtunes()))
    {
        m_player->changeSubtune(dir_t::ID_NEXT);
        return;
    }

    if (m_playMode)
    {
        QModelIndex index;
        if (isPlaylistDirSelected())
        {
            int nextSong = m_playlist->currentIndex().row() + 1;
            index = m_proxyModel->index(nextSong, 0);
        }

        if (index.isValid())
        {
            m_playlist->setCurrentIndex(index);
            return;
        }
    }

    m_player->stop();
}

void centralFrame::onSettingsChanged()
{
    createHomeMenu();

    QString songLoaded = m_player->loadedSong();
    if (!songLoaded.isEmpty())
    {
        // we must reload the song
        m_player->loaded(nullptr, false);
        onCmdSongSelected(m_playlist->currentIndex());
    }
}

void centralFrame::onRgtClkDirList(const QPoint& pos)
{
    qDebug() << "onRgtClkDirList";
    QModelIndex item = m_dirlist->indexAt(pos);
    if (!item.isValid())
        return;

    QMenu pane(this);
    QWidgetAction *wa = new QWidgetAction(&pane);
    wa->setDefaultWidget(new QLabel(utils::shrink(m_fsm->fileName(item))));
    pane.addAction(wa);
    pane.addSeparator();

    if ((m_editMode->isChecked()) && (!m_fsm->isDir(item)))
    {
        QAction *additem = pane.addAction(
            GET_ICON(icon_listadd), tr("Add to playlist"), this,
            [this, item]()
            {
                m_playlistModel->append(m_fsm->fileInfo(item).absoluteFilePath());
            }
        );
        additem->setStatusTip(tr("Add song to the playlist"));
    }
    else
    {
        QAction *additem = pane.addAction(
            GET_ICON(icon_bookmark), tr("Add to bookmarks"), this, 
            [this, item]()
            {
                m_bookmarkList->add(m_fsm->fileInfo(item).absoluteFilePath());
            }
        );
        additem->setStatusTip(tr("Add directory to the bookmarks"));
    }

    pane.exec(m_dirlist->mapToGlobal(pos));
}

void centralFrame::onRgtClkPlayList(const QPoint& pos)
{
    qDebug() << "onRgtClkPlayList";
    QModelIndex item = m_playlist->indexAt(pos);

    QMenu pane(this);
    if (item.isValid())
    {
        const int itemRow = item.row();

        QWidgetAction *wa = new QWidgetAction(&pane);
        QLabel *label = new QLabel(utils::shrink(m_playlistModel->data(item, Qt::UserRole).toString()));
        label->setAlignment(Qt::AlignCenter);
        wa->setDefaultWidget(label);
        pane.addAction(wa);
        pane.addSeparator();
        QAction *delitem = pane.addAction(
            GET_ICON(icon_listremove), tr("Remove item"), this,
            [this, itemRow]()
            {
                qDebug() << "remove item " << itemRow;
                m_playlistModel->removeRow(itemRow);
            }
        );
        delitem->setStatusTip(tr("Remove selected item from playlist"));
        pane.addSeparator();
    }

    QAction* asc = new QAction(tr("Sort ascending"), &pane);
    asc->setCheckable(true);
    asc->setStatusTip(tr("Sort ascending"));
    if (m_proxyModel->getMode() == proxymodel::sortMode::Ascending) asc->setChecked(true);
    connect(asc, &QAction::triggered, this,
        [this]()
        {
            m_proxyModel->sort(proxymodel::sortMode::Ascending);
        }
    );
    QAction* desc = new QAction(tr("Sort descending"), &pane);
    desc->setCheckable(true);
    desc->setStatusTip(tr("Sort descending"));
    if (m_proxyModel->getMode() == proxymodel::sortMode::Descending) desc->setChecked(true);
    connect(desc, &QAction::triggered, this,
        [this]()
        {
            m_proxyModel->sort(proxymodel::sortMode::Descending);
        }
    );
    QAction* rnd = new QAction(tr("Shuffle"), &pane);
    rnd->setCheckable(true);
    rnd->setStatusTip(tr("Sort randomly"));
    if (m_proxyModel->getMode() == proxymodel::sortMode::Random) rnd->setChecked(true);
    connect(rnd, &QAction::triggered, this,
        [this]()
        {
            m_proxyModel->sort(proxymodel::sortMode::Random);
        }
    );

    QActionGroup *radioGroup = new QActionGroup(&pane);
    radioGroup->addAction(asc);
    radioGroup->addAction(desc);
    radioGroup->addAction(rnd);

    pane.addSeparator()->setText(tr("Sorting"));
    pane.addAction(asc);
    pane.addAction(desc);
    pane.addAction(rnd);

    pane.exec(m_playlist->mapToGlobal(pos));
}

void centralFrame::onCmdPlEdit(bool checked)
{
    m_playlist->setAcceptDrops(checked);
    m_playlist->setDropIndicatorShown(checked);

    if (checked)
    {
        m_playlistModel->clear();

        m_fsm->setNameFilters(getPattern());

        setProperty("SelectedDir", QVariant(m_fsm->filePath(m_dirlist->currentIndex())));
    }
    else
    {
        m_fsm->setNameFilters(TFACTORY->plExt());
        m_dirlist->setCurrentIndex(QModelIndex());
        QString dir = property("SelectedDir").toString();
        qDebug() << "dir " << dir;
        m_dirlist->setCurrentIndex(m_fsm->index(dir));
    }
}

void centralFrame::onCmdPlSave()
{
    const int n = m_proxyModel->rowCount();
    if (!n)
        return;

    QString filter = TFACTORY->plExt().join(" ");
    QString filename = QFileDialog::getSaveFileName(this, tr("Save playlist"), "", filter);
    if (filename.isEmpty())
        return;

    std::unique_ptr<trackList> tracklist(TFACTORY->get(filename));

    if (tracklist.get() == nullptr)
        return;

    tracks_t tracks;
    for(int r = 0; r < m_proxyModel->rowCount(); ++r)
    {
        QModelIndex idx = m_proxyModel->index(r, 0);
        tracks.append(m_proxyModel->data(idx, Qt::UserRole+1).toString());
    }
    if (!tracklist->save(tracks))
        QMessageBox::critical(this, tr("Error"), tr("Error saving playlist"));
}

void centralFrame::updateSongs()
{
    if ((m_player->state() != state_t::STOP) && !isPlaylistDirSelected())
        return;

    const int tunes = m_proxyModel->rowCount();
    const int tune = 1 + m_playlist->currentIndex().row();

    QString text(QString("%1/%2").arg(tune).arg(tunes));
    emit songUpdated(text);
}

void centralFrame::setDir(const QString& dir)
{
    qDebug() << "centralFrame::setDir: " << dir;
    if (dir.isEmpty())
        return;

    QModelIndex index = m_fsm->index(dir);
    if (!index.isValid())
    {
        QMessageBox::warning(this, tr("Warning"), QString(tr("Path %1 does not exist")).arg(dir));
        return;
    }

    QMetaObject::Connection * const connection = new QMetaObject::Connection;
    *connection = connect(
        m_fsm, &QFileSystemModel::directoryLoaded,
        [connection, this](const QString &path)
        {
            qDebug() << "dirLoaded: " << path;
            if (path.compare(m_fsm->fileInfo(m_dirlist->currentIndex()).absolutePath()) == 0)
            {
                qDebug() << "scrollTo" << path;
                QObject::disconnect(*connection);
                delete connection;
                QTimer::singleShot(50, [this](){m_dirlist->scrollTo(m_dirlist->currentIndex());});
            }
        }
    );

    m_dirlist->setCurrentIndex(index);
    m_dirlist->scrollTo(index);
}

void centralFrame::init()
{
    qDebug() << "*** init ***";
    m_fsm->setRootPath(QDir::rootPath());

    m_bookmarkList->load();
}

QString centralFrame::getFilter() const
{
    QString filter(IFACTORY->getExtensions().join("|"));
    filter.prepend(".*\\.(").append(")$");
    qDebug() << "filter: " << filter;
    return filter;
}

QStringList centralFrame::getPattern() const
{
    QStringList result;
    for (QString str: IFACTORY->getExtensions())
    {
        result << str.prepend("*.");
    }
    qDebug() << "pattern: " << result;
    return result;
}
