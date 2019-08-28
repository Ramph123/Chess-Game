#ifndef CLIENTGAMECONFIG_H
#define CLIENTGAMECONFIG_H

#include <QDialog>

namespace Ui {
class clientGameConfig;
}

class clientGameConfig : public QDialog
{
    Q_OBJECT

public:
    explicit clientGameConfig(QWidget *parent = nullptr);
    ~clientGameConfig();

    void setSide(bool side);
    void setTimeLimit(bool enable, int time);

signals:
    void startGame();

public slots:
    void send();

private:
    Ui::clientGameConfig *ui;
};

#endif // CLIENTGAMECONFIG_H
