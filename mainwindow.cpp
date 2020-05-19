#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QtCore/QDateTime>
#include <QtMqtt/QMqttClient>
#include <QtWidgets/QMessageBox>
#include <QSettings>
#include <cstring>


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    load_settings();
    ui->setupUi(this);
    series_map = new QMap<QString, QList<double>>();

    m_client = new QMqttClient(this);
    m_client->setHostname("mqtt.thingspeak.com");
    m_client->setPort(1883);

    connect(m_client, &QMqttClient::stateChanged, this, &MainWindow::update_connection_state_label);
    connect(m_client, &QMqttClient::disconnected, this, &MainWindow::brokerDisconnected);
    connect(m_client, &QMqttClient::disconnected, this, [this]{
        this->set_button_enabled(false);
    });
    connect(m_client, &QMqttClient::connected, this, [this]{
        this->set_button_enabled(true);
    });
    connect(m_client, &QMqttClient::messageReceived, this, [this](const QByteArray &message, const QMqttTopicName &topic) {
        auto field_name = extract_field(topic.name().toStdString());
        const QString content = QDateTime::currentDateTime().toString()
                + QLatin1String(" Received: ")
                + field_name
                + QLatin1String(" = ")
                + message
                + QLatin1Char('\n');

        ui->editLog->insertPlainText(content);
        add_fields_to_map(field_name, message);
    });
}

MainWindow::~MainWindow()
{
    if(ui_chart != nullptr) {
        delete ui_chart;
    }
    if(configurator_modal != nullptr) {
        delete configurator_modal;
    }
    delete series_map;
    delete ui;
}

void MainWindow::on_buttonConnect_clicked()
{
    if (m_client->state() == QMqttClient::Disconnected) {
        ui->buttonConnect->setText(tr("Disconnect"));
        m_client->setUsername(user);
        m_client->setPassword(mqtt_key);
        m_client->connectToHost();
    } else {
        ui->buttonConnect->setText(tr("Connect"));
        m_client->disconnectFromHost();
    }
}

void MainWindow::update_connection_state_label()
{
    ui->labelState->setText(convert_connection_state());
}

void MainWindow::brokerDisconnected()
{
    ui->buttonConnect->setText(tr("Connect"));
}

void MainWindow::setClientPort(int p)
{
    m_client->setPort(static_cast<quint16>(p));
}

void MainWindow::on_buttonPublish_clicked()
{
    if(ui->lineEditChannel->text().isEmpty() || ui->lineEditMessage->text().isEmpty()) {
        auto error_cause = QDateTime::currentDateTime().toString() +
                " Fill all fields!\n";
        ui->editLog->insertPlainText(error_cause);
        return;
    }

    if (m_client->publish(prepare_response_topic(ui->lineEditChannel->text(), MainWindow::ConnectionMode::PUBLISH_MODE), ui->lineEditMessage->text().toUtf8()) == -1)
        QMessageBox::critical(this, QLatin1String("Error"), QLatin1String("Could not publish message"));
}

void MainWindow::on_buttonSubscribe_clicked()
{
    if(ui->lineEditChannel->text().isEmpty()) {
        auto error_cause = QDateTime::currentDateTime().toString() +
                " Fill channelId field!\n";
        ui->editLog->insertPlainText(error_cause);
        return;
    }

    auto subscription = m_client->subscribe(prepare_response_topic(ui->lineEditChannel->text(), MainWindow::ConnectionMode::SUBSCRIBE_MODE));
    if (!subscription) {
        QMessageBox::critical(this, QLatin1String("Error"), QLatin1String("Could not subscribe. Is there a valid connection?"));
        return;
    }
}

QString MainWindow::prepare_response_topic(const QString &channel, MainWindow::ConnectionMode&& mode) const{
    auto response = QString{"channels/"}.append(channel)
            .append("/")
            .append(mode == MainWindow::ConnectionMode::SUBSCRIBE_MODE ? "subscribe/fields/+" + read_key : "publish" + write_key);
    return response;
}

