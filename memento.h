#ifndef MEMENTO_H
#define MEMENTO_H

#include <QList>
#include <QAction>
#include <QImage>

class Memento
{
private:
    QAction * undo;
    QAction * redo;
    QList<QImage> imageListUndo;
    QList<QImage> imageListRedo;
public:
    Memento(QAction *undo, QAction *redo);
    void mementoImage(QImage image);
    QImage undoImage();
    QImage redoImage();
    void drawInit(QImage image);
    void clear(QImage image);
};

#endif // MEMENTO_H
