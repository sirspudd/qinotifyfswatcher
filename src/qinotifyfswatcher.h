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

#ifndef Q_INOTIFY_FS_WATCHER
#define Q_INOTIFY_FS_WATCHER

#include <QObject>

#if 0
/* Supported events suitable for MASK parameter of INOTIFY_ADD_WATCH.  */
#define IN_ACCESS	 0x00000001	/* File was accessed.  */
#define IN_MODIFY	 0x00000002	/* File was modified.  */
#define IN_ATTRIB	 0x00000004	/* Metadata changed.  */
#define IN_CLOSE_WRITE	 0x00000008	/* Writtable file was closed.  */
#define IN_CLOSE_NOWRITE 0x00000010	/* Unwrittable file closed.  */
#define IN_CLOSE	 (IN_CLOSE_WRITE | IN_CLOSE_NOWRITE) /* Close.  */
#define IN_OPEN		 0x00000020	/* File was opened.  */
#define IN_MOVED_FROM	 0x00000040	/* File was moved from X.  */
#define IN_MOVED_TO      0x00000080	/* File was moved to Y.  */
#define IN_MOVE		 (IN_MOVED_FROM | IN_MOVED_TO) /* Moves.  */
#define IN_CREATE	 0x00000100	/* Subfile was created.  */
#define IN_DELETE	 0x00000200	/* Subfile was deleted.  */
#define IN_DELETE_SELF	 0x00000400	/* Self was deleted.  */
#define IN_MOVE_SELF	 0x00000800	/* Self was moved.  */


/* Events sent by the kernel.  */
#define IN_UNMOUNT	 0x00002000	/* Backing fs was unmounted.  */
#define IN_Q_OVERFLOW	 0x00004000	/* Event queued overflowed.  */
#define IN_IGNORED	 0x00008000	/* File was ignored.  */

/* Special flags.  */
#define IN_ONLYDIR	 0x01000000	/* Only watch the path if it is a
					   directory.  */
#define IN_DONT_FOLLOW	 0x02000000	/* Do not follow a sym link.  */
#define IN_EXCL_UNLINK	 0x04000000	/* Exclude events on unlinked
					   objects.  */
#define IN_MASK_ADD	 0x20000000	/* Add to the mask of an already
					   existing watch.  */
#define IN_ISDIR	 0x40000000	/* Event occurred against dir.  */
#define IN_ONESHOT	 0x80000000	/* Only send event once.  */

/* All events which a program can wait on.  */
#define IN_ALL_EVENTS	 (IN_ACCESS | IN_MODIFY | IN_ATTRIB | IN_CLOSE_WRITE  \
			  | IN_CLOSE_NOWRITE | IN_OPEN | IN_MOVED_FROM	      \
			  | IN_MOVED_TO | IN_CREATE | IN_DELETE		      \
			  | IN_DELETE_SELF | IN_MOVE_SELF)
#endif

class QInotifyFSWatcherPrivate;

class QInotifyFSWatcher : public QObject
{
    Q_OBJECT

public:
    // Should I expose the INotify innards?
    enum InEvent {
        Access=0x00000001,
        Modified=0x00000002,
        AttrModified=0x00000004,
        CloseWrite=0x00000008,
        CloseRead=0x00000010,
        Open=0x00000020,
        MoveFrom=0x00000040,
        MoveTo=0x00000080,
        Create=0x00000100,
        Delete=0x00000200,
        DeleteSelf=0x00000400,
        MoveSelf=0x00000800,
        AllEvents=0x000008FF,
        Close=(CloseWrite | CloseRead),
	Move=(MoveFrom | MoveTo),
        KernelEvents=0x00002000,
        Unmount=0x00002000,
        Overflow=0x00004000,
        Ignored=0x00008000,
        SpecialEvents=0x01000000,
        OnlyDir=0x01000000,
        DontFollow=0x02000000,
        ExcludeUnlinked=0x04000000,
        MaskAdd=0x20000000,
        IsDir=0x40000000,
        OneShot=0x80000000
    };
    Q_DECLARE_FLAGS(InEvents, InEvent)

    QInotifyFSWatcher(QObject *parent);
protected:
    ~QInotifyFSWatcher();
public Q_SLOTS:
    void addPath(const QString &, bool recursive = true, InEvent eventType = AllEvents);
    void removePath(const QString &) { /*Todo*/ }
Q_SIGNALS:
    void fileChanged(const QString&, QInotifyFSWatcher::InEvent);
    void dirChanged(const QString&, QInotifyFSWatcher::InEvent);
private:
    QInotifyFSWatcher(const QInotifyFSWatcher&) = delete;
    QInotifyFSWatcher& operator=(const QInotifyFSWatcher&) = delete;
    QInotifyFSWatcherPrivate *d;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(QInotifyFSWatcher::InEvents)
#endif //Q_INOTIFY_FS_WATCHER
