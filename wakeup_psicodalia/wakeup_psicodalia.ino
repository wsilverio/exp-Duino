#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
#include <avr/power.h>
#endif

// constantes da media exponencial movel
#define NM 20.0         // numero de medias
#define ALPHA NM/(NM+1) // coeficiente exponencial
// constantes do debounce
#define DEBOUNCING_MS 5
#define TEMPO_MAX 3000 // 3s
// constantes de ativacao dos LEDs
#define NEXTUPDATE true
#define UPDATED false
#define NEO_PIN 2

// numero de fitas
const uint8_t NSTRIPS = 10;
Adafruit_NeoPixel strip = Adafruit_NeoPixel(NSTRIPS, NEO_PIN, NEO_RGB + NEO_KHZ800);

// enumeracao para acesso ao vetor dos botoes
enum BUTTONS {BT_0 = 0, BT_1, BT_2, BT_3, BT_4, BT_5, BT_6, BT_7, BT_8, BT_9, BT_MODE, BT_BLACKOUT, NUM_BUTTONS};
uint8_t BUTTON_PIN[NUM_BUTTONS];

// enumeracao para acesso ao vetor dos potenciometros
enum POTS {POT_R1 = 0, POT_G1, POT_B1, POT_R2, POT_G2, POT_B2, POT_BRIGHTNESS, POT_TEMPO, NUM_POTS};
uint8_t POT_PIN[NUM_POTS];

// media exponencial
float valPot[NUM_POTS][2] = {0};
// cores para LerpColor
uint32_t color1, color2;
// tempos de atualizacao
unsigned long timeNow, timeLastCheck = 0;
// blackout
bool blackout = false;

void setup() {
  // Serial.begin(57600);  
  // inicializa a fita
  strip.begin();
  strip.show();

  // pinos dos botoes
  BUTTON_PIN[BT_0] = 53;
  BUTTON_PIN[BT_1] = 51;
  BUTTON_PIN[BT_2] = 49;
  BUTTON_PIN[BT_3] = 47;
  BUTTON_PIN[BT_4] = 45;
  BUTTON_PIN[BT_5] = 43;
  BUTTON_PIN[BT_6] = 41;
  BUTTON_PIN[BT_7] = 39;
  BUTTON_PIN[BT_8] = 37;
  BUTTON_PIN[BT_9] = 35;
  BUTTON_PIN[BT_MODE] = 33;
  BUTTON_PIN[BT_BLACKOUT] = 31;
  // pinos dos potenciometros
  POT_PIN[POT_R1] = A15;
  POT_PIN[POT_G1] = A14;
  POT_PIN[POT_B1] = A13;
  POT_PIN[POT_R2] = A8;
  POT_PIN[POT_G2] = A9;
  POT_PIN[POT_B2] = A10;
  POT_PIN[POT_BRIGHTNESS] = A12;
  POT_PIN[POT_TEMPO] = A11;

  for (uint8_t i = BT_0; i < NUM_BUTTONS; i++) {
    pinMode(BUTTON_PIN[i], INPUT_PULLUP);
  }
}

