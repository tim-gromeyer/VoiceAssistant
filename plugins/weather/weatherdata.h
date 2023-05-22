#include <QDateTime>
#include <QObject>

class WeatherData : QObject
{
    Q_OBJECT

    friend QDebug operator<<(QDebug debug, const WeatherData &data);

public:
    explicit WeatherData(QObject *parent = nullptr);

    enum TemperatureUnit { Celsius = 0, Fahrenheit = 1 };
    enum SpeedUnit { KilometersPerHour = 0, MilesPerHour = 1 };

    enum WeatherCode {
        ClearSky = 0,
        MainlyClear = 1,
        PartlyCloudy = 2,
        Overcast = 3,
        Fog = 45,
        DepositingRimeFog = 48,
        DrizzleLight = 51,
        DrizzleModerate = 53,
        DrizzleDense = 55,
        FreezingDrizzleLight = 56,
        FreezingDrizzleDense = 57,
        RainSlight = 61,
        RainModerate = 63,
        RainHeavy = 65,
        FreezingRainLight = 66,
        FreezingRainHeavy = 67,
        SnowFallSlight = 71,
        SnowFallModerate = 73,
        SnowFallHeavy = 75,
        SnowGrains = 77,
        RainShowersSlight = 80,
        RainShowersModerate = 81,
        RainShowersViolent = 82,
        SnowShowersSlight = 85,
        SnowShowersHeavy = 86,
        ThunderstormSlight = 95,
        ThunderstormSlightHail = 96,
        ThunderstormHeavyHail = 99
    };

    enum WindSpeedCategory {
        Calm = 0,
        LightBreeze = 1,
        ModerateBreeze = 2,
        StrongBreeze = 3,
        NearGale = 4,
        Gale = 5,
        StrongGale = 6,
        Storm = 7,
        ViolentStorm = 8,
        Hurricane = 9
    };

    enum WindDirection {
        North = 0,
        NorthNortheast = 1,
        Northeast = 2,
        EastNortheast = 3,
        East = 4,
        EastSoutheast = 5,
        Southeast = 6,
        SouthSoutheast = 7,
        South = 8,
        SouthSouthwest = 9,
        Southwest = 10,
        WestSouthwest = 11,
        West = 12,
        WestNorthwest = 13,
        Northwest = 14,
        NorthNorthwest = 15
    };

    enum PrecipitationType { NoPrecipitation = 0, Rain = 1, Snow = 2, Sleet = 3, Hail = 4 };

    enum VisibilityRange {
        ExcellentVisibility = 0,
        GoodVisibility = 1,
        ModerateVisibility = 2,
        PoorVisibility = 3,
        VeryPoorVisibility = 4
    };

    enum UVIndexCategory {
        LowUVIndex = 0,
        ModerateUVIndex = 1,
        HighUVIndex = 2,
        VeryHighUVIndex = 3,
        ExtremeUVIndex = 4
    };

    [[nodiscard]] static WeatherCode convertToWeatherCode(int code)
    {
        switch (code) {
        case 0:
            return ClearSky;
        case 1:
            return MainlyClear;
        case 2:
            return PartlyCloudy;
        case 3:
            return Overcast;
        case 45:
            return Fog;
        case 48:
            return DepositingRimeFog;
        case 51:
            return DrizzleLight;
        case 53:
            return DrizzleModerate;
        case 55:
            return DrizzleDense;
        case 56:
            return FreezingDrizzleLight;
        case 57:
            return FreezingDrizzleDense;
        case 61:
            return RainSlight;
        case 63:
            return RainModerate;
        case 65:
            return RainHeavy;
        case 66:
            return FreezingRainLight;
        case 67:
            return FreezingRainHeavy;
        case 71:
            return SnowFallSlight;
        case 73:
            return SnowFallModerate;
        case 75:
            return SnowFallHeavy;
        case 77:
            return SnowGrains;
        case 80:
            return RainShowersSlight;
        case 81:
            return RainShowersModerate;
        case 82:
            return RainShowersViolent;
        case 85:
            return SnowShowersSlight;
        case 86:
            return SnowShowersHeavy;
        case 95:
            return ThunderstormSlight;
        case 96:
            return ThunderstormSlightHail;
        case 99:
            return ThunderstormHeavyHail;
        default:
            return ClearSky;
        }
    }

    [[nodiscard]] static WindSpeedCategory getWindSpeedCategory(double windSpeed, SpeedUnit unit)
    {
        // Convert the wind speed to a common unit (e.g., Kph)
        if (unit == MilesPerHour)
            windSpeed = windSpeed * 1.60934; // Convert mph to Kph

        // Determine the wind speed category based on the converted value
        if (windSpeed < 1.0) {
            return Calm;
        } else if (windSpeed < 6.0) {
            return LightBreeze;
        } else if (windSpeed < 12.0) {
            return ModerateBreeze;
        } else if (windSpeed < 20.0) {
            return StrongBreeze;
        } else if (windSpeed < 29.0) {
            return NearGale;
        } else if (windSpeed < 39.0) {
            return Gale;
        } else if (windSpeed < 50.0) {
            return StrongGale;
        } else if (windSpeed < 62.0) {
            return Storm;
        } else if (windSpeed < 75.0) {
            return ViolentStorm;
        } else {
            return Hurricane;
        }
    }

    [[nodiscard]] static WindDirection getWindDirection(int direction)
    {
        std::vector<WindDirection> directions = {North,
                                                 NorthNortheast,
                                                 Northeast,
                                                 EastNortheast,
                                                 East,
                                                 EastSoutheast,
                                                 Southeast,
                                                 SouthSoutheast,
                                                 South,
                                                 SouthSouthwest,
                                                 Southwest,
                                                 WestSouthwest,
                                                 West,
                                                 WestNorthwest,
                                                 Northwest,
                                                 NorthNorthwest};

        int index = int(direction / 22.5);
        return directions[index];
    }

    [[nodiscard]] static VisibilityRange convertVisibilityToRange(double visibilityMeters)
    {
        if (visibilityMeters >= 10000)
            return ExcellentVisibility;
        else if (visibilityMeters >= 5000)
            return GoodVisibility;
        else if (visibilityMeters >= 1000)
            return ModerateVisibility;
        else if (visibilityMeters >= 500)
            return PoorVisibility;
        else
            return VeryPoorVisibility;
    }

    [[nodiscard]] static UVIndexCategory convertUVIndexToCategory(double uvIndex)
    {
        if (uvIndex <= 2.9)
            return LowUVIndex;
        else if (uvIndex <= 5.9)
            return ModerateUVIndex;
        else if (uvIndex <= 7.9)
            return HighUVIndex;
        else if (uvIndex <= 10.9)
            return VeryHighUVIndex;
        else
            return ExtremeUVIndex;
    }

    void parseWeatherData(const QByteArray &jsonData);

private:
    TemperatureUnit temperatureUnit = Celsius;
    SpeedUnit speedUnit = KilometersPerHour;

    struct HourlyWeather
    {
        QDateTime time;
        double temperature;
        double apparentTemp;
        double windspeed;
        double winddirection;
        WeatherCode weathercode;
        int relativehumidity;
        VisibilityRange visibility;
        bool is_day;
    };

    struct DailyWeather
    {
        QDate day;
        QList<HourlyWeather> hours;
        QDateTime sunrise;
        QDateTime sunset;
        double uvIndex = 0;
        double tempMin = 0;
        double tempMax = 0;
    };

    QList<DailyWeather> dailyWeatherList;

    QList<HourlyWeather> hourlyWeatherList;
};
