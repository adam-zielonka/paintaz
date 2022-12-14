#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    mywidget = new Widget(this,ui->actionUndo,ui->actionRedo);
    scrollArea = new QScrollArea(this);
    scrollArea->setWidget(mywidget);
    scrollArea->setWidgetResizable(true);
    setCentralWidget(scrollArea);

    penColorButton = new QPushButton(this);
    penColorButton->setStyleSheet("background-color: "+mywidget->getPenColor().name()+";");
    ui->toolBar->addWidget(penColorButton);
    connect(penColorButton,SIGNAL(clicked()),this,SLOT(setPenColor()));

    brushColorButton = new QPushButton(this);
    brushColorButton->setStyleSheet("background-color: "+mywidget->getBrushColor().name()+";");
    ui->toolBar->addWidget(brushColorButton);
    connect(brushColorButton,SIGNAL(clicked()),this,SLOT(setBrushColor()));

    mywidget->setColorButtons(penColorButton,brushColorButton);

    newFile = new NewFileDialog();

    updateWindowTitle();

    mouseLabel = new QLabel(this);
    ui->statusbar->addPermanentWidget(mouseLabel);
    mywidget->setMouseLabel(mouseLabel);

    fontComboBox = new QFontComboBox(this);
    fontSizeSpinBox = new QSpinBox(this);
    fontSizeSpinBox->setValue(22);
    textEdit = new QLineEdit(this);
    boldFontButton = new QPushButton(this);
    boldFontButton->setText("bold");
    boldFontButton->setStyleSheet("font-weight: bold;");
    connect(boldFontButton,SIGNAL(clicked()),this,SLOT(setBoldFont()));
    italicsFontButton = new QPushButton(this);
    italicsFontButton->setText("italics");
    italicsFontButton->setStyleSheet("font-style: italic;");
    connect(italicsFontButton,SIGNAL(clicked()),this,SLOT(setItalicsFont()));
    ui->toolBarText->addWidget(fontComboBox);
    ui->toolBarText->addWidget(fontSizeSpinBox);
    ui->toolBarText->addWidget(boldFontButton);
    ui->toolBarText->addWidget(italicsFontButton);
    ui->toolBarText->addWidget(textEdit);
    mywidget->setTextEdit(textEdit,fontSizeSpinBox,fontComboBox);

    drawSizeSpinBox = new QSpinBox(this);
    drawSizeSpinBox->setValue(1);
    ui->toolBarShape->addWidget(drawSizeSpinBox);
    mywidget->setDrawSize(drawSizeSpinBox);
    setCursor(Qt::ArrowCursor);

    icons8Link = new QPushButton(this);
    icons8Link->setText(" icons8.com ");
    icons8Link->setStyleSheet("text-decoration: underline; color: blue");
    myLink = new QPushButton(this);
    myLink->setText(" adamzielonka.pro ");
    myLink->setStyleSheet("text-decoration: underline; color: blue");
    ui->toolBarAbout->addWidget(new QLabel("Icons from: ", this));
    ui->toolBarAbout->addWidget(icons8Link);
    ui->toolBarAbout->addSeparator();
    ui->toolBarAbout->addWidget(new QLabel("Author: ", this));
    ui->toolBarAbout->addWidget(myLink);
    connect(icons8Link,SIGNAL(clicked()),this,SLOT(icons8LinkClicked()));
    connect(myLink,SIGNAL(clicked()),this,SLOT(myLinkClicked()));
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::updateWindowTitle()
{
    if(mywidget->getFileName() == "")
        setWindowTitle("paintAZ");
    else
        setWindowTitle("paintAZ - "+mywidget->getFileName());
}

void MainWindow::keyPressEvent(QKeyEvent *event)
{
    if(event->key() == Qt::Key_Shift)
    {
        mywidget->setShapeEqual();
    }
}

void MainWindow::keyReleaseEvent(QKeyEvent *event)
{
    if(event->key() == Qt::Key_Shift)
    {
        mywidget->setShapeNormal();
    }
}

void MainWindow::setPenColor()
{
    QColor color = QColorDialog::getColor(mywidget->getPenColor(),this);
    if(color.isValid())
    {
        mywidget->setPenColor(color);
        penColorButton->setStyleSheet("background-color: "+mywidget->getPenColor().name()+";");
    }
}

void MainWindow::setBrushColor()
{
    QColor color = QColorDialog::getColor(mywidget->getPenColor(),this);
    if(color.isValid())
    {
        mywidget->setBrushColor(color);
        brushColorButton->setStyleSheet("background-color: "+mywidget->getBrushColor().name()+";");
    }
}

void MainWindow::setBoldFont()
{
    if(!mywidget->setBoldFont())
        boldFontButton->setStyleSheet("font-weight: bold;");
    else
        boldFontButton->setStyleSheet("font-weight: bold;"
                                      "background-color: black;"
                                      "color: white");
}

void MainWindow::setItalicsFont()
{
    if(!mywidget->setItalicsFont())
        italicsFontButton->setStyleSheet("font-style: italic;");
    else
        italicsFontButton->setStyleSheet("font-style: italic;"
                                         "background-color: black;"
                                         "color: white");
}

