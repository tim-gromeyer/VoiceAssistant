#include "weatherdata.h"

#include <QDebug>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

std::string getIconPath(WeatherData::WeatherCode code, bool isDay)
{
    std::string path = "icons/";
    switch (code) {
    case WeatherData::ClearSky:
    case WeatherData::MainlyClear:
        path += isDay ? "animated/clear-day.svg" : "animated/clear-night.svg";
        break;
    case WeatherData::PartlyCloudy:
        path += isDay ? "animated/cloudy-3-day.svg" : "animated/cloudy-3-night.svg";
        break;
    case WeatherData::Overcast:
        path += "animated/cloudy.svg";
        break;
    case WeatherData::Fog:
    case WeatherData::DepositingRimeFog:
        path += isDay ? "animated/fog-day.svg" : "animated/fog-night.svg";
        break;
    case WeatherData::DrizzleLight:
    case WeatherData::DrizzleModerate:
    case WeatherData::DrizzleDense:
    case WeatherData::FreezingDrizzleLight:
    case WeatherData::FreezingDrizzleDense:
        path += isDay ? "bom/app/08_light_rain.svg" : "bom/app/08_light_rain_night.svg";
        break;
    case WeatherData::RainSlight:
    case WeatherData::RainModerate:
    case WeatherData::RainHeavy:
    case WeatherData::FreezingRainLight:
    case WeatherData::FreezingRainHeavy:
        path += isDay ? "bom/app/12_rain.svg" : "bom/app/12_rain_night.svg";
        break;
    case WeatherData::SnowFallSlight:
    case WeatherData::SnowFallModerate:
    case WeatherData::SnowFallHeavy:
    case WeatherData::SnowGrains:
        path += isDay ? "bom/app/15_snow.svg" : "bom/app/15_snow_night.svg";
        break;
    case WeatherData::RainShowersSlight:
    case WeatherData::RainShowersModerate:
    case WeatherData::RainShowersViolent:
        path += isDay ? "bom/app/17_light_showers.svg" : "bom/app/17_light_showers_night.svg";
        break;
    case WeatherData::SnowShowersSlight:
    case WeatherData::SnowShowersHeavy:
        path += isDay ? "bom/app/18_heavy_showers.svg" : "bom/app/18_heavy_showers_night.svg";
        break;
    case WeatherData::ThunderstormSlight:
    case WeatherData::ThunderstormSlightHail:
    case WeatherData::ThunderstormHeavyHail:
        path += isDay ? "animated/thunderstorms.svg" : "animated/scattered-thunderstorms-night.svg";
        break;
    default:
        path += isDay ? "bom/app/00_missing_data.svg" : "bom/app/00_missing_night.svg";
        path += "animated/weather.svg";
        break;
    }
    return path;
}

WeatherData::WeatherData(QObject *parent)
    : QObject(parent)
{}

