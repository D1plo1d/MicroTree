#include "audio_react.h"

// If set to true, messages will be output to serial console
const bool debug_audioreact = 1;

// Audio library objects
// AudioInputAnalog         adc1(A3);
// AudioAnalyzeFFT1024      fft;
// AudioConnection          patchCord1(adc1, fft);

// GUItool: begin automatically generated code
AudioInputAnalog         adc1(A3);
AudioFilterBiquad        biquad1;
AudioAnalyzeFFT1024      fft;
AudioConnection          patchCord1(adc1, biquad1);
AudioConnection          patchCord2(biquad1, fft);
// GUItool: end automatically generated code


const unsigned int audio_rective_setting_max_value = 255;
// These parameters adjust the vertical thresholds
const float maxLevel = 0.5;      // 1.0 = max, lower is more "sensitive"
const float dynamicRange = 60.0; // total range to display, in decibels
const float linearBlend = 0.1;   // useful range is 0 to 0.7
// This array holds the volume level (0 to 1.0) for each
// vertical pixel to turn on.  Computed in setup() using
// the 3 parameters above.
float thresholdVertical[audio_rective_setting_max_value];
float level_old;
float smoothing_coeff_positive = 1.0; // experiment with different coefficients; --> 0.0 < smoothing_coeff < 1.0
float smoothing_coeff_negative = 1.0;

//Setting some start values for the sound reactivity. These are later supposed
//to be manipulated with TouchOSC
bool react_to_audio = 0;
uint16_t fft_stop = 3;
uint16_t fft_start = 3;
float bandpass_freq = 130.0;
float bandpass_width = 0.1;



void audioReact(audio_reactive_setting setting)
{
  if(react_to_audio)
  {
    biquad1.setBandpass(0, bandpass_freq, bandpass_width);
    float level;
    if (fft.available())
    {
      //In casee the user sets stop higher than start via TouchOSC
      if(fft_stop > fft_start) {
        fft_stop = fft_start;
      }
      level = fft.read(fft_start,fft_stop);

      if((level-level_old) > 0.0)
      {
        level = smoothing_coeff_positive * level + (1.0 - smoothing_coeff_positive) * level_old;
      }
      else
      {
        level = smoothing_coeff_negative * level + (1.0 - smoothing_coeff_negative) * level_old;
      }
      level_old = level;
      value2 = (level * audio_rective_setting_max_value) * 10;
    }
  }
}

void computeVerticalLevels() {
  unsigned int y;
  float n, logLevel, linearLevel;

  for (y=0; y < audio_rective_setting_max_value; y++) {
    n = (float)y / (float)(audio_rective_setting_max_value - 1);
    logLevel = pow10f(n * -1.0 * (dynamicRange / 20.0));
    linearLevel = 1.0 - n;
    linearLevel = linearLevel * linearBlend;
    logLevel = logLevel * (1.0 - linearBlend);
    thresholdVertical[y] = (logLevel + linearLevel) * maxLevel;
  }
}

void sendValuesToTouchosc()
{
  OSCMsgSend("/react/toggle", (float)react_to_audio);
  OSCMsgSend("/fft/start/value", (float)fft_start);
  OSCMsgSend("/fft/stop/value", (float)fft_stop);
  OSCMsgSend("/bandpass/freq/value", (float)bandpass_freq);
  OSCMsgSend("/bandpass/width/value", (float)bandpass_width);
}

void setAudioReact(bool react) {
  react_to_audio = react;
  OSCMsgSend("/react/toggle", (float)react_to_audio);
  sendValuesToTouchosc();

  if(debug_audioreact) {
    Serial.print("Toggeling audio react: ");
    Serial.println(react_to_audio);
  }
}

void setFftStart(uint16_t fft) {
  fft_start = fft;
  OSCMsgSend("/fft/start/value", (float)fft_start);

  if(debug_audioreact) {
    Serial.print("FFT start: ");
    Serial.println(fft_start);
  }
}

void setFftStop(uint16_t fft) {
  fft_stop = fft;
  OSCMsgSend("/fft/stop/value", (float)fft_stop);

  if(debug_audioreact) {
    Serial.print("FFT stop: ");
    Serial.println(fft_stop);
  }
}

void setBandpassFreq(uint16_t freq) {
  bandpass_freq = freq;
  OSCMsgSend("/bandpass/freq/value", (float)bandpass_freq);

  if(debug_audioreact) {
    Serial.print("Bandpass frequency: ");
    Serial.println(bandpass_freq);
  }
}

void setBandpassWidth(float width) {
  bandpass_width = width;
  OSCMsgSend("/bandpass/width/value", (float)bandpass_width);

  if(debug_audioreact) {
    Serial.print("Bandpass width: ");
    Serial.println(bandpass_width);
  }
}

void setSmoothPos(float smooth_pos) {
  smoothing_coeff_positive = smooth_pos;
  OSCMsgSend("/react/smooth_pos/value", (float)smoothing_coeff_positive);

  if(debug_audioreact) {
    Serial.print("Smoothing coefficient positive: ");
    Serial.println(smoothing_coeff_positive);
  }
}

void setSmoothNeg(float smooth_neg) {
  smoothing_coeff_negative = smooth_neg;
  OSCMsgSend("/react/smooth_neg/value", (float)smoothing_coeff_negative);

  if(debug_audioreact) {
    Serial.print("Smoothing coefficient negative: ");
    Serial.println(smoothing_coeff_negative);
  }
}

void changeAudioReactSettings(OSCMessage &msg, int addrOffset)
{
  if (msg.fullMatch("/react/toggle"))
  {
    float value = msg.getFloat(0);
    setAudioReact((bool)value);
  }
  if (msg.fullMatch("/react/bandpass/freq"))
  {
    float value = msg.getFloat(0);
    setBandpassFreq((uint16_t)value);
  }
  if (msg.fullMatch("/react/bandpass/width"))
  {
    float value = msg.getFloat(0);
    setBandpassWidth((uint16_t)value);
  }
  if (msg.fullMatch("/react/fft/start"))
  {
    float value = msg.getFloat(0);
    setFftStart((uint16_t)value);
  }
  if (msg.fullMatch("/react/fft/stop"))
  {
    float value = msg.getFloat(0);
    setFftStop((uint16_t)value);
  }
  if (msg.fullMatch("/react/smooth_pos"))
  {
    float value = msg.getFloat(0);
    setSmoothPos(value);
  }
  if (msg.fullMatch("/react/smooth_neg"))
  {
    float value = msg.getFloat(0);
    setSmoothNeg(value);
  }
}
