#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMqttClient>
#include <QApplication>

#include "chart_view.h"
#include "configurator.h"

#define MQTT_KEY_ "1SPREK71PEVYY4M2"
#define READ_KEY_ "/5OVE2SO8WOEFN8PX"
#define WRITE_KEY_  "/U91IJJTUDK2CTCK0"


namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    enum class ConnectionMode {
        PUBLISH_MODE, SUBSCRIBE_MODE
    };

    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

signals:
    void main_window_closed();

public slots:
    void setClientPort(int p);

private slots:
    void on_buttonConnect_clicked();
    void update_connection_state_label();

    void brokerDisconnected();

    void on_buttonPublish_clicked();

    void on_buttonSubscribe_clicked();

    void on_buttonChart_clicked();

    void on_actionConfigure_triggered();

private:
    Ui::MainWindow *ui;
    QMqttClient *m_client;
    chart_view *ui_chart;
    configurator *configurator_modal;
    QDialog *chart_dialog;
    QMap<QString, QList<double>> *series_map;

    QString read_key;
    QString write_key;
    QString mqtt_key;
    QString user;
    QString settings_path =  QApplication::applicationDirPath() + "/mqtt_connection_settings.ini";


    void closeEvent(QCloseEvent *event) override;
    QString prepare_response_topic(const QString &channel, MainWindow::ConnectionMode &&mode) const;
    QString convert_connection_state() const;
    void set_button_enabled(bool enabled) const;
    QString extract_field(const std::string &topic) const;
     void add_fields_to_map(const QString &field_name, const QString &message) const;
    void add_to_map();
    void load_settings();
    void save_settings() const;
    void refresh_settings(QString new_mqtt_key, QString new_read_key, QString new_write_key, QString new_user);
};

#endif // MAINWINDOW_H
