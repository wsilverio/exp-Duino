#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
  #include <avr/power.h>
#endif

#define DEBOUNCING_MS 5
#define TEMPO_MAX 3000 // 3s

#define NSTRIPS 10
#define NEO_PIN 2

Adafruit_NeoPixel strip = Adafruit_NeoPixel(NSTRIPS, NEO_PIN, NEO_RGB + NEO_KHZ800);

enum{
    BT0 = 53,
    BT1 = 51,
    BT2 = 49,
    BT3 = 47,
    BT4 = 45,
    BT5 = 43,
    BT6 = 41,
    BT7 = 39,
    BT8 = 37,
    BT9 = 35,
    BTM = 33,
    BTB = 31
}BUTTONS;

// BOTOES
const uint8_t BUTTON[] = { BT0, BT1, BT2, BT3, BT4, BT5, BT6, BT7, BT8, BT9 };
const uint8_t BUTTON_MODE = BTM;
const uint8_t BUTTON_BLACKOUT = BTB;

// POT
const uint8_t POT_R1 = A15;
const uint8_t POT_G1 = A14;
const uint8_t POT_B1 = A13;
const uint8_t POT_R2 = A8;
const uint8_t POT_G2 = A9;
const uint8_t POT_B2 = A10;
const uint8_t POT_BRIGHTNESS = A12;
const uint8_t POT_TEMPO = A11;

uint32_t color1, color2;
unsigned long timeNow, timeLastCheck = 0;
bool blackout = false;
volatile bool blackoutIntCalled = false;

void setup() {
    strip.begin();
    strip.show();

    // Serial.begin(57600);

    for(int i=0; i<NSTRIPS; i++){
        pinMode(BUTTON[i], INPUT_PULLUP);
    }
    
    pinMode(BUTTON_MODE, INPUT_PULLUP);
    pinMode(BUTTON_BLACKOUT, INPUT_PULLUP);

    // color1 = strip.Color(255, 255, 255);
    // color2 = strip.Color(255, 255, 255);

    // attachInterrupt(digitalPinToInterrupt(BUTTON_BLACKOUT), blackoutInt, FALLING);

}

