#include "memento.h"

Memento::Memento(QAction *undo, QAction *redo)
{
    this->undoAgain = false;
    this->redoAgain = false;
    this->undo = undo;
    this->redo = redo;

    this->undo->setDisabled(true);
    this->redo->setDisabled(true);
}

void Memento::mementoImage(QImage image)
{
    if(!imageListUndo.isEmpty())
    {
        if(imageListUndo.last() != image)
        {
            imageListUndo << image;
        }
    }
    else
    {
        imageListUndo << image;
    }
    undo->setEnabled(true);
    undoAgain = false;

    imageListRedo.clear();
    redo->setEnabled(false);
    redoAgain = false;
}

QImage Memento::undoImage()
{
    if(!undoAgain)
    {
        imageListRedo << imageListUndo.takeLast();
    }
    undoAgain = true;
    redoAgain = false;
    QImage image = imageListUndo.takeLast();
    if(!imageListUndo.isEmpty())
        undo->setEnabled(true);
    else
    {
        undo->setEnabled(false);
        imageListUndo << image;
    }

    redo->setEnabled(true);
    imageListRedo << image;

    return image;
}

QImage Memento::redoImage()
{
    if(!redoAgain)
    {
        if(imageListUndo.size() != 1)
            imageListUndo << imageListRedo.takeLast();
        else
            imageListRedo.takeLast();
    }
    redoAgain = true;
    undoAgain = false;
    QImage image = imageListRedo.takeLast();
    if(!imageListRedo.isEmpty())
        redo->setEnabled(true);
    else
        redo->setEnabled(false);

    undo->setEnabled(true);
    imageListUndo << image;

    return image;
}

void Memento::drawInit(QImage image)
{
    if(imageListUndo.isEmpty())
    {
        imageListUndo << image;
        undoAgain = false;
    }
    else if(undoAgain && imageListUndo.last() != image)
    {
        imageListUndo << image;
        undoAgain = false;
    }
}

void Memento::clear(QImage image)
{
    imageListUndo.clear();
    imageListUndo << image;
    imageListRedo.clear();
    undo->setDisabled(true);
    redo->setDisabled(true);
    undoAgain = false;
    redoAgain = false;
}
