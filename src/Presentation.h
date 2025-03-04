#ifndef PRESENTATION_H_
#define PRESENTATION_H_

#include "display/IDisplay.h"

class Presentation {
  private:
    IDisplay* display;
    static Presentation* instance;  // Singleton instance
    Presentation(IDisplay* display);
  public:
    ~Presentation();
    static Presentation* getInstance(IDisplay* display = nullptr);
    void initDisplay();
    void setDisplayBrightness(bool updown);
    void ShowIconsOnTopBar(bool isTrainer, bool isLaptop, bool isPhone);
    void ShowMessageWindow(const String& Line1, const String& Line2, const String& Line3, uint16_t Pause);
    void ShowRoadGrade(float gradePerc);
};
#endif // PRESENTATION_H_
