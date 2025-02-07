#include <stDHT.h>              // Подключаем библиотеку для работы с датчиком влажности DHT11
#include <Wire.h>               // Подключаем библиотеку для работы с шиной I2C
#include <LiquidCrystal_I2C.h>  // Подключаем библиотеку для работы с LCD дисплеем по шине I2C
#include <iarduino_RTC.h>       // Подключаем библиотеку iarduino_RTC для работы с модулями реального времени.

iarduino_RTC watch(RTC_DS1307);     // Указываем модель RTC модуля        
LiquidCrystal_I2C lcd(0x27,16,2);   // Объявляем  объект библиотеки, указывая параметры дисплея (адрес I2C = 0x27, количество столбцов = 16, количество строк = 2)
                                    // Если надпись не появилась, замените адрес 0x27 на 0x3F
DHT sens(DHT11);

// Константы, которые можно менять
//! Время сеанса, настраивается в мс
// 9 часов = 32400000 мс, 8 часов = 28800000 мс, 7 часов = 25200000 мс, 6 часов = 21600000 мс, 5 часов = 18000000 мс
unsigned long last_time = 21600000;
//! Границы влажности, настраивается в %
int min_hum = 40; // минимальная допустимая влажность
int max_hum = 50; // максимальная допустимая влажность
//====================================================

// Далее идут номера пинов куда идёт подключение разных датчиков(можно изменить),
// а также разных параметров, которые можно изменить
#define button_pin 9        // пин кнопки
#define relay_pin 6         // пин  2-х реле
#define light_pin_red 3     // пин красного светодиода
#define light_pin_green 4   // пин зеленого светодиода
#define light_pin_blue 5    // пин голубого светодиода
#define datchik_pin 7       // пин датчика
#define sound_pin 8         // пин пищалки 
//====================================================

uint32_t tim, end_tim;         
boolean butt_flag = 0;      // флажок нажатия кнопки
boolean butt;               // переменная, хранящая состояние кнопки
boolean flag = 0;           // флажок режима
unsigned long last_press;   // таймер для фильтра дребезга
boolean condition = 0;      
boolean final_end = 0;
unsigned long s_last_time = last_time / 1000; // время до отключения(увеличивается, если ставить на паузу) (с)
unsigned long end_time;
unsigned long start_time;
String mr_wolf = "00:00:00 , 0";

void setup() 
{
  Serial.begin(9600);
  lcd.init();           // Инициируем работу с LCD дисплеем
  lcd.backlight();      // Включаем подсветку LCD дисплея
  watch.begin();        // Инициируем работу с модулем.

  pinMode(datchik_pin, INPUT);
  digitalWrite(datchik_pin, HIGH);
  pinMode(light_pin_red, OUTPUT);       // red
  pinMode(light_pin_green, OUTPUT);     // green
  pinMode(light_pin_blue, OUTPUT);      // blue
  pinMode(button_pin, INPUT_PULLUP);    // кнопка подтянута внутренним резистором (урок 5)
  pinMode(relay_pin, OUTPUT);           // пин реле как выход
  digitalWrite(relay_pin, LOW);
  delay(1000);
}

void loop() 
{
  relay_and_sound();    // проверка реле
  if(final_end == 0){   // если ещё не конец, то выводим время
    real_time(); 
  }
  
  // Далее блок паузы
  butt = !digitalRead(button_pin);  // считать текущее положение кнопки
  if (butt == 1 && butt_flag == 0 && millis() - last_press > 100 && final_end == 0) {  // если кнопка НАЖАТА, до этого была была ОТПУЩЕНА
    if (flag == 0){
      start_time = millis();
      digitalWrite(relay_pin, HIGH);
      
    } else {
      end_time = millis();
      digitalWrite(relay_pin, LOW);
    }
    butt_flag = 1;          // запоминаем, что нажимали кнопку
    flag = !flag;           // инвертируем флажок
    last_press = millis();  // запоминаем время
    digitalWrite(light_pin_green, LOW);
    digitalWrite(light_pin_blue, HIGH);
    digitalWrite(light_pin_red, LOW);
    lcd.setCursor(0, 1);
    lcd.print("     PAUSE      ");
    condition = flag;
  }
  if (butt == 0 && butt_flag == 1 && final_end == 0) {  // если кнопка ОТПУЩЕНА, и до этого была НАЖАТА
    butt_flag = 0;                    // запоминаем, что отпустили кнопку
  } // Конец блока паузы
  
  // Проверка, что паузы нет и вывод всего
  if (condition == 0 && final_end == 0){
    humidity();
    deadlain_time();
  }


  // Увеличение времени отключения после паузы
  if (butt == 0 && butt_flag == 0 && flag == 0){
    last_time = last_time + end_time - start_time;
    end_time = 0;
    start_time = 0;
  }
  
}

// функция измерения влажности и температуры (и отображение)
void humidity(){
  int t = sens.readTemperature(datchik_pin); // чтение датчика на пине datchik_pin
  int h = sens.readHumidity(datchik_pin);    // чтение датчика на пине datchik_pin
  if (h <= max_hum or h >= min_hum){ // изменение цвета светодиода в зависимости от "хорошей" влажности
      digitalWrite(light_pin_green, HIGH);
      digitalWrite(light_pin_blue, LOW);
      digitalWrite(light_pin_red, LOW);
  } else {
      digitalWrite(light_pin_red, HIGH);
      digitalWrite(light_pin_green, LOW);
      digitalWrite(light_pin_blue, LOW);
  }
  if(end_tim > 1 && final_end == 0){
  lcd.setCursor(9, 1);
  lcd.print(h);
  lcd.print("% ");
  lcd.print(t);
  lcd.println("C");
  } 
}

// функция отключения питания
void relay_and_sound(){
  if((millis() - last_time >= 0) && (millis() - last_time <= 100) && condition == 0){
    digitalWrite(relay_pin, HIGH);
    tone(sound_pin, 200);
    delay(200);
    noTone(sound_pin);
  }
}

// функция вывода на дисплей сколько осталось до конца
void deadlain_time(){
    tim   =  millis();  // сколько прошло с момента включения (мс)
    end_tim = (last_time - tim)/1000; // сколько осталось (с)
    lcd.setCursor(0, 1);  //  Устанавливаем курсор в позицию (0 столбец, 1 строка).
    if (s_last_time >= end_tim && end_tim >= 7200){               // если осталось от 7 до 2 часов    
        lcd.print(end_tim / 3600);
        lcd.print(" hours  ");                                      
    } else if (end_tim < 7200 && end_tim >= 3600) {               // если остался 1 час
        lcd.print(end_tim / 3600);
        lcd.print(" hour   ");                                  
    } else if (end_tim < 3600 && end_tim >= 60) {                 // если остался меньше часа
        lcd.print(end_tim / 60);
        lcd.print(" min   ");                                  
    } else if (end_tim < 60 && end_tim > 0) {                     // если остался меньше минуты
        lcd.print(end_tim);
        lcd.print(" sec   ");                                     // финальный вывод завершения работы  
    } else if (end_tim <= 0) {
        lcd.setCursor(0, 0);
        final_end = 1;
        lcd.print("  END OF WORK  ");
        lcd.setCursor(0, 1);
        lcd.print("  PRESS <Reset> ");
        
    }
}

// функция вывода реального времени
void real_time(){
  mr_wolf = watch.gettime("H:i:s , D");
  lcd.setCursor(1, 0);
  lcd.print(mr_wolf);
}