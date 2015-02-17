#include "qinotifyfswatcher.h"

#include <QCoreApplication>
#include <QDebug>
#include <QStandardPaths>

class EventResponder : public QObject
{
    Q_OBJECT

public slots:
    void handleFileNotification(const QString& fileName, QInotifyFSWatcher::InEvent event) { qDebug() << "Getting notified:" << fileName << event; }
};

int main(int argc, char **argv)
{
    QCoreApplication app(argc, argv);
    QInotifyFSWatcher *watcher = new QInotifyFSWatcher(&app);
    QString testFilePath = QStandardPaths::standardLocations(QStandardPaths::HomeLocation).first() + "/test-file";
    watcher->addPath(testFilePath);
    watcher->addPath("/x/y/z/");
    QObject::connect(watcher, SIGNAL(fileChanged(const QString&, QInotifyFSWatcher::InEvent)), new EventResponder, SLOT(handleFileNotification(const QString&, QInotifyFSWatcher::InEvent)));
    app.exec();
}

#include "main.moc"
