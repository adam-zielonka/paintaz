#include "widget.h"
#include "ui_widget.h"
#include <QBuffer>
#include <QImageReader>

Widget::Widget(QWidget *parent, QAction * actionUndo, QAction * actionRedo) :
    QWidget(parent),
    ui(new Ui::Widget),
    image(800,600, QImage::Format_RGB32),
    tempImage(800,600, QImage::Format_RGB32),
    memento(actionUndo, actionRedo)
{
    penColor = QColor(Qt::black);
    brushColor = QColor(Qt::white);
    ui->setupUi(this);
    image.fill(Qt::white);
    tempImage = image;
    setCursor(Qt::CrossCursor);
    mode = DRAW;
    shape = NORMAL;
    brush = NONE;
    scale = 1.0;
    setMinimumSize(800*scale,600*scale);
    fileName = "";
    setMouseTracking(true);
    boldFont = false;
    italicsFont = false;
}

Widget::~Widget()
{
    delete ui;
}

bool Widget::validImageSize(const QSize &size)
{
    return size.width() > 0 && size.height() > 0 && size.width() <= 4096
        && size.height() <= 4096 && qint64(size.width()) * size.height() <= 4194304;
}

bool Widget::replaceImage(const QImage &next, const QString &name)
{
    if (next.isNull() || !validImageSize(next.size())) return false;
    image = next;
    tempImage = image;
    setMinimumSize(image.width()*scale, image.height()*scale);
    fileName = name;
    memento.clear(image);
    update();
    return true;
}

bool Widget::setNewImage(int width, int height)
{
    if (!validImageSize(QSize(width, height))) return false;
    QImage next(width, height, QImage::Format_RGB32);
    if (next.isNull()) return false;
    next.fill(Qt::white);
    return replaceImage(next, QString());
}

bool Widget::loadImage(QString file)
{
    QImageReader reader(file);
    if (!validImageSize(reader.size())) return false;
    return replaceImage(reader.read(), file);
}

bool Widget::loadImageData(const QByteArray &data, const QString &name)
{
    if (data.size() > 32 * 1024 * 1024) return false;
    QBuffer buffer;
    buffer.setData(data);
    buffer.open(QIODevice::ReadOnly);
    QImageReader reader(&buffer);
    if (!validImageSize(reader.size())) return false;
    return replaceImage(reader.read(), name);
}

QByteArray Widget::imageData(const char *format) const
{
    QByteArray data;
    QBuffer buffer(&data);
    if (!buffer.open(QIODevice::WriteOnly) || !image.save(&buffer, format)) return QByteArray();
    return data;
}

void Widget::saveImage()
{
    image.save(fileName);
}

QString Widget::getFileName()
{
    return fileName;
}

bool Widget::saveImage(QString file)
{
    if(image.save(file))
    {
        fileName = file;
        return true;
    }
    else
        return false;
}

QColor Widget::getPenColor()
{
    return penColor;
}

void Widget::setPenColor(QColor color)
{
    penColor = color;
}

QColor Widget::getBrushColor()
{
    return brushColor;
}

void Widget::setBrushColor(QColor color)
{
    brushColor = color;
}

void Widget::setColorButtons(QPushButton *penColorButton, QPushButton *brushColorButton)
{
    this->penColorButton = penColorButton;
    this->brushColorButton = brushColorButton;
}

void Widget::setDrawSize(QSpinBox * drawSizeSpinBox)
{
    this->drawSizeSpinBox = drawSizeSpinBox;
}

void Widget::mousePressEvent(QMouseEvent * e)
{
    lastX = e->x()/scale;
    lastY = e->y()/scale;
}

QRect Widget::GetRect(int x,int y)
{
    QRect rect = QRect();
    if(shape == EQUAL)
        rect.setRect(lastX,lastY,x-lastX,x-lastX);
    else
        rect.setCoords(lastX,lastY,x,y);
    return rect;
}

QImage * Widget::GetImage()
{
    if(IsTempImage())
    {
        tempImage = image;
        return &tempImage;
    }
    else
        return &image;
}

