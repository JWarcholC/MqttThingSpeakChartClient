#ifndef CHART_VIEW_H
#define CHART_VIEW_H

#include <QWidget>
#include <QLineSeries>
#include <QChart>
#include <QChartView>
#include <QValueAxis>

namespace Ui {
class chart_view;
}

class chart_view : public QWidget
{
    Q_OBJECT

public:
    explicit chart_view(QMap<QString, QList<double>> *series_map, QWidget *parent = nullptr);
    ~chart_view();

public slots:
    void chart_repaint();

signals:
    void enable_chart_btn();

private:
    Ui::chart_view *ui;
    QMap<QString, QList<double>> *series_map;
    QVector<QtCharts::QLineSeries*> series;
    QVector<unsigned long> samples_counters;
    QtCharts::QChart *chart;
    QtCharts::QChartView *view;
    QtCharts::QValueAxis *x_axis;
    QtCharts::QValueAxis *y_axis;
    std::pair<double, double> calculate_chart_limits(std::map<QString, QList<double>> &data_series_map) const;
    void prepare_chart();
    void prepare_series();
    void prepare_legend();
    void prepare_axis();
    void closeEvent(QCloseEvent *event);

};

#endif // CHART_VIEW_H
