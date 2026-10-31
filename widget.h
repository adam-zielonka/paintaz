#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QLabel>
#include <QMouseEvent>
#include <QPaintEvent>
#include <QPainter>
#include <QImage>
#include <QRect>
#include <cmath>
#include <QList>
#include <QTextEdit>
#include <QSpinBox>
#include <QFontComboBox>
#include <QLineEdit>
#include <QPushButton>
#include "memento.h"
#include "fillspace.h"

enum Mode{
    DRAW,
    DRAW_LINE,
    DRAW_RECT,
    DRAW_ROUND_RECT,
    DRAW_ELLIPSE,
    FILL,
    COLOR_PICKER,
    ERASE,
    TEXT
};

enum Shape{
    NORMAL,
    EQUAL
};

enum Brush{
    NONE,
    FULL
};

namespace Ui {
class Widget;
}

class Widget : public QWidget
{
    Q_OBJECT

public:
    explicit Widget(QWidget *parent = 0, QAction * actionUndo = 0, QAction * actionRedo = 0);
    ~Widget();

    static bool validImageSize(const QSize &size);
    bool setNewImage(int width, int height);
    bool loadImageData(const QByteArray &data, const QString &name);
    QByteArray imageData(const char *format) const;
    bool loadImage(QString file);
    bool saveImage(QString file);
    void saveImage();
    QString getFileName();

    QColor getPenColor();
    void setPenColor(QColor color);
    QColor getBrushColor();
    void setBrushColor(QColor color);
    void setColorButtons(QPushButton * penColorButton, QPushButton * brushColorButton);

    void setDrawSize(QSpinBox * drawSizeSpinBox);

    void setDraw();
    void setDrawLine();
    void setDrawRect();
    void setDrawEllipse();
    void setDrawRoundRect();
    void setFill();
    void setColorPicker();
    void setErase();

    void setShapeNormal();
    void setShapeEqual();

    void setBrushNone();
    void setBrushFull();

    bool zoomIn();
    bool zoomOut();
    void zoomReset();

    void setMouseLabel(QLabel *label);

    void undoImage();
    void redoImage();

    void setPaintDefault();
    void setPaintText();
    void setTextEdit(QLineEdit * textEdit,QSpinBox * fontSizeSpinBox,QFontComboBox * fontComboBox);
    bool setBoldFont();
    bool setItalicsFont();
private:
    bool replaceImage(const QImage &next, const QString &name);
    Ui::Widget *ui;
    QString fileName;
    QImage image;
    QImage tempImage;
    QColor penColor;
    QColor brushColor;
    QPushButton * penColorButton;
    QPushButton * brushColorButton;
    int lastX;
    int lastY;
    int actualX;
    int actualY;
    Mode mode;
    Shape shape;
    Brush brush;
    void mousePressEvent(QMouseEvent *e);
    void mouseMoveEvent(QMouseEvent *e);
    void mouseReleaseEvent(QMouseEvent *e);
    void paintEvent(QPaintEvent * event);
    QRect GetRect(int x, int y);
    QImage *GetImage();
    bool IsTempImage();
    qreal scale;
    QLabel * mouseLabel;
    void changeMouseLabel(int x, int y);
    Memento memento;
    QLineEdit * textEdit;
    QSpinBox * fontSizeSpinBox;
    QFontComboBox * fontComboBox;
    bool boldFont;
    bool italicsFont;
    QSpinBox * drawSizeSpinBox;
};

#endif // WIDGET_H
