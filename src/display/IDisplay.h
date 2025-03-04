#ifndef IDISPLAY_H_
#define IDISPLAY_H_

#include <Arduino.h>

class IDisplay {
  public:
    virtual ~IDisplay() = default;

    virtual void initDisplay() = 0;
    virtual void setDisplayBrightness(bool updown) = 0;
    virtual void ShowIconsOnTopBar(bool isTrainer, bool isLaptop, bool isPhone) = 0;
    virtual void ShowMessageWindow(const String& Line1, const String& Line2, const String& Line3, uint16_t Pause) = 0;
    virtual void ShowRoadGrade(float gradePerc) = 0;
};

#endif // IDISPLAY_H_
