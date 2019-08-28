#ifndef CONNECTDIALOG_H
#define CONNECTDIALOG_H

#include <QDialog>

namespace Ui{
class connectDialog;
}

class connectDialog : public QDialog
{
     Q_OBJECT

public:
    explicit connectDialog(QWidget *parent = nullptr);
    ~connectDialog();

signals:
    void ipAddress(QString);
    void abort();

public slots:
    void serverClicked();
    void clientClicked();
    void inputIPChanged(QString);
    void portChanged(QString);
    void cancelConnection();
    void checkFormat();

private:
    Ui::connectDialog *ui;
    int mode; // 0: server   1: client
    QStringList localIP;
    QString inputIP;
    QString portNumber;
    void startConnection();
};

#endif // CONNECTDIALOG_H
