#include <Arduino.h>
#include <Adafruit_NeoPixel.h>

#define UART_INPUT_BUFFER_LENGTH 100
#define LED_PIN1 2         // Пин для первого светодиода
#define LED_PIN2 4         // Пин для второго светодиода
#define LED_STRIP_PIN 6    // Пин для ленты WS2812
#define LED_COUNT 10       // Количество светодиодов в ленте

char uartInputBuffer[UART_INPUT_BUFFER_LENGTH];
uint8_t uartInputBufferIndex = 0;
char lastFindedFragment[UART_INPUT_BUFFER_LENGTH - 3];
unsigned long lastFragmentTime = 0; // Время последнего обновления lastFindedFragment
const unsigned long timeout = 2000; // Таймаут в миллисекундах

Adafruit_NeoPixel strip(LED_COUNT, LED_STRIP_PIN, NEO_GRB + NEO_KHZ800);

void setup() {
    Serial.begin(9600); // Настройка UART
    pinMode(LED_PIN1, OUTPUT); // Настраиваем пин для первого светодиода
    pinMode(LED_PIN2, OUTPUT); // Настраиваем пин для второго светодиода
    digitalWrite(LED_PIN1, LOW); // Выключаем первый светодиод
    digitalWrite(LED_PIN2, LOW); // Выключаем второй светодиод

    strip.begin();      // Инициализация ленты
    strip.show();       // Убедимся, что все светодиоды выключены
    strip.clear();      // Очистим все пиксели
}

void loop() {
    while (Serial.available() > 0) {
        char receivedChar = Serial.read();

        // Если буфер близок к переполнению, сбросить счётчик
        if (uartInputBufferIndex >= UART_INPUT_BUFFER_LENGTH - 1) {
            uartInputBufferIndex = 0;
        }

        // Добавляем символ в буфер, если это не конец строки
        if (receivedChar != '\n') {
            uartInputBuffer[uartInputBufferIndex++] = receivedChar;
        } else {
            // Завершаем строку нулевым символом
            uartInputBuffer[uartInputBufferIndex] = '\0';
            uartInputBufferIndex = 0;

            // Проверяем формат строки
            if (strncmp(uartInputBuffer, "CNE=", 4) == 0/** &&
                uartInputBuffer[strlen(uartInputBuffer) - 2] == '\r'*/) {
                // Извлекаем фрагмент %s
                size_t fragmentLength = strlen(uartInputBuffer) - 6; // Учитываем "CNE=", "\r\n"
                if (fragmentLength > sizeof(lastFindedFragment) - 1) {
                    fragmentLength = sizeof(lastFindedFragment) - 1; // Ограничиваем размер
                }
                strncpy(lastFindedFragment, uartInputBuffer + 4, fragmentLength);
                lastFindedFragment[fragmentLength] = '\0'; // Завершаем строку

                // Обновляем время последнего найденного фрагмента
                lastFragmentTime = millis();

                // Проверяем содержимое lastFindedFragment и управляем светодиодами и лентой
                if (strncmp(lastFindedFragment, "Eng", 3) == 0) {
                    digitalWrite(LED_PIN1, HIGH);
                    digitalWrite(LED_PIN2, LOW);
                    Serial.println("ENG");
                    setStripColor(0, 0, 255); // Синий
                } else if (strncmp(lastFindedFragment, "Rus", 3) == 0) {
                    digitalWrite(LED_PIN1, LOW);
                    digitalWrite(LED_PIN2, HIGH);
                    Serial.println("RUS");
                    setStripColor(255, 0, 0); // Красный
                } else {
                    digitalWrite(LED_PIN1, LOW);
                    digitalWrite(LED_PIN2, LOW);
                    Serial.println("NONE");
                    setStripColor(0, 255, 255); // Голубой
                }
            }
        }
    }

    // Проверяем таймаут для гашения светодиодов
    if (millis() - lastFragmentTime > timeout) {
        digitalWrite(LED_PIN1, LOW);
        digitalWrite(LED_PIN2, LOW);
        setStripColor(0, 0, 0); // Выключаем ленту
       // Serial.println("TIMEOUT");
    }
}

// Функция для установки цвета всей ленты
void setStripColor(uint8_t r, uint8_t g, uint8_t b) {
    for (int i = 0; i < LED_COUNT; i++) {
        strip.setPixelColor(i, strip.Color(r, g, b));
    }
    strip.show();
}
