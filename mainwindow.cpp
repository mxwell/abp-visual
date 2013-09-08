#include "mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    QFrame *frame = new QFrame;
    QVBoxLayout *layout = new QVBoxLayout(frame);

    plot = new QCustomPlot;
    plot->setMinimumHeight(400);
    plot->setMinimumWidth(400);
    plot->xAxis->setRange(0, 100);
    plot->yAxis->setRange(0, 100);
    graphs_count = 0;
    layout->addWidget(plot);

    QHBoxLayout *bottomPanel = new QHBoxLayout;
    fileOpenButton = new QPushButton(CLICK_TO_SELECT_FILE);
    bottomPanel->addWidget(fileOpenButton);
    controlButton = new QPushButton("Start");
    bottomPanel->addWidget(controlButton);
    layout->addLayout(bottomPanel);

    animator = new AnimationThread;
    connect(fileOpenButton, SIGNAL(pressed()), this, SLOT(selectAndOpenFile()));
    connect(controlButton, SIGNAL(pressed()), this, SLOT(start_animation()));
    connect(animator, SIGNAL(next_frame_ready()), this, SLOT(render_frame()));
    connect(animator, SIGNAL(finish_reached()), this, SLOT(finish_animation()));

    setCentralWidget(frame);
}

MainWindow::~MainWindow()
{
    if (logFileIsOpened) {
        logFile->close();
        logFileIsOpened = false;
    }
}

void MainWindow::selectAndOpenFile()
{
    QString fileName = QFileDialog::getOpenFileName(this,
                       tr("Open simulatin log"), "", tr("Simulation Log Files (*.log *.dat)"));
    if (fileName.isNull() || fileName.isEmpty())
        return;
    logFile = new QFile(fileName);
    if (!logFile->open(QIODevice::ReadOnly | QIODevice::Text))
        return;
    QString buttonLabel;
    int buttonLabelLen = strlen(CLICK_TO_SELECT_FILE);
    if (fileName.length() <= buttonLabelLen) {
        buttonLabel = fileName;
    } else {
        QStringRef tail(&fileName,
                        fileName.length() - buttonLabelLen,
                        buttonLabelLen);
        buttonLabel = QString("..").append(tail);
    }
    fileOpenButton->setText(buttonLabel);
    logFileIsOpened = true;
    animator->setSourceFile(logFile);
    animationStarted = false;
}

void MainWindow::start_animation()
{
    if (animationStarted) {
        if (isPaused) {
            controlButton->setText("Pause");
            isPaused = false;
            animator->start();
        } else {
            isPaused = true;
            return;
        }
    }
    animationStarted = true;
    controlButton->setText("Pause");
    isPaused = false;
    animator->start();
}

void MainWindow::render_frame()
{
    const std::vector<std::pair<double, double> >& prev =
            animator->getPrev();
    const std::vector<std::pair<double, double> >& cur =
            animator->getCur();
    if (prev.size() != cur.size()) {
        printf("error: number of points changed\n");
        fflush(stdout);
        finish_animation();
        return;
    }
    if (graphs_count == 0) {
        while (graphs_count < (int) prev.size()) {
            plot->addGraph();
            ++graphs_count;
        }
    }
    if (graphs_count != (int) prev.size()) {
        printf("graphs number isn't equal to number of points\n");
        fflush(stdout);
        finish_animation();
        return;
    }
    QVector<double> point_x(1), point_y(1);
    QVector<double> segment_x(2), segment_y(2);
    for (int i = 0; i < graphs_count; ++i) {
        /* direction of segment is reversed here, but,
         * you know... segments don't have directions
         **/
        if (particle_cross_boundaries(prev[i], cur[i])) {
            point_x[0] = cur[i].first;
            point_y[0] = cur[i].second;
            plot->graph(i)->setData(point_x, point_y);
        } else {
            segment_x[0] = prev[i].first;
            segment_y[0] = prev[i].second;
            segment_x[1] = cur[i].first;
            segment_y[1] = cur[i].second;
            plot->graph(i)->setData(segment_x, segment_y);
        }
    }
    plot->replot();
    if (isPaused) {
        controlButton->setText("Resume");
    } else {
        animator->start();
    }
}

void MainWindow::finish_animation()
{
    if (!animationStarted)
        return;
    animationStarted = false;
    logFile->seek(0);
    controlButton->setText("Start");
}

bool MainWindow::particle_cross_boundaries(const std::pair<double, double>& prev,
                               const std::pair<double, double>& cur)
{
    return sqr(prev.first - cur.first) + sqr(prev.second - cur.second) > sqr(50);
}
