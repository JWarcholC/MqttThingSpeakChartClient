#ifndef CONFIGURATOR_H
#define CONFIGURATOR_H

#include <QWidget>

namespace Ui {
class configurator;
}

class configurator : public QWidget
{
    Q_OBJECT

public:
    explicit configurator(QWidget *parent = nullptr);
    ~configurator();

private slots:
    void on_buttonSave_clicked();

signals:
    void save_new_configuration(QString, QString, QString, QString);

private:
    Ui::configurator *ui;
};

#endif // CONFIGURATOR_H
