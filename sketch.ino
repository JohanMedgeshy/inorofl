// Калькулятор на Arduino Uno: матричная клавиатура 4x4 + LCD 1602 (I2C)
// Клавиши:
//   0-9  цифры
//   *    десятичная точка (удержать ~0.7 с — полный сброс)
//   #    равно
//   A    +    B    -    C    ×    D    ÷

#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Keypad.h>

LiquidCrystal_I2C lcd(0x27, 16, 2);

const byte ROWS = 4;
const byte COLS = 4;
char keys[ROWS][COLS] = {
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'*', '0', '#', 'D'}
};
byte rowPins[ROWS] = {9, 8, 7, 6};
byte colPins[COLS] = {5, 4, 3, 2};
Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

const byte MAX_LEN = 10;

String input = "";      // вводимое число
double acc = 0;         // накопленное значение
char op = 0;            // текущая операция: + - * /
bool showResult = false;
bool error = false;

// ---------- вспомогательные функции ----------

String formatNumber(double v) {
  if (isnan(v) || isinf(v)) return "Error";
  char buf[20];
  dtostrf(v, 1, 4, buf);               // до 4 знаков после запятой
  String s = String(buf);
  if (s.indexOf('.') >= 0) {           // убираем лишние нули в конце
    while (s.endsWith("0")) s.remove(s.length() - 1);
    if (s.endsWith(".")) s.remove(s.length() - 1);
  }
  if (s == "-0") s = "0";
  if (s.length() > 16) return "Overflow";
  return s;
}

char opSymbol(char o) {
  switch (o) {
    case '+': return '+';
    case '-': return '-';
    case '*': return 'x';
    case '/': return '/';
  }
  return ' ';
}

void printLine(byte row, String text) {
  lcd.setCursor(0, row);
  lcd.print(text);
  for (byte i = text.length(); i < 16; i++) lcd.print(' ');
}

String rightAlign(String s) {
  while (s.length() < 16) s = " " + s;
  return s;
}

void updateDisplay() {
  if (error) {
    printLine(0, "");
    printLine(1, rightAlign("Error"));
    return;
  }
  if (showResult) {
    printLine(0, "=");
    printLine(1, rightAlign(formatNumber(acc)));
    return;
  }
  String top = "";
  if (op) top = formatNumber(acc) + " " + String(opSymbol(op));
  printLine(0, top);
  printLine(1, rightAlign(input.length() ? input : "0"));
}

void clearAll() {
  input = "";
  acc = 0;
  op = 0;
  showResult = false;
  error = false;
  updateDisplay();
}

double calculate(double a, char o, double b) {
  switch (o) {
    case '+': return a + b;
    case '-': return a - b;
    case '*': return a * b;
    case '/':
      if (b == 0) { error = true; return 0; }
      return a / b;
  }
  return b;
}

// ---------- обработка клавиш ----------

void onDigit(char k) {
  if (error || showResult) clearAll();
  if (input.length() >= MAX_LEN) return;
  if (input == "0") input = "";        // без ведущих нулей
  input += k;
}

void onDot() {
  if (error || showResult) clearAll();
  if (input.indexOf('.') >= 0 || input.length() >= MAX_LEN) return;
  if (input.length() == 0) input = "0";
  input += '.';
}

void onOperator(char o) {
  if (error) return;
  if (input.length() > 0) {
    double val = input.toDouble();
    acc = op ? calculate(acc, op, val) : val;
    if (error) { updateDisplay(); return; }
    input = "";
  }
  // если ввода нет, но есть результат — продолжаем с ним
  op = o;
  showResult = false;
}

void onEquals() {
  if (error || !op || input.length() == 0) return;
  acc = calculate(acc, op, input.toDouble());
  op = 0;
  input = "";
  showResult = true;
}

void keypadEvent(KeypadEvent key) {
  // удержание '*' — полный сброс
  if (keypad.getState() == HOLD && key == '*') clearAll();
}

void setup() {
  lcd.init();
  lcd.backlight();
  keypad.setHoldTime(700);
  keypad.addEventListener(keypadEvent);

  lcd.setCursor(0, 0);
  lcd.print("  Calculator");
  lcd.setCursor(0, 1);
  lcd.print("  Arduino Uno");
  delay(1200);
  clearAll();
}

void loop() {
  char k = keypad.getKey();
  if (!k) return;

  if (k >= '0' && k <= '9') onDigit(k);
  else if (k == '*') onDot();
  else if (k == '#') onEquals();
  else if (k == 'A') onOperator('+');
  else if (k == 'B') onOperator('-');
  else if (k == 'C') onOperator('*');
  else if (k == 'D') onOperator('/');

  updateDisplay();
}
