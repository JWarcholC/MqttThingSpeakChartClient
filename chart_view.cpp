#include "chart_view.h"
#include "ui_chart_view.h"
#include <QDebug>
#include <limits>
#include <utility>


chart_view::chart_view(QMap<QString, QList<double>> *series_map, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::chart_view),
    series_map(series_map),
    chart(new QtCharts::QChart),
    x_axis(new QtCharts::QValueAxis),
    y_axis(new QtCharts::QValueAxis)
{
    ui->setupUi(this);
    ui->widgetChart->setLayout(new QVBoxLayout);
    prepare_series();
    prepare_chart();
    prepare_legend();
    prepare_axis();

    view = new QtCharts::QChartView(chart, ui->widgetChart);
    view->setRenderHint(QPainter::Antialiasing);
    view->resize(600, 600);

    ui->widgetChart->layout()->addWidget(view);
}

void chart_view::chart_repaint(){
    auto data_series_map = series_map->toStdMap();
    auto y_min = y_axis->min();
    auto y_max = y_axis->max();

    for (int i = 0; i < series.size(); i++) {
        chart->removeSeries(series[i]);
        auto key = QString{"field"} + QString::number(i+1);
        auto last_value = data_series_map[key].last();

        series[i]->append(samples_counters[i]++, last_value);
        chart->addSeries(series[i]);

        series[i]->attachAxis(x_axis);
        series[i]->attachAxis(y_axis);

        if(last_value > y_max) {
            y_max = last_value;
        }
        if(last_value < y_min) {
            y_min = last_value;
        }
    }

    x_axis->setMax(samples_counters[0]);
    if(y_max > y_axis->max()) {
        y_axis->setMax(y_max + 3);
    }
    if(y_min < y_axis->min()) {
        y_axis->setMin(y_min - 1);
    }
}

std::pair<double, double> chart_view::calculate_chart_limits(std::map<QString, QList<double>> &data_series_map) const {
    auto min_val = std::numeric_limits<double>::min();
    auto max_val = std::numeric_limits<double>::min();

    std::for_each(data_series_map.cbegin(), data_series_map.cend(), [&min_val, &max_val](auto &el) {
        auto cur_min = std::min_element(el.second.cbegin(), el.second.cend());
        auto cur_max = std::max_element(el.second.cbegin(), el.second.cend());

        if(*cur_min < 0 && min_val > *cur_min) {
            min_val = *cur_min;
        }

        if(max_val < *cur_max) {
            max_val = *cur_max;
        }
    });
    return std::make_pair(min_val - 1, max_val + 3);
}

void chart_view::prepare_series()
{
    auto chart_no = series_map->size();
    series = QVector<QtCharts::QLineSeries*>(chart_no);
    samples_counters = QVector<unsigned long>(chart_no);
    samples_counters.fill(1L);

    for (int i = 0; i < series.size(); i++) {
        auto key = QString{"field"} + QString::number(i+1);
        series[i] = new QtCharts::QLineSeries();
        series[i]->setName(key);
        series[i]->append(0, 0);
    }
}

void chart_view::prepare_legend()
{
    chart->legend()->setVisible(true);
    chart->legend()->setBackgroundVisible(true);
    chart->legend()->setBrush(QBrush(QColor(128, 128, 128, 128)));
    chart->legend()->setPen(QPen(QColor(192, 192, 192, 192)));
    chart->legend()->setAlignment(Qt::AlignBottom);
    chart->addAxis(x_axis, Qt::AlignBottom);
    chart->addAxis(y_axis, Qt::AlignLeft);
    chart->legend()->setGeometry(QRectF(85, 80, 140, 14));
    chart->legend()->update();
}

void chart_view::prepare_axis()
{
    x_axis->setTickCount(5);
    y_axis->setTickCount(10);
    for (int i = 0; i < series.size(); i++) {
        chart->addSeries(series[i]);
        series[i]->attachAxis(x_axis);
        series[i]->attachAxis(y_axis);
    }
}

void chart_view::prepare_chart()  {
    auto data_series_map = series_map->toStdMap();

    for(int i = 0; i < series.size(); i++) {
        auto key = QString{"field"} + QString::number(i+1);
        for(auto &el : data_series_map[key]) {
            series[i]->append(samples_counters[i]++, el);
        }
    }

    auto[min_val, max_val] = calculate_chart_limits(data_series_map);
            y_axis->setMin(min_val);
            y_axis->setMax(max_val);
            x_axis->setMax(samples_counters[0]);

            chart->setTitle("MQTT Chart");
}

            void chart_view::closeEvent(QCloseEvent *event) {
        emit enable_chart_btn();
        event->accept();
    }


    chart_view::~chart_view()
    {
        delete x_axis;
        delete y_axis;
        delete chart;
        delete view;
        delete ui;
    }
