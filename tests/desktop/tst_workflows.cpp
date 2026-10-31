#include <QtTest>
#include <QDialogButtonBox>
#include <QTemporaryDir>
#include "mainwindow.h"

class Workflows : public QObject
{
    Q_OBJECT
private slots:
    void stationaryClickHistory()
    {
        MainWindow window;
        Widget *canvas = window.findChild<Widget *>();
        QVERIFY(canvas);
        QVERIFY(canvas->setNewImage(64, 48));
        auto undo = window.findChild<QAction *>("actionUndo");
        auto redo = window.findChild<QAction *>("actionRedo");
        QVERIFY(undo);
        QVERIFY(redo);
        const QImage blank = QImage::fromData(canvas->imageData("PNG"));
        canvas->setPaintDefault();
        QTest::mouseClick(canvas, Qt::LeftButton, Qt::NoModifier, QPoint(20, 20));
        QVERIFY(!undo->isEnabled());
        canvas->undoImage();
        QCOMPARE(QImage::fromData(canvas->imageData("PNG")), blank);
        canvas->setDrawLine();
        QTest::mousePress(canvas, Qt::LeftButton, Qt::NoModifier, QPoint(5, 8));
        QTest::mouseRelease(canvas, Qt::LeftButton, Qt::NoModifier, QPoint(50, 8));
        const QImage painted = QImage::fromData(canvas->imageData("PNG"));
        QCOMPARE(painted.pixelColor(20, 8), QColor(Qt::black));
        canvas->undoImage();
        QCOMPARE(QImage::fromData(canvas->imageData("PNG")), blank);
        canvas->setPaintDefault();
        QTest::mouseMove(canvas, QPoint(15, 15));
        QTest::mouseClick(canvas, Qt::LeftButton, Qt::NoModifier, QPoint(20, 20));
        QVERIFY(!undo->isEnabled());
        QVERIFY(redo->isEnabled());
        canvas->redoImage();
        QCOMPARE(QImage::fromData(canvas->imageData("PNG")), painted);
        QVERIFY(undo->isEnabled());
        QVERIFY(!redo->isEnabled());
    }
    void historyBranches()
    {
        QAction undo(nullptr), redo(nullptr);
        Memento history(&undo, &redo);
        QImage white(4, 4, QImage::Format_RGB32);
        white.fill(Qt::white);
        QImage black = white, red = white, green = white;
        black.fill(Qt::black);
        red.fill(Qt::red);
        green.fill(Qt::green);
        history.clear(white);
        history.mementoImage(black);
        history.mementoImage(red);
        QCOMPARE(history.undoImage(), black);
        history.drawInit(black); // Hover/pre-draw initialization cannot edit history.
        history.mementoImage(black);
        QCOMPARE(history.redoImage(), red);
        QCOMPARE(history.undoImage(), black);
        QCOMPARE(history.undoImage(), white);
        QVERIFY(!undo.isEnabled());
        QCOMPARE(history.undoImage(), white);
        QCOMPARE(history.redoImage(), black);
        history.mementoImage(green); // A real edit, unlike a no-op, replaces redo.
        QVERIFY(!redo.isEnabled());
        QCOMPARE(history.redoImage(), green);
        QCOMPARE(history.undoImage(), black);
        QCOMPARE(history.undoImage(), white);
    }
    void imageTransactions()
    {
        MainWindow window;
        Widget *canvas = window.findChild<Widget *>();
        QVERIFY(canvas);
        QVERIFY(canvas->setNewImage(64, 48));
        canvas->setDrawLine();
        QTest::mousePress(canvas, Qt::LeftButton, Qt::NoModifier, QPoint(5, 5));
        QTest::mouseRelease(canvas, Qt::LeftButton, Qt::NoModifier, QPoint(40, 5));
        QImage painted = QImage::fromData(canvas->imageData("PNG"));
        QCOMPARE(painted.pixelColor(20, 5), QColor(Qt::black));
        QVERIFY(!canvas->setNewImage(0, 48));
        QVERIFY(!canvas->setNewImage(4096, 4096));
        QVERIFY(!canvas->loadImageData("not an image", "bad.png"));
        QCOMPARE(QImage::fromData(canvas->imageData("PNG")), painted);
        canvas->undoImage();
        QCOMPARE(QImage::fromData(canvas->imageData("PNG")).pixelColor(20, 5), QColor(Qt::white));
        canvas->redoImage();
        QCOMPARE(QImage::fromData(canvas->imageData("PNG")), painted);
        QTemporaryDir dir;
        QVERIFY(canvas->saveImage(dir.filePath("image.png")));
        QVERIFY(!canvas->loadImage(dir.filePath("missing.png")));
        QCOMPARE(QImage::fromData(canvas->imageData("PNG")), painted);
        canvas->undoImage();
        QVERIFY(window.findChild<QAction *>("actionRedo")->isEnabled());
        QVERIFY(canvas->setNewImage(8, 8));
        QVERIFY(!window.findChild<QAction *>("actionRedo")->isEnabled());
        QVERIFY(canvas->loadImage(dir.filePath("image.png")));
        QCOMPARE(QImage::fromData(canvas->imageData("PNG")), painted);
        // Loading/new must disable both history actions, including stale redo.
        QVERIFY(!window.findChild<QAction *>("actionUndo")->isEnabled());
        QVERIFY(!window.findChild<QAction *>("actionRedo")->isEnabled());
    }
    void asynchronousDialogs()
    {
        MainWindow window;
        window.show();
        Widget *canvas = window.findChild<Widget *>();
        const QByteArray before = canvas->imageData("PNG");
        QVERIFY(QMetaObject::invokeMethod(&window, "on_actionNew_triggered"));
        QTRY_VERIFY(window.newFile->isVisible());
        auto width = window.newFile->findChild<QLineEdit *>("lineEditWidth");
        auto height = window.newFile->findChild<QLineEdit *>("lineEditHeight");
        width->setText("0"); height->setText("48");
        window.newFile->accept();
        QVERIFY(window.newFile->isVisible());
        QCOMPARE(canvas->imageData("PNG"), before);
        width->setText("64");
        window.newFile->reject();
        QCOMPARE(canvas->imageData("PNG"), before);
        QVERIFY(QMetaObject::invokeMethod(&window, "on_actionNew_triggered"));
        window.newFile->accept();
        QCOMPARE(QImage::fromData(canvas->imageData("PNG")).size(), QSize(64,48));
        for (bool pen : {true, false}) {
            const QColor initial = pen ? canvas->getPenColor() : canvas->getBrushColor();
            if (pen) window.setPenColor(); else window.setBrushColor();
            auto dialog = window.findChild<QColorDialog *>();
            QVERIFY(dialog);
            dialog->setCurrentColor(Qt::red);
            dialog->reject();
            QCOMPARE(pen ? canvas->getPenColor() : canvas->getBrushColor(), initial);
            QCoreApplication::sendPostedEvents(nullptr, QEvent::DeferredDelete);
            if (pen) window.setPenColor(); else window.setBrushColor();
            dialog = window.findChild<QColorDialog *>();
            QVERIFY(dialog);
            QCOMPARE(dialog->currentColor(), initial);
            dialog->setCurrentColor(Qt::green);
            dialog->accept();
            QCOMPARE(pen ? canvas->getPenColor() : canvas->getBrushColor(), QColor(Qt::green));
            QCoreApplication::sendPostedEvents(nullptr, QEvent::DeferredDelete);
        }
    }
};
QTEST_MAIN(Workflows)
#include "tst_workflows.moc"
