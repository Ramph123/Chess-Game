#ifndef CLIENTGAMECONFIG_H
#define CLIENTGAMECONFIG_H

#include <QDialog>

namespace Ui {
class clientGameConfig;
}

class clientGameConfig : public QDialog
{
public:
    explicit clientGameConfig(QWidget *parent = nullptr);
    ~clientGameConfig();

    void setSide(bool side);
    void setTimeLimit(bool enable, int time);

private:
    Ui::clientGameConfig *ui;
};

#endif // CLIENTGAMECONFIG_H
