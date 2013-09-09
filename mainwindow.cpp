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

    speedPlot = new QCustomPlot;
    speedPlot->setMinimumHeight(100);
    speedPlot->setMaximumHeight(150);
    resetSpeedPlot();
    speedPlot->addGraph()->setPen(QPen(Qt::red));
    layout->addWidget(speedPlot);

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
    /* make_rainbow(); */
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
    QPen pen;
    pen.setWidth(1);
    double speed_x = 0;
    double speed_y = 0;
    double speed_count = 0;
    for (int i = 0; i < graphs_count; ++i) {
        /* direction of segment is reversed here, but,
         * you know... segments don't have directions
         **/
        if (particle_cross_boundaries(prev[i], cur[i])) {
            point_x[0] = cur[i].first;
            point_y[0] = cur[i].second;
            pen.setColor(Qt::black);
            plot->graph(i)->setPen(pen);
            plot->graph(i)->setData(point_x, point_y);
        } else {
            segment_x[0] = prev[i].first;
            segment_y[0] = prev[i].second;
            segment_x[1] = cur[i].first;
            segment_y[1] = cur[i].second;
            double dx = cur[i].first - prev[i].first;
            double dy = cur[i].second - prev[i].second;
            speed_x += dx;
            speed_y += dy;
            ++speed_count;
            double angle = atan2(dy, dx);
            pen.setColor(colorFromAngle(angle));
            plot->graph(i)->setPen(pen);
            plot->graph(i)->setData(segment_x, segment_y);
        }
    }
    plot->replot();
    double tValue = tData.size();
    tData.append(tValue);
    if (tValue > 0.75 * tMax) {
        tMax = tValue / 0.25;
        speedPlot->xAxis->setRange(0, tMax);
    }
    speed_x /= speed_count;
    speed_y /= speed_count;
    double speedValue = sqrt(sqr(speed_x) + sqr(speed_y));
    speedData.append(speedValue);
    if (speedValue > 0.75 * speedMax) {
        speedMax = speedValue / 0.75;
        speedPlot->yAxis->setRange(0, speedMax);
    }
    speedPlot->graph(0)->setData(tData, speedData);
    speedPlot->replot();
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
    resetSpeedPlot();
    logFile->seek(0);
    controlButton->setText("Start");
}

bool MainWindow::particle_cross_boundaries(const std::pair<double, double>& prev,
                               const std::pair<double, double>& cur)
{
    return sqr(prev.first - cur.first) + sqr(prev.second - cur.second) > sqr(50);
}

void MainWindow::draw_segment(int id, const QColor &color,
                              const double &ax, const double &ay,
                              const double &bx, const double &by)
{
    QVector<double> x, y;
    x.append(ax + 50);
    y.append(ay + 50);
    x.append(bx + 50);
    y.append(by + 50);
    while (graphs_count <= id) {
        plot->addGraph();
        ++graphs_count;
    }
    plot->graph(id)->setPen(QPen(color));
    plot->graph(id)->setData(x, y);
}

void MainWindow::make_rainbow()
{
    int n = 1000;
    for (int i = 0; i < n; ++i) {
        QColor color = QColor::fromHsv(i * 256 / n, 255, 160, 255);
        double angle = i * 2 * M_PI / n;
        draw_segment(i, color, 5 * cos(angle), 5 * sin(angle), 10 * cos(angle), 10 * sin(angle));
    }
}

/* angle in about [-PI, PI), see atan2 */
const QColor MainWindow::colorFromAngle(const double &angle)
{
    int hue = (int) ((angle * 128 / M_PI) + 128) & 255;
    return QColor::fromHsv(hue, 255, 220, 255);
}

void MainWindow::resetSpeedPlot()
{
    tData.clear();
    speedData.clear();
    tMax = 200;
    speedMax = 1.0;
    speedPlot->xAxis->setRange(0, tMax);
    speedPlot->yAxis->setRange(0, speedMax);
}