bool Widget::IsTempImage()
{
    return (mode == DRAW_LINE ||
            mode == DRAW_RECT ||
            mode == DRAW_ROUND_RECT ||
            mode == DRAW_ELLIPSE ||
            mode == ERASE);
}

void Widget::changeMouseLabel(int x, int y)
{
    if ((image.size().rwidth() >= x/scale)
            && (image.size().rheight() >= y/scale)
            && (0 <= x)
            && (0 <= y)
            )
        mouseLabel->setText("x: "+QString::number((int)(x/scale))+" y: "+QString::number((int)(y/scale)));
}

void Widget::mouseMoveEvent(QMouseEvent * e)
{
    memento.drawInit(image);

    changeMouseLabel(e->x(),e->y());
    if(e->buttons() == Qt::LeftButton)
    {
        QPainter p(GetImage());
        p.setPen(QPen(penColor, drawSizeSpinBox->value()));
        if (brush == FULL)
            p.setBrush(brushColor);

        switch (mode) {
        case DRAW:
            p.drawLine(e->x()/scale,e->y()/scale,lastX,lastY);
            lastX = e->x()/scale;
            lastY = e->y()/scale;
            break;
        case DRAW_LINE:
            p.drawLine(e->x()/scale,e->y()/scale,lastX,lastY);
            break;
        case DRAW_RECT:
            p.drawRect(GetRect(e->x()/scale,e->y()/scale));
            actualX = e->x()/scale;
            actualY = e->y()/scale;
            break;
        case DRAW_ROUND_RECT:
            p.drawRoundedRect(GetRect(e->x()/scale,e->y()/scale),25,25,Qt::RelativeSize);
            actualX = e->x()/scale;
            actualY = e->y()/scale;
            break;
        case DRAW_ELLIPSE:
            p.drawEllipse(GetRect(e->x()/scale,e->y()/scale));
            actualX = e->x()/scale;
            actualY = e->y()/scale;
            break;
        case FILL:
            break;
        case COLOR_PICKER:
            break;
        case ERASE:
            p.setPen(penColor);
            p.setBrush(brushColor);
            p.drawRect(e->x()/scale,e->y()/scale,10+drawSizeSpinBox->value(),10+drawSizeSpinBox->value());
            p.end();
            p.begin(&image);
            p.setPen(brushColor);
            p.setBrush(brushColor);
            p.drawRect(e->x()/scale,e->y()/scale,10+drawSizeSpinBox->value(),10+drawSizeSpinBox->value());
            break;
        case TEXT:
            break;
        }

        update();
    }
    else if(mode == ERASE)
    {
        QPainter p(GetImage());
        p.setPen(penColor);
        p.setBrush(brushColor);
        p.drawRect(e->x()/scale,e->y()/scale,10+drawSizeSpinBox->value(),10+drawSizeSpinBox->value());
        update();
    }
}

void Widget::mouseReleaseEvent(QMouseEvent * e)
{
    memento.drawInit(image);
    int x = e->x()/scale;
    int y = e->y()/scale;

    QPainter p(&image);
    p.setPen(QPen(penColor, drawSizeSpinBox->value()));
    if (brush == FULL)
        p.setBrush(brushColor);

    switch (mode) {
    case DRAW:
        break;
    case DRAW_LINE:
        p.drawLine(x,y,lastX,lastY);
        break;
    case DRAW_RECT:
        p.drawRect(GetRect(x,y));
        break;
    case DRAW_ROUND_RECT:
        p.drawRoundedRect(GetRect(x,y),25,25,Qt::RelativeSize);
        break;
    case DRAW_ELLIPSE:
        p.drawEllipse(GetRect(x,y));
        break;
    case FILL:
        if(image.pixelColor(x,y) != penColor)
        {
            QColor color = image.pixelColor(x,y);
            p.setPen(QPen(penColor, 1));
            FillSpace fillSpace(&image,&p,color,x,y);
        }
        break;
    case COLOR_PICKER:
        setPenColor(image.pixelColor(x,y));
        penColorButton->setStyleSheet("background-color: "+getPenColor().name()+";");
        break;
    case ERASE:
        p.setPen(brushColor);
        p.setBrush(brushColor);
        p.drawRect(e->x()/scale,e->y()/scale,10+drawSizeSpinBox->value(),10+drawSizeSpinBox->value());
        break;
    case TEXT:
        QFont font = QFont(fontComboBox->currentFont());
        font.setPointSize(fontSizeSpinBox->value());
        font.setBold(boldFont);
        font.setItalic(italicsFont);
        p.setFont(font);
        p.drawText(x,y,textEdit->text());
        break;
    }

    lastX = x;
    lastY = y;

    memento.mementoImage(image);

    update();
}