void MainWindow::icons8LinkClicked()
{
    QString link = "https://icons8.com/";
    QDesktopServices::openUrl(QUrl(link));
}

void MainWindow::myLinkClicked()
{
    QString link = "https://adamzielonka.pro/";
    QDesktopServices::openUrl(QUrl(link));
}

void MainWindow::on_actionDraw_triggered()
{
    mywidget->setDraw();
}

void MainWindow::on_actionDraw_Line_triggered()
{
    mywidget->setDrawLine();
}

void MainWindow::on_actionDraw_Rect_triggered()
{
    mywidget->setDrawRect();
}

void MainWindow::on_actionDraw_Ellipse_triggered()
{
    mywidget->setDrawEllipse();
}

void MainWindow::on_actionNormal_triggered()
{
    mywidget->setShapeNormal();
}

void MainWindow::on_actionEqual_triggered()
{
    mywidget->setShapeEqual();
}

void MainWindow::on_actionNone_triggered()
{
    mywidget->setBrushNone();
}

void MainWindow::on_actionFull_triggered()
{
    mywidget->setBrushFull();
}

void MainWindow::on_actionNew_triggered()
{
    newFile->exec();
    if(newFile->checkAccept())
    {
        mywidget->setNewImage(newFile->getWidth(),newFile->getHeight());
        mywidget->update();
        ui->actionSave->setEnabled(false);
        updateWindowTitle();
    }
}

void MainWindow::on_actionOpen_triggered()
{
    QString selfilter = tr("JPEG (*.jpg *.jpeg)");
    QString file = QFileDialog::getOpenFileName(
            this,
            QString(),
            QString(),
            tr("All files (*.*);;BITMAP (*.bmp);;PNG (*.png);;JPEG (*.jpg *.jpeg);;TIFF (*.tif *.tiff)" ),
            &selfilter
    );

    if(file != "")
    {
        while(!mywidget->loadImage(file))
        {
            file = QFileDialog::getOpenFileName(
                        this,
                        QString(),
                        QString(),
                        tr("All files (*.*);;BITMAP (*.bmp);;PNG (*.png);;JPEG (*.jpg *.jpeg);;TIFF (*.tif *.tiff)" ),
                        &selfilter
            );
            if(file == "") return;
        }
        mywidget->update();
        ui->actionSave->setEnabled(true);
        updateWindowTitle();
    }
}

void MainWindow::on_actionSave_triggered()
{
    mywidget->saveImage();
}

void MainWindow::on_actionSave_As_triggered()
{
    QString selfilter = tr("JPEG (*.jpg *.jpeg)");
    QString file = QFileDialog::getSaveFileName(
            this,
            QString(),
            QString(),
            tr("BITMAP (*.bmp);;PNG (*.png);;JPEG (*.jpg *.jpeg);;TIFF (*.tif *.tiff)" ),
            &selfilter
    );
    if(file != "")
    {
        while(!mywidget->saveImage(file))
        {
            file = QFileDialog::getSaveFileName(
                        this,
                        QString(),
                        QString(),
                        tr("BITMAP (*.bmp);;PNG (*.png);;JPEG (*.jpg *.jpeg);;TIFF (*.tif *.tiff)" ),
                        &selfilter
            );
            if(file == "") return;
        }
        ui->actionSave->setEnabled(true);
        updateWindowTitle();
    }
}

void MainWindow::on_actionZoom_In_triggered()
{
    ui->actionZoom_In->setEnabled(mywidget->zoomIn());
    ui->actionZoom_Out->setEnabled(true);
}

void MainWindow::on_actionZoom_Out_triggered()
{
    ui->actionZoom_Out->setEnabled(mywidget->zoomOut());
    ui->actionZoom_In->setEnabled(true);
}

void MainWindow::on_actionZoom_to_Actual_Size_triggered()
{
    ui->actionZoom_In->setEnabled(true);
    ui->actionZoom_Out->setEnabled(true);
    mywidget->zoomReset();
}

void MainWindow::on_actionText_triggered()
{
    mywidget->setPaintText();
}

void MainWindow::on_actionCross_triggered()
{
    mywidget->setPaintDefault();
}


void MainWindow::on_actionUndo_triggered()
{
    mywidget->undoImage();
}

void MainWindow::on_actionRedo_triggered()
{
    mywidget->redoImage();
}

void MainWindow::on_actionDraw_Round_Rect_triggered()
{
    mywidget->setDrawRoundRect();
}

void MainWindow::on_actionDraw_Size_Up_triggered()
{
    drawSizeSpinBox->setValue(drawSizeSpinBox->value()+1);
    mywidget->update();
}

void MainWindow::on_actionDraw_Size_Down_triggered()
{
    drawSizeSpinBox->setValue(drawSizeSpinBox->value()-1);
    mywidget->update();
}

void MainWindow::on_actionFill_triggered()
{
    mywidget->setFill();
}

void MainWindow::on_actionColor_Picker_triggered()
{
    mywidget->setColorPicker();
}

void MainWindow::on_actionErase_triggered()
{
    mywidget->setErase();
}
