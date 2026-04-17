# RM3100-Arduino

Arduino-библиотека для магнитометров семейства RM3100/RM2100 с поддержкой SPI и I2C.

Библиотека оформлена в стандартной структуре Arduino:

- `src` содержит исходники библиотеки.
- `examples` содержит готовые примеры для Arduino IDE и `arduino-cli`.
- `library.properties` описывает библиотеку для Arduino Library Manager.
- `keywords.txt` добавляет подсветку ключевых слов в Arduino IDE 1.x.
- `LICENSE` содержит оригинальный текст лицензии MIT.

## Возможности

- Работа через `SPIClass` и `TwoWire` без принудительного вызова `SPI.begin()` или `Wire.begin()` внутри библиотеки.
- Доступ к регистрам RM3100.
- Одиночные и непрерывные измерения.
- Опрос `DRDY` через вывод или по биту `STATUS`.
- Встроенный self-test (`BIST`).
- Целочисленная калибровка в picoTesla без `float`.
- Разделение реализации на core/math/transport-модули.

## Установка

1. Скопируйте папку репозитория в каталог библиотек Arduino.
2. Перезапустите Arduino IDE, если она была открыта.
3. Откройте один из примеров из меню `Файл -> Примеры -> RM3100-Arduino`.

## Быстрый старт

Для SPI:

```cpp
#include <SPI.h>
#include <RM3100.h>

void setup() {
    SPI.begin();
    RM3100.beginSPI(10, 9);
    RM3100.setCycleCountAll(RM3100Class::kDefaultCycleCount);
    RM3100.setTMRC(RM3100Class::kTmrc75Hz);
    RM3100.startContinuous(RM3100Class::kAxisAll, true);
}
```

Для I2C:

```cpp
#include <Wire.h>
#include <RM3100.h>

void setup() {
    Wire.begin();
    RM3100.beginI2C(RM3100Class::kI2CAddress00);
    RM3100.setCycleCountAll(RM3100Class::kDefaultCycleCount);
    RM3100.setTMRC(RM3100Class::kTmrc75Hz);
    RM3100.startContinuous(RM3100Class::kAxisAll, true);
}
```

## Примеры

- `BasicSPI` показывает минимальное подключение по SPI.
- `BasicI2C` показывает минимальное подключение по I2C.
- `FullFeaturedDemo` демонстрирует инициализацию транспорта, настройку регистра `HSHAKE`, self-test, single-shot, continuous mode и работу с типом `RM3100MagField3`.

## Замечания по API

- Все публичные константы вынесены в `RM3100Class::...`, чтобы не засорять глобальное пространство имён.
- Тип `RM3100MagField3` использует префикс библиотеки и не конфликтует с пользовательским кодом.
- `RM3100Class::BusType` и `RM3100Class::Axis` оформлены как `enum class`.
- Библиотека не вызывает `SPI.begin()` и `Wire.begin()`: запуск шин делает пользователь в `setup()`.
- Поля `picoTesla` и `magnitudePicoTesla` будут равны нулю, пока пользователь не загрузит реальные коэффициенты калибровки через `setAxisCalibration()` или `setCalibration()`.

## Документация по микросхеме

В репозитории сохранён пользовательский мануал:

- [RM3100-RM2100-Magneto-Inductive-Magnetometer-User-Manual-V14.0.md](RM3100-RM2100-Magneto-Inductive-Magnetometer-User-Manual-V14.0.md)

## Проверка сборки

Для локальной проверки можно использовать встроенный `arduino-cli` из Arduino IDE 2.x:

```powershell
& "C:\Program Files\Arduino IDE\resources\app\lib\backend\resources\arduino-cli.exe" compile --fqbn arduino:avr:uno --warnings all --library . examples/BasicSPI
```

Аналогично проверяются `examples/BasicI2C` и `examples/FullFeaturedDemo`.