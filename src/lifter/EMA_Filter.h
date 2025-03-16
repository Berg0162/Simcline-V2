
/* 
The exponential moving average (EMA) filter is a discrete, low-pass, infinite-impulse response (IIR) filter. 
It places more weight on recent data by discounting old data in an exponential fashion, and behaves similarly 
to the discrete first-order low-pass RC filter.
Unlike a simple moving average (SMA) filter, most EMA filters are not windowed, and the next value depends on 
all previous inputs. Thus most EMA filters are a form of infinite impulse response (IIR) filter, whilst a SMA 
is a finite impulse response (FIR) filter. There are exceptions, and you can indeed build a windowed 
exponential moving average filter in where the coefficients are weighted exponentially.
*/

// emaAlpha Factor should be between low (10-40) is maximal and high (50-90) is minimal filtering

class EMA_Filter {
private:
  float emaAlphaFactor;
  float exponential_average;
public:
  // constructor
  EMA_Filter(const float emaAlpha) {
    emaAlphaFactor = emaAlpha;
  }

  float getFilteredValue(float current_value) {
    exponential_average = (emaAlphaFactor*current_value + (100-emaAlphaFactor)*exponential_average) / 100 ;
    return exponential_average;
  }  
  
  void presetFilter(float current_value) {
    exponential_average = current_value;
  }
};