void loop() {
  if (!digitalRead(BUTTON_PIN[BT_BLACKOUT])) {
    blackout = true;
    strip.clear();
    strip.show();
  } else {
    blackout = false;
  }

  enum MODES {
    MENU_MIN,
    MANUAL, MANUAL_TOGGLE, SEQUENTIAL, SEQUENTIAL_INV, ROTATE, ROTATE_INV, SMOOTH, RANDOM,
    MENU_MAX
  };

  static uint8_t mode = MENU_MIN + 1;

  if (!digitalRead(BUTTON_PIN[BT_MODE])) {

    mode++;

    if (mode >= MENU_MAX) {
      mode = MENU_MIN + 1;
    }

    DebouncingPin(BUTTON_PIN[BT_MODE], DEBOUNCING_MS);
  }

  static boolean updateStrip = NEXTUPDATE;

  if (updateStrip == NEXTUPDATE) {

    static uint8_t lastMode = MENU_MIN+1;

    // media exp movel
    // float brightness = (readAnalogAndSetExpMed(POT_BRIGHTNESS) / 1023.0f);
    float brightness = (analogRead(POT_PIN[POT_BRIGHTNESS]) / 1023.0f);

    color1 = strip.Color(
               brightness * (readAnalogAndSetExpMed(POT_R1) >> 2),
               brightness * (readAnalogAndSetExpMed(POT_G1) >> 2),
               brightness * (readAnalogAndSetExpMed(POT_B1) >> 2));

    color2 = strip.Color(
               brightness * (readAnalogAndSetExpMed(POT_R2) >> 2),
               brightness * (readAnalogAndSetExpMed(POT_G2) >> 2),
               brightness * (readAnalogAndSetExpMed(POT_B2) >> 2));

    switch (mode) {
      case MANUAL:
        {
          for (uint8_t i = BT_0; i < NSTRIPS; i++) {
            if (!digitalRead(BUTTON_PIN[i])) {
              strip.setPixelColor(i, LerpColor(color1, color2, float(i) / NSTRIPS));
            } else {
              strip.setPixelColor(i, 0, 0, 0);
            }
          }
          updateStrip = UPDATED;
          lastMode = mode;
        } break;
      case MANUAL_TOGGLE:
        {
          static bool boolStrip[NSTRIPS] = {false};

          if (lastMode != mode)
            memset(boolStrip, false, NSTRIPS * sizeof(uint8_t));

          for (uint8_t i = BT_0; i < NSTRIPS; i++) {
            if (!digitalRead(BUTTON_PIN[i])) {
              boolStrip[i] = !boolStrip[i];
              if (boolStrip[i])
                strip.setPixelColor(i, LerpColor(color1, color2, float(i) / NSTRIPS));
              else
                strip.setPixelColor(i, 0, 0, 0);

              DebouncingPin(BUTTON_PIN[i], DEBOUNCING_MS);
            } else if (boolStrip[i]) {
              strip.setPixelColor(i, LerpColor(color1, color2, float(i) / NSTRIPS));
            }
          }
          lastMode = mode;
        } break;
      case SEQUENTIAL:
        {
          static int position = -1;

          strip.clear();
          position++;

          if (position >= NSTRIPS)
            position = 0;

          strip.setPixelColor(position, LerpColor(color1, color2, (float)position / NSTRIPS));
          updateStrip = UPDATED;
          lastMode = mode;
        } break;
      case SEQUENTIAL_INV:
        {
          static int position = -1;
          static int step = 1;

          strip.clear();
          position += step;

          if (position >= NSTRIPS) {
            position = NSTRIPS - 2;
            step = -1;
          } else if (position < 0) {
            position = 1;
            step = 1;
          }
          strip.setPixelColor(position, LerpColor(color1, color2, (float)position / NSTRIPS));
          updateStrip = UPDATED;
          lastMode = mode;
        } break;
      case RANDOM:
        {
          static bool boolStrips[NSTRIPS] = { false };

          unsigned int randomIndex = random(NSTRIPS);
          boolStrips[randomIndex] = !boolStrips[randomIndex];

          for (uint8_t i = 0; i < NSTRIPS; ++i) {
            if (boolStrips[i])
              strip.setPixelColor(i, LerpColor(color1, color2, (float)i / NSTRIPS));
            else
              strip.setPixelColor(i, 0, 0, 0);
          }

          static bool firstCall = true;
          if (firstCall) {
            strip.clear();
            firstCall = false;
          }
          updateStrip = UPDATED;
          lastMode = mode;
        } break;
      case SMOOTH:
        {
          static float amt = 0;
          static float step = 0.01;

          amt += step;

          if (amt > 1) {
            amt = 1;
            step = -0.01;
          } else if (amt < 0) {
            amt = 0;
            step = 0.01;
          }

          for (uint8_t i = 0; i < NSTRIPS; i++) {
            strip.setPixelColor(i, LerpColor(color1, color2, amt));
          }
          updateStrip = UPDATED;
          lastMode = mode;
        } break;
      case ROTATE:
        {
          static uint8_t lastArray[NSTRIPS];
          uint8_t thisArray[NSTRIPS];

          static boolean firstCall = true;

          if (firstCall) {
            lastArray[0] = NSTRIPS - 1;
            for (uint8_t i = 1; i < NSTRIPS; ++i) {
              lastArray[i] = i - 1;
            }
            firstCall = false;
          }

          for (uint8_t i = 0; i < NSTRIPS; i++) {
            if (!i) {
              thisArray[i] = lastArray[NSTRIPS - 1];
            } else {
              thisArray[i] = lastArray[i - 1];
            }

            strip.setPixelColor(i, LerpColor(color1, color2, (float)thisArray[i] / NSTRIPS));
          }

          memcpy(lastArray, thisArray, NSTRIPS * sizeof(uint8_t));
          updateStrip = UPDATED;
          lastMode = mode;
        } break;
      case ROTATE_INV:
        {
          static int8_t step = 1;
          static uint8_t lastArray[NSTRIPS];
          uint8_t thisArray[NSTRIPS];

          static boolean firstCall = true;

          if (firstCall) {
            lastArray[0] = NSTRIPS - 1;
            for (uint8_t i = 1; i < NSTRIPS; ++i) {
              lastArray[i] = i - 1;
            }
            firstCall = false;
          }


          for (uint8_t i = 0; i < NSTRIPS; i++) {
            if (i == 0 && step > 0) {
              thisArray[i] = lastArray[NSTRIPS - 1];
            } else if (i == NSTRIPS - 1 && step < 0) {
              thisArray[i] = lastArray[0];
            } else {
              thisArray[i] = lastArray[i - step];
            }

            if (step)
              strip.setPixelColor(i, LerpColor(color1, color2, (float)thisArray[i] / NSTRIPS));
            else
              strip.setPixelColor(i, LerpColor(color2, color1, (float)thisArray[i] / NSTRIPS));
          }

          if ((step > 0 && thisArray[NSTRIPS - 1] == (NSTRIPS-1)) || (step < 0 && thisArray[0] == 0))
            step = -step;

          memcpy(lastArray, thisArray, NSTRIPS * sizeof(uint8_t));
          updateStrip = UPDATED;
          lastMode = mode;
        } break;
      default:
        {} break;
    }
  }

  unsigned int tempo = map(readAnalogAndSetExpMed(POT_TEMPO), 0, 1023.0f, 0, TEMPO_MAX);
  timeNow = millis();

  if (mode != MANUAL && mode != MANUAL_TOGGLE) {
    if ((timeNow - timeLastCheck) >= tempo) {
      timeLastCheck = timeNow;
    } else {
      return; // loop
    }
  }

  SafeStripShow();
  updateStrip = NEXTUPDATE;

}

