#include "utils.h"

#include <QCoreApplication>
#include <QDebug>
#include <QDir>
#include <QHash>
#include <QTextStream>

#include <sstream>
#include <string>

namespace utils {
inline namespace numbers {
int wordToNumber(QString text)
{
    static QHash<QString, int> numMap
        = {{QCoreApplication::translate("number", "zero"), 0},
           {QCoreApplication::translate("number", "one"), 1},
           {QCoreApplication::translate("number", "two"), 2},
           {QCoreApplication::translate("number", "three"), 3},
           {QCoreApplication::translate("number", "four"), 4},
           {QCoreApplication::translate("number", "five"), 5},
           {QCoreApplication::translate("number", "six"), 6},
           {QCoreApplication::translate("number", "seven"), 7},
           {QCoreApplication::translate("number", "eight"), 8},
           {QCoreApplication::translate("number", "nine"), 9},
           {QCoreApplication::translate("number", "ten"), 10},
           {QCoreApplication::translate("number", "eleven"), 11},
           {QCoreApplication::translate("number", "twelve"), 12},
           {QCoreApplication::translate("number", "thirteen"), 13},
           {QCoreApplication::translate("number", "fourteen"), 14},
           {QCoreApplication::translate("number", "fifteen"), 15},
           {QCoreApplication::translate("number", "sixteen"), 16},
           {QCoreApplication::translate("number", "seventeen"), 17},
           {QCoreApplication::translate("number", "eighteen"), 18},
           {QCoreApplication::translate("number", "nineteen"), 19},
           {QCoreApplication::translate("number", "twenty"), 20},
           {QCoreApplication::translate("number", "thirty"), 30},
           {QCoreApplication::translate("number", "forty"), 40},
           {QCoreApplication::translate("number", "fifty"), 50},
           {QCoreApplication::translate("number", "sixty"), 60},
           {QCoreApplication::translate("number", "seventy"), 70},
           {QCoreApplication::translate("number", "eighty"), 80},
           {QCoreApplication::translate("number", "ninety"), 90},
           {QCoreApplication::translate("number", "hundred"), 100},
           {QCoreApplication::translate("number", "thousand"), 1000},
           {QCoreApplication::translate("number", "million"), 1000000},
           {QCoreApplication::translate("number", "billion"), 1000000000}};

    int result = 0;
    int currNum = 0;
    int prevNum = 0;
    bool isNegative = false;

    text = text.toLower();
    for (const QString &word : text.split(u' ')) {
        if (word == QCoreApplication::translate("number", "negative")) {
            isNegative = true;
        } else if (word == QCoreApplication::translate("number", "and")) {
            // ignore "and"
        } else if (numMap.contains(word)) {
            if (numMap[word] >= 100 && currNum > 0) {
                prevNum += currNum * numMap[word];
                currNum = 0;
            } else {
                currNum += numMap[word];
            }
        } else if (word == QCoreApplication::translate("number", "hundred")) {
            currNum *= 100;
        } else if (word == QCoreApplication::translate("number", "thousand")
                   || word == QCoreApplication::translate("number", "million")
                   || word == QCoreApplication::translate("number", "billion")) {
            result += (prevNum + currNum) * numMap[word];
            prevNum = 0;
            currNum = 0;
        } else {
            return -1; // unrecognized word/number
        }
    }

    result += prevNum + currNum;
    return isNegative ? -result : result;
}

//// Maybe reading from a JSON file is a better approach
//static QHash<QString, int> numbers;
//if (numbers.isEmpty()) {
//    numbers[QCoreApplication::translate("number", "zero")] = 0;
//    numbers[QCoreApplication::translate("number", "one")] = 1;
//    numbers[QCoreApplication::translate("number", "two")] = 2;
//    numbers[QCoreApplication::translate("number", "three")] = 3;
//    numbers[QCoreApplication::translate("number", "four")] = 4;
//    numbers[QCoreApplication::translate("number", "five")] = 5;
//    numbers[QCoreApplication::translate("number", "six")] = 6;
//    numbers[QCoreApplication::translate("number", "seven")] = 7;
//    numbers[QCoreApplication::translate("number", "eight")] = 8;
//    numbers[QCoreApplication::translate("number", "nine")] = 9;
//    numbers[QCoreApplication::translate("number", "ten")] = 10;
//}

//return numbers.value(word, 10);
} // namespace numbers
} // namespace utils

