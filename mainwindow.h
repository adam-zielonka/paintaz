#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <widget.h>
#include <newfiledialog.h>
#include <QPushButton>
#include <QColorDialog>
#include <QColor>
#include <QDebug>
#include <QFileDialog>
#include <QScrollArea>
#include <QLabel>
#include <QFontComboBox>
#include <QSpinBox>
#include <QLineEdit>
#include <QFont>
#include <QDesktopServices>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    QPushButton *brushColorButton;
    QPushButton *penColorButton;
    NewFileDialog * newFile;
    QScrollArea * scrollArea;
    QLabel * mouseLabel;
    void updateWindowTitle();
    QLineEdit * textEdit;
    QFontComboBox * fontComboBox;
    QSpinBox * fontSizeSpinBox;
    QPushButton * boldFontButton;
    QPushButton * italicsFontButton;
    QSpinBox * drawSizeSpinBox;
    void keyPressEvent(QKeyEvent *event);
    void keyReleaseEvent(QKeyEvent *event);
    QPushButton *icons8Link;
    QPushButton *myLink;

public slots:
    void setPenColor();
    void setBrushColor();
    void setBoldFont();
    void setItalicsFont();
    void icons8LinkClicked();
    void myLinkClicked();

private slots:
    void on_actionDraw_triggered();

    void on_actionDraw_Line_triggered();

    void on_actionDraw_Rect_triggered();

    void on_actionDraw_Ellipse_triggered();

    void on_actionNormal_triggered();

    void on_actionEqual_triggered();

    void on_actionNone_triggered();

    void on_actionFull_triggered();

    void on_actionNew_triggered();

    void on_actionOpen_triggered();

    void on_actionSave_triggered();

    void on_actionSave_As_triggered();

    void on_actionZoom_In_triggered();

    void on_actionZoom_Out_triggered();

    void on_actionZoom_to_Actual_Size_triggered();

    void on_actionText_triggered();

    void on_actionUndo_triggered();

    void on_actionRedo_triggered();

    void on_actionCross_triggered();

    void on_actionDraw_Round_Rect_triggered();

    void on_actionDraw_Size_Up_triggered();

    void on_actionDraw_Size_Down_triggered();

    void on_actionFill_triggered();

    void on_actionColor_Picker_triggered();

    void on_actionErase_triggered();

private:
#ifdef Q_OS_WASM
    void downloadImage();
#endif
    Ui::MainWindow *ui;
    Widget *mywidget;
};

#endif // MAINWINDOW_H
