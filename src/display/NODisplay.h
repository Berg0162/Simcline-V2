#ifndef NODISPLAY_H_
#define NODISPLAY_H_

#include "IDisplay.h"
static const char* IDISPLAY = "NO Display";

class NODisplay : public IDisplay {
  private:

  public:
    NODisplay() {     
    }

    void initDisplay() override {
        // Display implementation
        Serial.println("NO display to initialize...");
	delay(250);
    }

    void setDisplayBrightness(bool updown) override {
        // Display implementation
    }

    void ShowIconsOnTopBar(bool isTrainer, bool isLaptop, bool isPhone) override {
        // Display implementation
    }

    void ShowMessageWindow(const String& Line1, const String& Line2, const String& Line3, uint16_t Pause) override {
        // Display implementation
        if(Pause > 0) vTaskDelay(Pause/portTICK_PERIOD_MS); // Pause indicated time in ms
    }

    void ShowRoadGrade(float gradePerc) override {
        // Display implementation
    }
};

// Create the Display object of type NODISPLAY
NODisplay* pDisplay = new NODisplay();

#endif // NODISPLAY_H_