// Parse JSON data into struct
void WeatherData::parseWeatherData(const QByteArray &jsonData)
{
    hourlyWeatherList.clear();

    QJsonDocument jsonDoc = QJsonDocument::fromJson(jsonData);
    QJsonObject jsonObject = jsonDoc.object();

    // Daily object
    QJsonObject dailyObject = jsonObject[QStringLiteral("daily")].toObject();
    QJsonArray dailyTimesArray = dailyObject[QStringLiteral("time")].toArray();
    QJsonArray dailyTempMin = dailyObject[QStringLiteral("temperature_2m_min")].toArray();
    QJsonArray dailyTempMax = dailyObject[QStringLiteral("temperature_2m_max")].toArray();
    QJsonArray dailySunrise = dailyObject[QStringLiteral("sunrise")].toArray();
    QJsonArray dailySunset = dailyObject[QStringLiteral("sunset")].toArray();
    QJsonArray dailyUVIndex = dailyObject[QStringLiteral("uv_index_max")].toArray();

    for (int i = 0; i < dailyTimesArray.size(); ++i) {
        QString dateString = dailyTimesArray[i].toString();
        double tempMin = dailyTempMin[i].toDouble();
        double tempMax = dailyTempMax[i].toDouble();
        QString sunriseString = dailySunrise[i].toString();
        QString sunsetString = dailySunset[i].toString();
        double uvIndex = dailyUVIndex[i].toDouble();

        QDate date = QDate::fromString(dateString, Qt::ISODate);
        QDateTime sunrise = QDateTime::fromString(sunriseString, Qt::ISODate);
        QDateTime sunset = QDateTime::fromString(sunsetString, Qt::ISODate);

        DailyWeather dailyWeather;
        dailyWeather.day = date;
        dailyWeather.sunrise = sunrise;
        dailyWeather.sunset = sunset;
        dailyWeather.tempMin = tempMin;
        dailyWeather.tempMax = tempMax;
        dailyWeather.uvIndex = uvIndex;

        dailyWeatherList.append(dailyWeather);
    }

    // Hourly object
    QJsonObject hourlyObject = jsonObject[QStringLiteral("hourly")].toObject();
    QJsonArray timeArray = hourlyObject[QStringLiteral("time")].toArray();
    QJsonArray temperatureArray = hourlyObject[QStringLiteral("temperature_2m")].toArray();
    QJsonArray apparentTempArray = hourlyObject[QStringLiteral("apparent_temperature")].toArray();
    QJsonArray windspeedArray = hourlyObject[QStringLiteral("windspeed_10m")].toArray();
    QJsonArray winddirectionArray = hourlyObject[QStringLiteral("winddirection_10m")].toArray();
    QJsonArray weatherCodeArray = hourlyObject[QStringLiteral("weathercode")].toArray();
    QJsonArray relativehumidityArray = hourlyObject[QStringLiteral("relativehumidity_2m")].toArray();
    QJsonArray visibilityArray = hourlyObject[QStringLiteral("visibility")].toArray();
    QJsonArray isDayArray = hourlyObject[QStringLiteral("is_day")].toArray();

    for (int i = 0; i < timeArray.size(); ++i) {
        QString timeString = timeArray[i].toString();
        double temperature = temperatureArray[i].toDouble();
        double apparentTemp = apparentTempArray[i].toDouble();
        double windspeed = windspeedArray[i].toDouble();
        double winddirection = winddirectionArray[i].toDouble();
        int weatherCode = weatherCodeArray[i].toInt();
        int relativehumidity = relativehumidityArray[i].toInt();
        double visibility = visibilityArray[i].toDouble();
        bool isDay = isDayArray[i].toInt();

        QDateTime time = QDateTime::fromString(timeString, Qt::ISODate);

        HourlyWeather hourlyWeather;
        hourlyWeather.time = time;
        hourlyWeather.temperature = temperature;
        hourlyWeather.apparentTemp = apparentTemp;
        hourlyWeather.windspeed = windspeed;
        hourlyWeather.winddirection = winddirection;
        hourlyWeather.weathercode = convertToWeatherCode(weatherCode);
        hourlyWeather.relativehumidity = relativehumidity;
        hourlyWeather.visibility = convertVisibilityToRange(visibility);
        hourlyWeather.is_day = isDay;

        hourlyWeatherList.append(hourlyWeather);

        for (DailyWeather &dailyWeather : dailyWeatherList) {
            if (dailyWeather.day == time.date())
                dailyWeather.hours.append(hourlyWeather);
        }
    }

    // Print the parsed data
    for (const HourlyWeather &weather : qAsConst(hourlyWeatherList)) {
        qDebug() << "Time:" << weather.time.toString(QStringLiteral("yyyy-MM-dd hh:mm"));
        qDebug() << "Temperature:" << weather.temperature;
        qDebug() << "Apparent temperature:" << weather.apparentTemp;
        qDebug() << "Relative humidity:" << weather.relativehumidity;
        qDebug() << "Windspeed:" << weather.windspeed;
        qDebug() << "Winddirection:" << weather.winddirection;
        qDebug() << "Weather Code:" << weather.weathercode;
        qDebug() << "Visibility:" << weather.visibility;
        qDebug() << "Is Day:" << weather.is_day;
        qDebug() << "-----------------------------------";
    }
}

QDebug operator<<(QDebug debug, const WeatherData &data)
{
    debug << "Weather Data:";
    debug.nospace() << "\n  TemperatureUnit: " << data.temperatureUnit;
    debug.nospace() << "\n  SpeedUnit: " << data.speedUnit;

    debug << "\nDaily Weather:";
    for (const auto &dailyWeather : data.dailyWeatherList) {
        debug << "\n  Day: " << dailyWeather.day.toString(QStringView(u"dddd"));
        debug.nospace() << "\n    Sunrise: " << dailyWeather.sunrise.toString(QStringView(u"h:mm"));
        debug.nospace() << "\n    Sunset: " << dailyWeather.sunset.toString(QStringView(u"h:mm"));
        debug.nospace() << "\n    UV Index: " << dailyWeather.uvIndex;
        debug.nospace() << "\n    Min Temperature: " << dailyWeather.tempMin;
        debug.nospace() << "\n    Max Temperature: " << dailyWeather.tempMax;
        debug.nospace() << "\n    Hourly Weather Size: " << dailyWeather.hours.count();

        debug << "\n    Hourly Weather:";
        for (const auto &hourlyWeather : dailyWeather.hours) {
            debug << "\n      Time: " << hourlyWeather.time.toString(QStringView(u"h:mm"));
            debug.nospace() << "\n        Temperature: " << hourlyWeather.temperature;
            debug.nospace() << "\n        Apparent Temperature: " << hourlyWeather.apparentTemp;
            debug.nospace() << "\n        Wind Speed: " << hourlyWeather.windspeed;
            debug.nospace() << "\n        Wind Direction: " << hourlyWeather.winddirection;
            debug.nospace() << "\n        Weather Code: " << hourlyWeather.weathercode;
            debug.nospace() << "\n        Relative Humidity: " << hourlyWeather.relativehumidity;
            debug.nospace() << "\n        Visibility: " << hourlyWeather.visibility;
            debug.nospace() << "\n        Is Day: " << hourlyWeather.is_day;
        }
    }

    return debug.space();
}
