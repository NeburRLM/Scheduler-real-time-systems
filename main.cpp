#include "Grove_LCD_RGB_Backlight.h"
#include "mbed.h"
#include "photoresistor.h"
#include <cmath>
#include <cstdio>
#include <iomanip>
#include <iostream>
#include <sstream>

Grove_LCD_RGB_Backlight lcd(D14, D15);
#define SIZE 100
#define DEADLINE 100
// INPUTS
InterruptIn button(D4);
bool calculate_mean = false;
PwmOut buzzer(D5);
AnalogIn potentiometer(A1);
PwmOut led(D3);
float queue[SIZE];
int indexQueue = 0;
uint64_t tempsInici;
uint64_t budget;
Timer t;

void errorBuzzer() { buzzer.write(0.25); }

float valorPot() {
  float lectura = potentiometer.read();
  if (lectura < 0) {
    errorBuzzer();
  }
  return lectura;
}

float calculate_meanFunction() {
  float mean = 0;
  calculate_mean = false;
  printf("Ind: %d", indexQueue);
  for (int i = 0; i < indexQueue; i++) {
    mean = mean + queue[i];
    // printf("Lux: %.4f ºC\n", mean);
  }
  mean = (mean / indexQueue);
  indexQueue = 0;
  button.enable_irq();
  return mean;
}

void buttonCalculateMean() {
  button.disable_irq();
  calculate_mean = true;
  t.reset();
}

bool comprovarDeadline() {
  return (Kernel::get_ms_count() - tempsInici) <= DEADLINE;
}

int main() {
  tempsInici = Kernel::get_ms_count();
  t.start();
  float lux = 0;
  float mean = 0;
  int pMean = 0;
  bool mostrate = false;
  float compens = 0;
  float pot = 0;
  button.rise(&buttonCalculateMean);
  buzzer.period(0.10);
  lcd.setRGB(0, 0, 255);

  while (1) {
    tempsInici = Kernel::get_ms_count();
    lux = calcularLux();
    compens = 100 - lux;
    pot = valorPot() * 100;

    buzzer.write(0);

    // printf("Pt %.4f ºC\n", pot);
    if (compens > pot) {
      compens = pot;
    }
    led.write(compens / 100);

    if (calculate_mean) {
      if (t.read_ms() > 10000) {
        mean = calculate_meanFunction();
        pMean = 24;
        mostrate = false;
      } else {
        queue[indexQueue] = lux;
        indexQueue++;
      }
    }

    if (pMean > 0 && mostrate == false) {
      std::ostringstream stream;
      stream << "Mean: " << std::fixed << std::setprecision(2) << mean << ""
             << char(0xDF) << "%";
      std::string message = stream.str();
      char output[message.length() + 1];
      strcpy(output, message.c_str());

      lcd.clear();
      lcd.locate(0, 0);
      lcd.print(output);
      mostrate = true;
    } else {

      if (pMean <= 0) {
        std::ostringstream stream;
        stream << "Lux: " << std::fixed << std::setprecision(2) << lux << ""
               << char(0xDF) << "%";
        std::string message = stream.str();
        char output[message.length() + 1];
        strcpy(output, message.c_str());

        std::ostringstream stream2;
        stream2 << "Compens: " << std::fixed << std::setprecision(2) << compens
                << "" << char(0xDF) << "%";
        std::string mensajeAbajo = stream2.str();
        char outputAbajo[mensajeAbajo.length() + 1];
        strcpy(outputAbajo, mensajeAbajo.c_str());

        lcd.clear();
        lcd.locate(0, 0);
        lcd.print(output);
        lcd.locate(0, 1);
        lcd.print(outputAbajo);
      } else {
        pMean--;
      }
    }
    if (comprovarDeadline()) {
      budget = DEADLINE - (Kernel::get_ms_count() - tempsInici);
      printf("Budget: %llu \n", budget);
      ThisThread::sleep_for(budget);
    } else {
      errorBuzzer();
    }
    // ThisThread::sleep_for(1s);
  }
}