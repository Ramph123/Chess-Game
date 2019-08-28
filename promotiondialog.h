#ifndef PROMOTIONDIALOG_H
#define PROMOTIONDIALOG_H

#include <QDialog>

namespace Ui {
class promotionDialog;
}

class promotionDialog : public QDialog
{
    Q_OBJECT

public:
    explicit promotionDialog(QString side, int row, int col, QWidget *parent = nullptr);
    ~promotionDialog();

signals:
    void returnAns(QString, int, int);

public slots:
    void queenPushed();
    void rookPushed();
    void knightPushed();
    void bishopPushed();

private:
    Ui::promotionDialog *ui;
    int row, col;
    QString side;
};

#endif // PROMOTIONDIALOG_H
