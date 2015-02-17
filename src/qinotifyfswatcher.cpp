/*Qt Inotify Convenience wrapper
Copyright (C) 2015 Thoughtstream LLC (dba Bluescape)

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/

#include "qinotifyfswatcher.h"

#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/inotify.h>

#include <QSocketNotifier>
#include <QDataStream>

#include <QDebug>
#include <QFile>
#include <QDir>

class QInotifyFSWatcherPrivate : public QObject
{
    Q_OBJECT
public:
    QInotifyFSWatcherPrivate(QInotifyFSWatcher *p);
    ~QInotifyFSWatcherPrivate();

    int inotifyFd;
    QSocketNotifier *notifier;
    QDataStream	    *eventStream;

    void addPath(const QString &, bool recursive, QInotifyFSWatcher::InEvent eventType);

    QHash<int, QString> idToDir;
    QHash<int, QStringList> idToFileList;

    QInotifyFSWatcher *pSelf;

private:
    QInotifyFSWatcherPrivate(const QInotifyFSWatcherPrivate&) = delete;
    QInotifyFSWatcherPrivate& operator=(const QInotifyFSWatcherPrivate&) = delete;

public Q_SLOTS:
    void handleActivity();
};

QInotifyFSWatcherPrivate::QInotifyFSWatcherPrivate(QInotifyFSWatcher *p)
     : QObject(p),
       inotifyFd(inotify_init()),
       pSelf(p)
{
    if (inotifyFd == -1)
	    qFatal("Failed to init inotify");

    fcntl(inotifyFd, F_SETFD, FD_CLOEXEC);

    notifier = new QSocketNotifier(inotifyFd, QSocketNotifier::Read, this);
    connect(notifier, SIGNAL(activated(int)), this, SLOT(handleActivity()));

    QFile *source = new QFile(this);
    if(!source->open(inotifyFd, QIODevice::ReadOnly)) {
	    qDebug() << "Failed to open the inotify fd for reading";
    }
    eventStream = new QDataStream(source);
}

QInotifyFSWatcherPrivate::~QInotifyFSWatcherPrivate()
{
    notifier->setEnabled(false);
    ::close(inotifyFd);
}

void QInotifyFSWatcherPrivate::handleActivity()
{
    // Adopted directly from
    // /opt/dev/src/qtproject/qt5/qtbase/src/corelib/io/qfilesystemwatcher_inotify.cpp
    int buffSize = 0;
    ioctl(inotifyFd, FIONREAD, (char *) &buffSize);
    QVarLengthArray<char, 4096> buffer(buffSize);
    buffSize = read(inotifyFd, buffer.data(), buffSize);
    char *at = buffer.data();
    char * const end = at + buffSize;

    QHash<int, inotify_event *> eventForId;
    while (at < end) {
        inotify_event *event = reinterpret_cast<inotify_event *>(at);

        if (eventForId.contains(event->wd))
            eventForId[event->wd]->mask |= event->mask;
        else
            eventForId.insert(event->wd, event);

        at += sizeof(inotify_event) + event->len;
    }

    for (auto it = eventForId.constBegin(); it != eventForId.constEnd(); ++it) {
        const inotify_event &event = **it;

	    if (idToFileList.contains(event.wd)) {
	        if (idToFileList.isEmpty() || idToFileList[event.wd].contains(event.name)) {
	    	    pSelf->fileChanged(event.name, static_cast<QInotifyFSWatcher::InEvent>(event.mask));
	        }
	    } else {
	        qDebug() << "Busy being notified about directory we are not tracking";
	    }
    }
}

void QInotifyFSWatcherPrivate::addPath(const QString &path, bool recursive, QInotifyFSWatcher::InEvent eventType)
{
    // TODO:
    // #0 relative paths!
    // #1 filter per event type
    // #2 handle future directories
    qDebug() << "Setting a" << (recursive ? "recursive" : "") << "watch on:" << path;

    QFileInfo currentPath(path);

    QString dirName;
    QString fileName;

    if (currentPath.exists()) {
        if (currentPath.isFile())
        {
            dirName = currentPath.dir().canonicalPath();
            fileName = currentPath.fileName();
        } else
            dirName = path;
    } else if (path.endsWith("/"))
        dirName = path;
    else {
        // #2
        int dirIndex = path.lastIndexOf("/") + 1;
        if (dirIndex < 0 ) {
            qDebug() << "Unhandled case";
        } else {
            dirName = path.mid(0, dirIndex);
            fileName = path.mid(dirIndex);
        }
    }

    int id = inotify_add_watch(inotifyFd, dirName.toLatin1().constData(), eventType);

    idToDir.insert(id, dirName);

    if (idToFileList.contains(id)) {
	    idToFileList[id] << fileName;
    } else {
	    idToFileList.insert(id, QStringList() << fileName);
    }

    qDebug() << "Watching" << idToFileList[id] << "in" << dirName ;
}

QInotifyFSWatcher::QInotifyFSWatcher(QObject *parent)
    : QObject(parent)
{
    d = new QInotifyFSWatcherPrivate(this);
}

QInotifyFSWatcher::~QInotifyFSWatcher()
{
    delete d;
    d = 0;
}

void QInotifyFSWatcher::addPath(const QString &path, bool recursive, InEvent eventType)
{
    d->addPath(path, recursive, eventType);
}

#include "qinotifyfswatcher.moc"