namespace directory {
bool copyRecursively(const QString &fromDir, const QString &toDir, bool coverFileIfExist)
{
    QDir sourceDir(fromDir);
    QDir targetDir(toDir);
    if (!targetDir.exists()) { /* if directory don't exists, build it */
        if (!targetDir.mkdir(targetDir.absolutePath()))
            return false;
    }

    QFileInfoList fileInfoList = sourceDir.entryInfoList();
    foreach (QFileInfo fileInfo, fileInfoList) {
        if (fileInfo.fileName() == "." || fileInfo.fileName() == "..")
            continue;

        if (fileInfo.isDir()) { /* if it is directory, copy recursively*/
            if (!copyRecursively(fileInfo.filePath(),
                                 targetDir.filePath(fileInfo.fileName()),
                                 coverFileIfExist))
                return false;
        } else { /* if coverFileIfExist == true, remove old file first */
            if (coverFileIfExist && targetDir.exists(fileInfo.fileName())) {
                targetDir.remove(fileInfo.fileName());
            }

            // files copy
            if (!QFile::copy(fileInfo.filePath(), targetDir.filePath(fileInfo.fileName()))) {
                return false;
            }
        }
    }
    return true;
}
} // namespace directory

namespace file {
QString makeSizeRedalbe(qint64 size)
{
    QString sizeString;
    if (size < 1024) {
        sizeString = QString::number(size) + " B";
    } else if (size < 1048576) {
        sizeString = QString::number(size / 1024.0, 'f', 2) + " KB";
    } else {
        sizeString = QString::number(size / 1048576.0, 'f', 2) + " MB";
    }

    return sizeString;
}
} // namespace file

namespace strings {
int normalizeText(const QString &text)
{
    return utils::wordToNumber(text);
}
} // namespace strings

namespace download {
QString makeSecoundsReadable(qint64 secondsRemaining)
{
    QString timeString;

    // Convert seconds remaining to a more readable format
    if (secondsRemaining < 60) {
        timeString = QCoreApplication::translate("time", "%1 seconds")
                         .arg(QString::number(secondsRemaining));
    } else if (secondsRemaining < 3600) {
        qint64 minutes = secondsRemaining / 60;
        qint64 seconds = secondsRemaining % 60;
        timeString = QCoreApplication::translate("time", "%1 minute(s), %2 seconds")
                         .arg(QString::number(minutes), QString::number(seconds));
    } else if (secondsRemaining < 86400) {
        qint64 hours = secondsRemaining / 3600;
        qint64 minutes = (secondsRemaining % 3600) / 60;
        timeString = QCoreApplication::translate("time", "%1 hour(s), %2 minute(s)")
                         .arg(QString::number(hours), QString::number(minutes));
    } else if (secondsRemaining < 31536000) {
        qint64 days = secondsRemaining / 86400;
        qint64 hours = (secondsRemaining % 86400) / 3600;
        timeString = QCoreApplication::translate("time", "%1 day(s), %2 hour(s)")
                         .arg(QString::number(days), QString::number(hours));
    } else {
        qint64 years = secondsRemaining / 31536000;
        qint64 days = (secondsRemaining % 31536000) / 86400;
        timeString = QCoreApplication::translate("time", "%1 year(s), %2 day(s)")
                         .arg(QString::number(years), QString::number(days));
    }

    return timeString;
}
QString makeDownloadSpeedReadable(qint64 downloadSpeed)
{
    return file::makeSizeRedalbe(downloadSpeed) + STR("/s");
}
} // namespace download