void loop() {
    if(!digitalRead(BUTTON_BLACKOUT)){
        // noInterrupts();
        blackout = true;
        // if(blackout){
            strip.clear();
            strip.show();
            
            // Serial.println(F("backout!!!"));
        // }

        // DebouncingPin(BUTTON_BLACKOUT, DEBOUNCING_MS);
        // blackoutIntCalled = false;
        // interrupts();
    }else{
        blackout = false;  
    }

    // enum{
    //     MENU_MIN,
    //         MANUAL, MANUAL_TOGGLE, SEQUENTIAL, SEQUENTIAL_INV, ROTATE, ROTATE_INV, SMOOTH, RANDOM,
    //     MENU_MAX
    // }MODES;

    enum{
        MENU_MIN,
            MANUAL, MANUAL_TOGGLE, SEQUENTIAL, SEQUENTIAL_INV, ROTATE, ROTATE_INV, SMOOTH, RANDOM,
        MENU_MAX
    }MODES;
    
    static uint8_t mode = MENU_MIN+1;
    static uint8_t lastMode = mode;
    
    if(!digitalRead(BUTTON_MODE)){
        mode++;
        if(mode >= MENU_MAX)
            mode = MENU_MIN+1;
        
         // Serial.print(F("mode: "));
         // Serial.println(mode);

        DebouncingPin(BUTTON_MODE, DEBOUNCING_MS);
    }

    unsigned int tempo = map(analogRead(POT_TEMPO), 0, 1023, 0, TEMPO_MAX);
    timeNow = millis();

     // Serial.print(F("tempo: "));
     // Serial.println(tempo);

    if(mode != MANUAL && mode != MANUAL_TOGGLE){
        if((timeNow - timeLastCheck) >= tempo){
            timeLastCheck = timeNow;
        }else{
            return; // loop
        }
    }

    
    float brightness;
    static float lastBrightness = 0;
    for(int i = 0; i < 5; i++){
        brightness += (float)(analogRead(POT_BRIGHTNESS))/1023.0f;
    }
    brightness /= 5.0f;

    if(abs(brightness - lastBrightness) > 0.03){
        lastBrightness = brightness;
    }

    // Serial.print(F("brightness*: "));
    // Serial.println(lastBrightness);

    color1 = strip.Color(
        lastBrightness*analogRead(POT_R1)/4,
        lastBrightness*analogRead(POT_G1)/4,
        lastBrightness*analogRead(POT_B1)/4);

    color2 = strip.Color(
        brightness*analogRead(POT_R2)/4,
        brightness*analogRead(POT_G2)/4,
        brightness*analogRead(POT_B2)/4);

     // Serial.print(F("R1: "));
     // Serial.println(analogRead(POT_R1)/4);
     // Serial.print(F("G1: "));
     // Serial.println(analogRead(POT_G1)/4);
     // Serial.print(F("B1: "));
     // Serial.println(analogRead(POT_B1)/4);
     // Serial.print(F("R2: "));
     // Serial.println(analogRead(POT_R2)/4);
     // Serial.print(F("G2: "));
     // Serial.println(analogRead(POT_G2)/4);
     // Serial.print(F("B2: "));
     // Serial.println(analogRead(POT_B2)/4);
     // Serial.println();

    // ************************************************************************ //
    // return;
    // ************************************************************************ //

    switch (mode) {
        case MANUAL:
            {
                for(int i=0; i<NSTRIPS; i++){
                    if(!digitalRead(BUTTON[i])){
                        strip.setPixelColor(i, LerpColor(color1, color2, float(i)/NSTRIPS));
                    }else{
                        strip.setPixelColor(i, 0, 0, 0);
                    }
                }
                SafeStripShow(strip);
                lastMode = mode;
            }break;
        case MANUAL_TOGGLE:
            {
                static bool boolStrip[NSTRIPS] = {false};

                if(lastMode != mode)
                    memset(boolStrip, false, NSTRIPS*sizeof(uint8_t));

                for(int i=0; i<NSTRIPS; i++){
                    if(!digitalRead(BUTTON[i])){
                        boolStrip[i] = !boolStrip[i];
                        if(boolStrip[i])
                            strip.setPixelColor(i, LerpColor(color1, color2, float(i)/NSTRIPS));
                        else
                            strip.setPixelColor(i, 0, 0, 0);

                        DebouncingPin(BUTTON[i], DEBOUNCING_MS);
                    }else if(boolStrip[i]){
                        strip.setPixelColor(i, LerpColor(color1, color2, float(i)/NSTRIPS));
                    }
                }
                SafeStripShow(strip);
                lastMode = mode;
            }break;
        case SEQUENTIAL:
            {
                static int position = -1;
                
                strip.clear();
                position++;

                if (position >= NSTRIPS)
                    position = 0;
                
                strip.setPixelColor(position, LerpColor(color1, color2, (float)position/NSTRIPS));
                SafeStripShow(strip);
                lastMode = mode;
            }break;
        case SEQUENTIAL_INV:
            {
                static int position = -1;
                static int step = 1;

                strip.clear();
                position += step;
                
                if (position >= NSTRIPS){
                    position = NSTRIPS - 2;
                    step = -1;
                }else if (position < 0){
                    position = 1;
                    step = 1;
                }
                strip.setPixelColor(position, LerpColor(color1, color2, (float)position/NSTRIPS));
                SafeStripShow(strip);
                lastMode = mode;
            }break;  
        case RANDOM:
            {
                static bool boolStrips[NSTRIPS] = { false };

                unsigned int randomIndex = random(NSTRIPS);
                boolStrips[randomIndex] = !boolStrips[randomIndex];

                for (int i = 0; i < NSTRIPS; ++i){
                    if (boolStrips[i])
                        strip.setPixelColor(i, LerpColor(color1, color2, (float)i/NSTRIPS));
                    else
                        strip.setPixelColor(i, 0, 0, 0);
                }

                static bool firstCall = true;
                if (firstCall){
                    strip.clear();
                    firstCall = false;
                }
                SafeStripShow(strip);
                lastMode = mode;
            }break; 
        case SMOOTH:
            {
                static float amt = 0;
                static float step = 0.01;
                
                amt += step;

                if (amt > 1){
                    amt = 1;
                    step = -0.01;
                }else if (amt < 0){
                    amt = 0;
                    step = 0.01;
                }
                
                for(int i=0; i<NSTRIPS; i++){
                    strip.setPixelColor(i, LerpColor(color1, color2, amt));
                }
                SafeStripShow(strip);
                lastMode = mode;
            }break;
        case ROTATE:
            {
                static uint8_t lastArray[NSTRIPS] = {9, 0, 1, 2, 3, 4, 5, 6, 7, 8};
                uint8_t thisArray[NSTRIPS];
                
                for(int i = 0; i < NSTRIPS; i++){
                    if(!i){
                        thisArray[i] = lastArray[NSTRIPS-1];
                    }else{
                        thisArray[i] = lastArray[i-1];
                    }

                    strip.setPixelColor(i, LerpColor(color1, color2, (float)thisArray[i]/NSTRIPS));
                }
                
                memcpy(lastArray, thisArray, NSTRIPS*sizeof(uint8_t));
                SafeStripShow(strip);
                lastMode = mode;
            }break; 
        case ROTATE_INV:
            {
                static int8_t step = 1;
                static uint8_t lastArray[NSTRIPS] = {9, 0, 1, 2, 3, 4, 5, 6, 7, 8};
                uint8_t thisArray[NSTRIPS];
                
                for(int i = 0; i < NSTRIPS; i++){
                    if(i == 0 && step > 0){
                        thisArray[i] = lastArray[NSTRIPS-1];
                    }else if(i == NSTRIPS-1 && step < 0){
                        thisArray[i] = lastArray[0];
                    }else{
                        thisArray[i] = lastArray[i-step];
                    }
                    
                    if(step)
                        strip.setPixelColor(i, LerpColor(color1, color2, (float)thisArray[i]/NSTRIPS));
                    else
                        strip.setPixelColor(i, LerpColor(color2, color1, (float)thisArray[i]/NSTRIPS));
                }
                
                if((step > 0 && thisArray[NSTRIPS-1] == 9) || (step < 0 && thisArray[0] == 0))
                    step = -step;
                    
                memcpy(lastArray, thisArray, NSTRIPS*sizeof(uint8_t));
                SafeStripShow(strip);
                lastMode = mode;
            }break;           
        default:
            {

            }break;
    }
}

void DebouncingPin(uint8_t pin, unsigned int ms){
    delay(ms);
    while(!digitalRead(pin));
    delay(ms);
}

int SafeStripShow(Adafruit_NeoPixel &ledStrip){
    if(blackout)
        return 1;

    ledStrip.show();
    return 0;
}

uint32_t LerpColor(const uint32_t& from, const uint32_t& to, float amount){
    if (amount < 0) amount = 0;
    if (amount> 1) amount =1;

    float a1 = ((from >> 24) & 0xff);
    float r1 = (from >> 16) & 0xff;
    float g1 = (from >> 8) & 0xff;
    float b1 = from & 0xff;
    float a2 = (to >> 24) & 0xff;
    float r2 = (to >> 16) & 0xff;
    float g2 = (to >> 8) & 0xff;
    float b2 = to & 0xff;

    return ((round(a1 + (a2-a1)*amount) << 24) |
            (round(r1 + (r2-r1)*amount) << 16) |
            (round(g1 + (g2-g1)*amount) << 8) |
            (round(b1 + (b2-b1)*amount)));
}

//void blackoutInt(){
//    blackoutIntCalled = true;
//}