void Widget::paintEvent(QPaintEvent * event)
{
    event->accept();
    QPainter p(this);
    p.scale(scale,scale);
    if(IsTempImage())
        p.drawImage(0,0,tempImage);
    else
        p.drawImage(0,0,image);
}

void Widget::setDraw()
{
    setCursor(Qt::CrossCursor);
    mode = DRAW;
    update();
}

void Widget::setDrawLine()
{
    setCursor(Qt::CrossCursor);
    mode = DRAW_LINE;
    tempImage = image;
    update();
}

void Widget::setDrawRect()
{
    setCursor(Qt::CrossCursor);
    mode = DRAW_RECT;
    tempImage = image;
    update();
}

void Widget::setDrawEllipse()
{
    setCursor(Qt::CrossCursor);
    mode = DRAW_ELLIPSE;
    tempImage = image;
    update();
}

void Widget::setDrawRoundRect()
{
    setCursor(Qt::CrossCursor);
    mode = DRAW_ROUND_RECT;
    tempImage = image;
    update();
}

void Widget::setFill()
{
    setCursor(Qt::PointingHandCursor);
    mode = FILL;
    tempImage = image;
    update();
}

void Widget::setColorPicker()
{
    setCursor(Qt::ArrowCursor);
    mode = COLOR_PICKER;
    tempImage = image;
    update();
}

void Widget::setErase()
{
    setCursor(Qt::BlankCursor);
    mode = ERASE;
    tempImage = image;
    update();
}

void Widget::setShapeNormal()
{
    shape = NORMAL;
    update();
}

void Widget::setShapeEqual()
{
    shape = EQUAL;
    update();
}

void Widget::setBrushNone()
{
    brush = NONE;
    update();
}

void Widget::setBrushFull()
{
    brush = FULL;
    update();
}

bool Widget::zoomIn()
{
    if(scale+0.1 <= 3.0)
    {
        scale+=0.1;
        setMinimumSize(image.size().rwidth()*scale,image.size().rheight()*scale);
        update();
    }

    if(scale+0.1 <= 3.0)
        return true;
    else
        return false;
}

bool Widget::zoomOut()
{
    if(scale-0.1 >= 0.1)
    {
        scale-=0.1;
        setMinimumSize(image.size().rwidth()*scale,image.size().rheight()*scale);
        update();
    }

    if(scale-0.1 >= 0.1)
        return true;
    else
        return false;
}

void Widget::zoomReset()
{
    scale = 1.0;
    setMinimumSize(image.size().rwidth()*scale,image.size().rheight()*scale);
    update();
}

void Widget::setMouseLabel(QLabel *label)
{
    mouseLabel = label;
}

void Widget::undoImage()
{
    image = tempImage = memento.undoImage();
    update();
}

void Widget::redoImage()
{
    image = tempImage = memento.redoImage();
    update();
}

void Widget::setPaintDefault()
{
    setCursor(Qt::CrossCursor);
    mode = DRAW;
    update();
}

void Widget::setPaintText()
{
    setCursor(Qt::IBeamCursor);
    mode = TEXT;
    update();
}

void Widget::setTextEdit(QLineEdit * textEdit,QSpinBox * fontSizeSpinBox,QFontComboBox * fontComboBox)
{
    this->textEdit = textEdit;
    this->fontSizeSpinBox = fontSizeSpinBox;
    this->fontComboBox = fontComboBox;
}

bool Widget::setBoldFont()
{
    boldFont = !boldFont;
    return boldFont;
}

bool Widget::setItalicsFont()
{
    italicsFont = !italicsFont;
    return italicsFont;
}
