#include "configurator.h"
#include "ui_configurator.h"

configurator::configurator(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::configurator)
{
    ui->setupUi(this);
}

configurator::~configurator()
{
    delete ui;
}

void configurator::on_buttonSave_clicked()
{
    emit save_new_configuration(ui->lineMqtt->text(), ui->lineRead->text(), ui->lineWrite->text(), ui->lineUsername->text());
    window()->close();
}
