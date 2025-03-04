#ifndef YOURDISPLAY_H_
#define YOURDISPLAY_H_

#include "IDisplay.h"
static const char* IDISPLAY = "Your Display";

class YOURDisplay : public IDisplay {
  private:

  public:
    YOURDisplay() {     
    }

    void initDisplay() override {
        // SERIALDisplay implementation for your display
        Serial.println("Init Display (-> serial monitor)...");
	delay(250);
    }

    void setDisplayBrightness(bool updown) override {
        // SERIALDisplay implementation for your display
        Serial.printf("Set brightness!...[%X]\n", updown);
    }

    void ShowIconsOnTopBar(bool isTrainer, bool isLaptop, bool isPhone) override {
        // SERIALDisplay implementation for your display
        Serial.printf("Trainer: [%X] ", isTrainer);
        Serial.printf("Laptop: [%X] ", isLaptop);
        Serial.printf("Phone: [%X]\n", isPhone);
    }

    void ShowMessageWindow(const String& Line1, const String& Line2, const String& Line3, uint16_t Pause) override {
        // SERIALDisplay implementation for your display
        Serial.printf("%s ", Line1);
        Serial.printf("%s ", Line2);
        Serial.printf("%s\n", Line3);
        if(Pause > 0) vTaskDelay(Pause/portTICK_PERIOD_MS); // Pause indicated time in ms
    }

    void ShowRoadGrade(float gradePerc) override {
        // SERIALDisplay implementation for your display
        Serial.printf("Roadgrade: [%5.1f]\n", gradePerc);
    }
};

// Create the Display object of type YOURDisplay
YOURDisplay* pDisplay = new YOURDisplay();

#endif // YOURDISPLAY_H_
