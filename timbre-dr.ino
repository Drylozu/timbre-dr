#include <Wire.h>
#include <RTClib.h>
#include <LiquidCrystal_I2C.h>

#define PIN_ALARM 13
#define PIN_LED 8
#define PIN_ALARM_LED 5
#define RING_TIME 8

const DateTime nullTime = DateTime(1);

RTC_DS3231 rtc;
LiquidCrystal_I2C lcd(0x27, 16, 2);

DateTime alarm = nullTime;
byte nextAlarm[2] = { 0, 0 };
byte action = LOW;

String months[12] = { "Ene", "Feb", "Mar", "Abr", "May", "Jun", "Jul", "Ago", "Sep", "Oct", "Nov", "Dic" };
String days[7] = { "Dom", "Lun", "Mar", "Mie", "Jue", "Vie", "Sab" };
byte alarms[16][2] = {
  /// Mañana
  { 6, 30 },
  { 7, 30 },
  { 8, 30 },
  { 9, 30 },
  { 9, 45 },
  { 10, 35 },
  { 11, 25 },
  { 12, 10 },

  /// Tarde
  { 12, 30 },
  { 13, 30 },
  { 14, 30 },
  { 15, 25 },
  { 15, 40 },
  { 16, 30 },
  { 17, 15 },
  { 18, 0 },
};

void setup() {
  pinMode(PIN_ALARM, OUTPUT);
  pinMode(PIN_LED, OUTPUT);

  rtc.begin();
  lcd.init();
  lcd.backlight();
  lcd.clear();

  // Establece hora actual -> solo si está desincronizado
  // rtc.adjust(DateTime(__DATE__, __TIME__));

  delay(1000);
  digitalWrite(PIN_LED, HIGH);
}

void loop() {
  DateTime now = rtc.now();

  // Detiene la alarma si ya pasó el tiempo
  stopRing(now);


  // Día de la semana
  lcd.setCursor(0, 0);
  lcd.print(days[now.dayOfTheWeek()]);

  // Hora
  lcd.setCursor(5, 0);

  if (now.twelveHour() < 10)
    lcd.print('0');
  lcd.print(now.twelveHour());
  lcd.print(':');

  if (now.minute() < 10)
    lcd.print('0');
  lcd.print(now.minute());
  lcd.print(':');

  if (now.second() < 10)
    lcd.print('0');
  lcd.print(now.second());

  lcd.print(' ');
  if (now.hour() >= 12)
    lcd.print("PM");
  else
    lcd.print("AM");

  // Fecha
  lcd.setCursor(0, 1);

  if (now.day() < 10)
    lcd.print('0');
  lcd.print(now.day());
  lcd.print('/');

  lcd.print(months[now.month() - 1]);
  lcd.print('/');

  lcd.print(now.year() % 100);


  // Comprueba alarma
  checkAlarm(now);

  // Siguiente alarma
  lcd.setCursor(11, 1);
  if (nextAlarm[0] == 0 && nextAlarm[1] == 0) {
    lcd.print("  OFF");
  } else {
    if (nextAlarm[0] < 10)
      lcd.print('0');
    lcd.print(nextAlarm[0]);
    lcd.print(':');
    if (nextAlarm[1] < 10)
      lcd.print('0');
    lcd.print(nextAlarm[1]);
  }

  // Anti-lag
  delay(500);
}

void checkAlarm(DateTime time) {
  if (time.dayOfTheWeek() == 0 || time.dayOfTheWeek() == 6) {
    nextAlarm[0] = 0;
    nextAlarm[1] = 0;
    return;
  }

  for (int i = 0; i < 16; i++) {
    if (time.hour() > alarms[i][0]) {
      if (i == 15) {
        nextAlarm[0] = 0;
        nextAlarm[1] = 0;
      }
      continue;  // Si la hora ya pasó, saltar...
    }
    if (time.hour() == alarms[i][0] && time.minute() > alarms[i][1]) {
      if (i == 15) {
        nextAlarm[0] = 0;
        nextAlarm[1] = 0;
      }
      continue;  // Si la hora ya pasó y el minuto ya pasó, saltar...
    }

    nextAlarm[0] = alarms[i][0] % 12;
    if (nextAlarm[0] == 0)
      nextAlarm[0] = 12;
    nextAlarm[1] = alarms[i][1];

    if (time.hour() == alarms[i][0] && time.minute() == alarms[i][1] && time.second() == 0)
      ring(time);  // Ejecutar alarma si es la hora, minuto y segundo cero

    break;
  }
}

void ring(DateTime time) {
  setAlarm(HIGH);
  if (alarm == nullTime)
    alarm = time;
}

void stopRing(DateTime time) {
  if (alarm == nullTime) return;
  if (time.secondstime() - alarm.secondstime() > RING_TIME) {
    setAlarm(LOW);
    alarm = nullTime;
  }
}

void setAlarm(byte value) {
  digitalWrite(PIN_ALARM, value);
  digitalWrite(PIN_ALARM_LED, value);
}