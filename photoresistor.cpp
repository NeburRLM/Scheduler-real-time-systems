#include "photoresistor.h"
#include <cmath>
#include "mbed.h"
#include <iostream>

AnalogIn photoresistor(A0);  
float calcularLux() {
    

    const double Rl = 10.0;  
    const double VRef = 3.3;    
    const double luxRel = 500.0;

    float ADCres = photoresistor.read()*3.3;

    float lux = (((VRef*luxRel)*ADCres)-luxRel)/Rl;
    if (lux<0){
        lux =0;
    }

    return (lux * 100 / luxRel);
}