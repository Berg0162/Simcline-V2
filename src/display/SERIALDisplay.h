#ifndef SERIALDISPLAY_H_
#define SERIALDISPLAY_H_

#include "IDisplay.h"
static const char* IDISPLAY = "Serial Monitor";

class SERIALDisplay : public IDisplay {
  private:

  public:
    SERIALDisplay() {     
    }

    void initDisplay() override {
        // SERIALDisplay implementation
        Serial.println("Init Display (-> serial monitor)...");
	delay(250);
    }

    void setDisplayBrightness(bool updown) override {
        // SERIALDisplay implementation
        Serial.printf("Set brightness!...[%X]\n", updown);
    }

    void ShowIconsOnTopBar(bool isTrainer, bool isLaptop, bool isPhone) override {
        // SERIALDisplay implementation
        Serial.printf("Trainer: [%X] ", isTrainer);
        Serial.printf("Laptop: [%X] ", isLaptop);
        Serial.printf("Phone: [%X]\n", isPhone);
    }

    void ShowMessageWindow(const String& Line1, const String& Line2, const String& Line3, uint16_t Pause) override {
        // SERIALDisplay implementation
        Serial.printf("%s ", Line1);
        Serial.printf("%s ", Line2);
        Serial.printf("%s\n", Line3);
        if(Pause > 0) vTaskDelay(Pause/portTICK_PERIOD_MS); // Pause indicated time in ms
    }

    void ShowRoadGrade(float gradePerc) override {
        // SERIALDisplay implementation
        Serial.printf("Roadgrade: [%5.1f]\n", gradePerc);
    }
};

// Create the Display object of type Serial Monitor
SERIALDisplay* pDisplay = new SERIALDisplay();

#endif // SERIALDISPLAY_H_
