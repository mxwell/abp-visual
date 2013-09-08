#include "animationthread.h"

AnimationThread::AnimationThread(QObject *parent) :
    QThread(parent)
{
    fileIsSet = false;
    prev = -1;
    cur = -1;
}

void AnimationThread::setSourceFile(QFile *sourceFile)
{
    file = sourceFile;
    fileIsSet = true;
    prev = -1;
    cur = -1;
}

bool AnimationThread::processLine(std::vector<std::pair<double, double> > &dest)
{
    if (file->atEnd())
        return false;
    QString line = file->readLine();
    dest.clear();
    int pos = 0;
    double prev = -1;
    while (pos < line.size()) {
        bool ok;
        int space = line.indexOf(QRegExp("\\s"), pos);
        QStringRef mid = line.midRef(pos, space - pos);
        double val = mid.toDouble(&ok);
        if (!ok) {
            printf("can't recognize number\n");
            fflush(stdout);
            return false;
        }
        pos = space + 1;
        while (pos < line.size() && line[pos].isSpace())
            ++pos;
        if (!ok) {
            printf("wrong value in file\n");
            fflush(stdout);
            val = 0.0;
        }
        if (prev > -0.5) {
            /* found point (prev, val) */
            dest.push_back(std::make_pair(prev, val));
            prev = -1;
        } else {
            prev = val;
        }
    }
    return true;
}

void AnimationThread::run()
{
    if (!fileIsSet)
        return;
    if (prev == -1) {
        prev = 0;
        cur = 1;
        if (!processLine(data[prev])) {
            finish_reached();
            return;
        }
    } else {
        std::swap(prev, cur);
    }
    if (!processLine(data[cur])) {
        finish_reached();
        return;
    }
    msleep(20);
    next_frame_ready();
}

const std::vector<std::pair<double, double> >&
    AnimationThread::getPrev()
{
    return data[prev];
}

const std::vector<std::pair<double, double> >&
    AnimationThread::getCur()
{
    return data[cur];
}
