#include "program_vumeter.h"
bool debug_vumeter = false;

spectrum_mode spec_mode;
float vumeter_level_old[NUM_STRIPS*STRIP_SPLIT];
float vumeter_smoothing_coeff_positive = 1.0; // experiment with different coefficients; --> 0.0 < smoothing_coeff < 1.0
float vumeter_smoothing_coeff_negative = 0.1;

// AUDIO OBJECTS
// Audio objects are defined in main.h because they are shared between multiple programs

// These parameters adjust the vertical thresholds
const float maxLevel = 0.5;      // 1.0 = max, lower is more "sensitive"
const float dynamicRange = 40.0; // total range to display, in decibels
const float linearBlend = 0.3;   // useful range is 0 to 0.7
// This array holds the volume level (0 to 1.0) for each
// vertical pixel to turn on.  Computed in setup() using
// the 3 parameters above.
float thresholdVertical[NUM_LEDS_PER_STRIP/STRIP_SPLIT];

// This array specifies how many of the FFT frequency bin
// to use for each horizontal pixel.  Because humans hear
// in octaves and FFT bins are linear, the low frequencies
// use a small number of bins, higher frequencies use more.
int frequencyBinsHorizontal[NUM_STRIPS*STRIP_SPLIT] = {
   1, 2, 4, 8, 16, 32, 64, 128
};

void vumeter()
{
  computeVerticalLevels();
  active_program = VUMETER;
  update = true;
  interval = 10;
  spec_mode = GRADIENT;
  active_palette = &spectrum_classic;
  saturation1 = 255;
  value1 = 255;
}

void vumeterUpdate()
{
  if(debug_vumeter) {
    Serial.println("Er vi her?");
  }
  unsigned int x, y, freqBin;
  float level;
  uint8_t gradientIndex;

  if (fft.available())
  {
    // freqBin counts which FFT frequency data has been used,
    // starting at low frequency
    freqBin = 0;

    for (x=0; x < NUM_STRIPS*STRIP_SPLIT; x++)
    {
      // get the volume for each horizontal pixel position
      level = fft.read(freqBin, freqBin + frequencyBinsHorizontal[x] - 1);
      float diff = level - vumeter_level_old[x];
      if (diff > 0.0) {
        level = vumeter_smoothing_coeff_positive * level + (1.0 - vumeter_smoothing_coeff_positive) * vumeter_level_old[x];
        //level = level + diff * smoothing_coeff_positive;
      } else {
        level = vumeter_smoothing_coeff_negative * level + (1.0 - vumeter_smoothing_coeff_negative) * vumeter_level_old[x];
        //level = level + diff * smoothing_coeff_negative;
      }


      for (y=0; y < NUM_LEDS_PER_STRIP/STRIP_SPLIT; y++) {
        // for each vertical pixel, check if above the threshold
        // and turn the LED on or off
        gradientIndex = (y+1)*(255/(NUM_LEDS_PER_STRIP/STRIP_SPLIT));
        //gradientIndex = 200;

        if (level >= thresholdVertical[y]) {
          if (spec_mode == RAINBOW)
          {
            leds[led_order_array[xy(x,y)]] = CHSV(gradientIndex,saturation1,value1);
          }
          if(spec_mode == GRADIENT)
          {
            leds[led_order_array[xy(x,y)]] = ColorFromPalette(*active_palette,gradientIndex);
          }
        }
        else
        {
          leds[led_order_array[xy(x,y)]] = CRGB::Black;
        }
      }
      // increment the frequency bin count, so we display
      // low to higher frequency from left to right
      freqBin = freqBin + frequencyBinsHorizontal[x];
      vumeter_level_old[x] = level;
    }
    // after all pixels set, show them all at the same instant
    FastLED.show();
  }
}

// A simple xy() function to turn display matrix coordinates
// into the index numbers OctoWS2811 requires.  If your LEDs
// are arranged differently, edit this code...
unsigned int xy(unsigned int x, unsigned int y)
{
  return ((NUM_LEDS_PER_STRIP/STRIP_SPLIT)*x)+y;
}

// Run once from setup, the compute the vertical levels
void computeVerticalLevels() {
  unsigned int y;
  float n, logLevel, linearLevel;
  uint8_t x = NUM_LEDS_PER_STRIP/STRIP_SPLIT;
  for (y=0; y < NUM_LEDS_PER_STRIP/STRIP_SPLIT; y++) {
    n = (float)x / (float)(NUM_LEDS_PER_STRIP/STRIP_SPLIT - 1);
    logLevel = pow10f(n * -1.0 * (dynamicRange / 20.0));
    linearLevel = 1.0 - n;
    linearLevel = linearLevel * linearBlend;
    logLevel = logLevel * (1.0 - linearBlend);
    thresholdVertical[y] = (logLevel + linearLevel) * maxLevel;
    x--;
  }
}
