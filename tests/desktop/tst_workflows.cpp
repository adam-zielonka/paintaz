#include <QtTest>
#include <QDialogButtonBox>
#include <QTemporaryDir>
#include "mainwindow.h"

class Workflows : public QObject
{
    Q_OBJECT
private slots:
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
