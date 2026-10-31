#ifndef NEWFILEDIALOG_H
#define NEWFILEDIALOG_H

#include <QDialog>

namespace Ui {
class NewFileDialog;
}

class NewFileDialog : public QDialog
{
    Q_OBJECT

public:
    explicit NewFileDialog(QWidget *parent = 0);
    ~NewFileDialog();
    int getWidth();
    int getHeight();
    void accept() override;

private:
    Ui::NewFileDialog *ui;
    int height;
    int width;

};

#endif // NEWFILEDIALOG_H
