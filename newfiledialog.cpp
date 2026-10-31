#include "newfiledialog.h"
#include "ui_newfiledialog.h"
#include "widget.h"
#include <QIntValidator>

NewFileDialog::NewFileDialog(QWidget *parent) :
    QDialog(parent), ui(new Ui::NewFileDialog), height(600), width(800)
{
    ui->setupUi(this);
    ui->lineEditWidth->setValidator(new QIntValidator(1, 4096, this));
    ui->lineEditHeight->setValidator(new QIntValidator(1, 4096, this));
    ui->lineEditWidth->setText("800");
    ui->lineEditHeight->setText("600");
}
NewFileDialog::~NewFileDialog() { delete ui; }
int NewFileDialog::getWidth() { return width; }
int NewFileDialog::getHeight() { return height; }
void NewFileDialog::accept()
{
    const int w = ui->lineEditWidth->text().toInt();
    const int h = ui->lineEditHeight->text().toInt();
    if (!Widget::validImageSize(QSize(w, h))) {
        ui->labelError->setText(tr("Use 1–4096 per side, up to 4 megapixels."));
        return;
    }
    width = w;
    height = h;
    ui->labelError->clear();
    QDialog::accept();
}