auto MainWindow::convert_connection_state() const -> QString {
    return m_client->state() == 0 ?
                "<span style=\"color: #cc0000;\">Disconnected</span>" :
                m_client->state() == 1 ?
                    "<span style=\"color: #e68a00;\">Connecting</span>" :
                    "<span style=\"color: #00802b;\">Connected</span>";
}

void MainWindow::on_buttonChart_clicked()
{
    if(series_map->isEmpty() || std::all_of(series_map->begin(), series_map->end(), [](auto& l) { return l.isEmpty(); })) {
        auto error_cause = QDateTime::currentDateTime().toString() +
                " No data to show on chart!\n";
        ui->editLog->insertPlainText(error_cause);
        return;
    }

    ui_chart = new chart_view(series_map);
    auto cur_geometry = geometry();
    auto chart_geometry = QRect{cur_geometry.x() +  cur_geometry.width() + 10, cur_geometry.y(), 650, 650};
    ui_chart->setGeometry(chart_geometry);
    ui_chart->show();
    ui->buttonChart->setEnabled(false);

    connect(this, &MainWindow::main_window_closed, ui_chart, [this] {
        if(this->ui_chart != nullptr) {
            this->ui_chart->window()->close();
            this->ui_chart = nullptr;
        }
    });
    connect(m_client, &QMqttClient::messageReceived, ui_chart, &chart_view::chart_repaint);
    connect(this->ui_chart, &chart_view::enable_chart_btn, this, [this] {
        this->ui->buttonChart->setEnabled(true);
    });
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    emit main_window_closed();
    save_settings();
    event->accept();
}

void MainWindow::set_button_enabled(bool enabled) const {
    ui->buttonPublish->setEnabled(enabled);
    ui->buttonSubscribe->setEnabled(enabled);
    ui->buttonChart->setEnabled(enabled);
}

QString MainWindow::extract_field(const std::string &topic) const {
    auto last_slash = topic.find_last_of("/");
    auto topic_substr =topic.substr(0, last_slash);

    auto pre_last_slash = topic_substr.find_last_of("/");
    auto field_name = QString::fromStdString(topic_substr.substr(pre_last_slash+1));

    return field_name;
}

void MainWindow::add_fields_to_map(const QString& field_name, const QString &message) const
{
    auto list_ = series_map->contains(field_name) ? series_map->take(field_name) : QList<double>();
    auto value = strtol(message.toStdString().c_str(), nullptr, 0);

    list_.append(value);
    series_map->insert(field_name, list_);
}

void MainWindow::load_settings() {
    QSettings settings{settings_path, QSettings::NativeFormat};
    mqtt_key = settings.value("keys/mqtt", MQTT_KEY_).toString();
    read_key = settings.value("keys/read", READ_KEY_).toString();
    write_key = settings.value("keys/write", WRITE_KEY_).toString();
    user = settings.value("user", "user").toString();
}

void MainWindow::save_settings() const {
    QSettings settings{settings_path, QSettings::NativeFormat};
    settings.setValue("keys/mqtt", mqtt_key);
    settings.setValue("keys/read", read_key);
    settings.setValue("keys/write", write_key);
    settings.setValue("user", user);
}

void MainWindow::refresh_settings(QString new_mqtt_key, QString new_read_key, QString new_write_key, QString new_user) {
    QSettings settings{settings_path, QSettings::NativeFormat};
    if(!new_mqtt_key.isEmpty()) {
        settings.setValue("keys/mqtt", new_mqtt_key);
    }
    if(!new_read_key.isEmpty()) {
        settings.setValue("keys/read", new_read_key);
    }
    if(!new_write_key.isEmpty()) {
        settings.setValue("keys/write", new_write_key);
    }
    if(!new_user.isEmpty()) {
        settings.setValue("user", new_user);
    }

    load_settings();
}

void MainWindow::on_actionConfigure_triggered()
{
    configurator_modal = new configurator();
    configurator_modal->show();
    connect(configurator_modal, &configurator::save_new_configuration, this, &MainWindow::refresh_settings);
    connect(this, &MainWindow::main_window_closed, configurator_modal, [this] {
        if(this->configurator_modal != nullptr) {
            this->configurator_modal->window()->close();
            this->configurator_modal = nullptr;
        }
    });;
}
