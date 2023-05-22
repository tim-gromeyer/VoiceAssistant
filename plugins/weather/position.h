#include <QObject>

class QGeoPositionInfoSource;
class QGeoPositionInfo;

class Position : public QObject
{
    Q_OBJECT

public:
    explicit Position(QObject *parent = nullptr);

    void getUserPosition(double *latitude, double *longitude) const;
    static void getCityPosition(QStringView city, double *latitude, double *longitude);

private Q_SLOTS:
    void positionUpdated(const QGeoPositionInfo &);

private:
    QGeoPositionInfoSource *source;

    double m_latitude = 0;
    double m_longitude = 0;
};
