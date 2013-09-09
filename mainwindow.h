#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QtWidgets>
#include "qt-custom-plot/qcustomplot.h"
#include "animationthread.h"

#define CLICK_TO_SELECT_FILE "Click to select file.."

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void selectAndOpenFile();
    void render_frame();
    void finish_animation();
    void start_animation();

private:
    QCustomPlot *plot;
    QCustomPlot *speedPlot;
    double tMax;
    double speedMax;
    QVector<double> tData;
    QVector<double> speedData;
    QPushButton *fileOpenButton;
    QPushButton *controlButton;
    QFile *logFile;
    bool logFileIsOpened;
    AnimationThread *animator;
    bool animationStarted;
    int graphs_count;
    bool isPaused;

    /* methods */
    bool particle_cross_boundaries(const std::pair<double, double>& prev,
                                   const std::pair<double, double>& cur);
    double sqr(const double& x) { return x * x; }
    void draw_segment(int id, const QColor &color,
                      const double &ax, const double &ay,
                      const double &bx, const double &by);
    void make_rainbow();
    const QColor colorFromAngle(const double &angle);
    void resetSpeedPlot();
};

#endif // MAINWINDOW_H
