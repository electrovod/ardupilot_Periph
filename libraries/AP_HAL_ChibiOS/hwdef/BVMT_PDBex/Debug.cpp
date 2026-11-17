
extern const AP_HAL::HAL &hal;

AP_Periph_FW periph;


// GC_Debug:
// Wave Pin:
#define WavePin 2
#define WavePin2 1
hal.gpio->pinMode( WavePin, HAL_GPIO_OUTPUT);
hal.gpio->pinMode( WavePin2, HAL_GPIO_OUTPUT);
hal.gpio->write( WavePin, 1); 
hal.gpio->write( WavePin2, 1); 
hal.scheduler->delay(1);
hal.gpio->write( WavePin, 0); 
hal.scheduler->delay(1);
hal.gpio->write( WavePin, 1); 
hal.gpio->write( WavePin2, 0); 
hal.scheduler->delay(1);
hal.gpio->write( WavePin, 0); 
hal.gpio->write( WavePin2, 1); 
hal.scheduler->delay(1);
hal.gpio->write( WavePin, 1); 

hal.scheduler->delay_microseconds(50);                                    // Interval:
hal.gpio->write( WavePin, 0);
hal.scheduler->delay_microseconds(20);                                    // Interval:
hal.gpio->write( WavePin, 1);

    
// Short Wave Pin:
hal.scheduler->delay(1);
hal.gpio->write( WavePin2, 0); 
hal.scheduler->delay(2);
hal.gpio->write( WavePin2, 1); 
hal.scheduler->delay(1);

