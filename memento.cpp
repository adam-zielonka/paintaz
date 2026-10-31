#include "memento.h"

Memento::Memento(QAction *undo, QAction *redo)
{
    this->undo = undo;
    this->redo = redo;

    this->undo->setDisabled(true);
    this->redo->setDisabled(true);
}

void Memento::mementoImage(QImage image)
{
    // The undo stack always ends with the current image. A no-op must not
    // create an undo step or discard an existing redo branch.
    if (!imageListUndo.isEmpty() && imageListUndo.last() == image)
        return;
    imageListUndo << image;
    undo->setEnabled(imageListUndo.size() > 1);
    imageListRedo.clear();
    redo->setEnabled(false);
}

QImage Memento::undoImage()
{
    if (imageListUndo.size() > 1)
        imageListRedo << imageListUndo.takeLast();
    undo->setEnabled(imageListUndo.size() > 1);
    redo->setEnabled(!imageListRedo.isEmpty());
    return imageListUndo.isEmpty() ? QImage() : imageListUndo.last();
}

QImage Memento::redoImage()
{
    if (!imageListRedo.isEmpty())
        imageListUndo << imageListRedo.takeLast();
    undo->setEnabled(imageListUndo.size() > 1);
    redo->setEnabled(!imageListRedo.isEmpty());
    return imageListUndo.isEmpty() ? QImage() : imageListUndo.last();
}

void Memento::drawInit(QImage image)
{
    if(imageListUndo.isEmpty())
        imageListUndo << image;
}

void Memento::clear(QImage image)
{
    imageListUndo.clear();
    imageListUndo << image;
    imageListRedo.clear();
    undo->setDisabled(true);
    redo->setDisabled(true);
}