// media exponencial movel
uint16_t readAnalogAndSetExpMed(const uint8_t POT) {
  valPot[POT][1] = ALPHA*valPot[POT][0] + (1-ALPHA)*analogRead(POT_PIN[POT]);
  valPot[POT][0] = valPot[POT][1];
  return valPot[POT][1];
}

inline void DebouncingPin(uint8_t pin, unsigned int ms) {
  delay(ms);
  while (!digitalRead(pin));
  delay(ms);
}

int SafeStripShow() {
  if (blackout)
    return 1;

  strip.show();
  return 0;
}

uint32_t LerpColor(const uint32_t& from, const uint32_t& to, float amount) {
  if (amount < 0) amount = 0;
  if (amount > 1) amount = 1;

  float a1 = ((from >> 24) & 0xff);
  float r1 = (from >> 16) & 0xff;
  float g1 = (from >> 8) & 0xff;
  float b1 = from & 0xff;
  float a2 = (to >> 24) & 0xff;
  float r2 = (to >> 16) & 0xff;
  float g2 = (to >> 8) & 0xff;
  float b2 = to & 0xff;

  return ((round(a1 + (a2 - a1) * amount) << 24) |
          (round(r1 + (r2 - r1) * amount) << 16) |
          (round(g1 + (g2 - g1) * amount) << 8)  |
          (round(b1 + (b2 - b1) * amount)));
}
