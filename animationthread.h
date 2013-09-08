#ifndef ANIMATIONTHREAD_H
#define ANIMATIONTHREAD_H

#include <QThread>
#include <QFile>
#include <QRegExp>
#include <vector>

class AnimationThread : public QThread
{
    Q_OBJECT
public:
    explicit AnimationThread(QObject *parent = 0);
    void setSourceFile(QFile *sourceFile);
    const std::vector<std::pair<double, double> >& getPrev();
    const std::vector<std::pair<double, double> >& getCur();

signals:
    void next_frame_ready();
    void finish_reached();

private:
    QFile *file;
    bool fileIsSet;
    std::vector<std::pair<double, double> > data[2];
    int prev;
    int cur;

    /* methods */
    void run();
    bool processLine(std::vector<std::pair<double, double> >& dest);
};

#endif // ANIMATIONTHREAD_H
