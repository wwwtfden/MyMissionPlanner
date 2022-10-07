#ifndef FMWDIALOG_H
#define FMWDIALOG_H

#include <QtWidgets>

class fmwDialog : public QDialog
{
    Q_OBJECT
private:
    QLineEdit* indexLine;
    QIntValidator* validi;
  //  QString str;
public:
    fmwDialog(QWidget* pwgt = 0);
    QString txt() const;
  //  QLabel* lbltxt;
    void setIndexLine(QString str);
//    void setIndexValidator(int index);

};

#endif // FMWDIALOG_H
