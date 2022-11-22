#include "fmwdialog.h"
#include <QDialog>
fmwDialog::fmwDialog(QWidget* pwgt) : QDialog(pwgt, Qt::WindowTitleHint | Qt::WindowSystemMenuHint)
{
    setWindowTitle("Введите индекс: ");
    indexLine = new QLineEdit;
    QLabel* lbltxt = new QLabel ("&Index:");
    lbltxt->setBuddy(indexLine);
    QIntValidator* validi = new QIntValidator(0,99, this);
    indexLine->setValidator(validi);
    QPushButton* okButtonDialog = new QPushButton("&Ok");
   // QPushButton* cancelButtonDialog = new QPushButton("&Cancel");
    connect(okButtonDialog, SIGNAL(clicked()), SLOT(accept()));
   // connect(cancelButtonDialog, SIGNAL(clicked()), SLOT(reject()));

    QGridLayout* ptopLayout = new QGridLayout;
    ptopLayout->addWidget(lbltxt, 0, 0);
    ptopLayout->addWidget(indexLine, 1, 0);
    ptopLayout->addWidget(okButtonDialog, 2, 0);
    setLayout(ptopLayout);
    setAttribute(Qt::WA_DeleteOnClose); // удаляем при закрытии
}

QString fmwDialog::txt() const
{
    return indexLine->text();
}

void fmwDialog::setIndexLine(QString str)
{
    indexLine->setText(str);
}

//void fmwDialog::setIndexValidator(int index)
//{
//    //qDebug() << "MAXINDEX " << index;
//    QIntValidator* validi = new QIntValidator(0, index, this);
//    //indexLine->setValidator(validi);
//    //QIntValidator* validi = new QIntValidator/*(0,999, this)*/;
//    indexLine->setValidator(validi);
//    validi->setTop(index);
//   // validi->setTop(index);
//}

