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
};

#endif // MAINWINDOW_H
