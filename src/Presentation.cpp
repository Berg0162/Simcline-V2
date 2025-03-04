
#include "Presentation.h"

  // Initialize the static instance pointer to nullptr
  Presentation* Presentation::instance = nullptr;

  //Constructor 
  Presentation::Presentation(IDisplay* display) : display(display) {}

  //Destructor
  Presentation::~Presentation() {}

  // Singleton access method with optional argument
  Presentation* Presentation::getInstance(IDisplay* display) {
      if (instance == nullptr) {
         if (display == nullptr) {
            // Error handling if no display is provided on first call
            // You can throw an exception or handle it another way
         }
         instance = new Presentation(display);  // Create the instance with the dependency
      }
      return instance;
  }

  void Presentation::initDisplay() {
      display->initDisplay();
  }

  void Presentation::setDisplayBrightness(bool updown) {
      display->setDisplayBrightness(updown);
  }

  void Presentation::ShowIconsOnTopBar(bool isTrainer, bool isLaptop, bool isPhone) {
      display->ShowIconsOnTopBar(isTrainer, isLaptop, isPhone);
  }

  void Presentation::ShowMessageWindow(const String& Line1, const String& Line2, const String& Line3, uint16_t Pause) {
      display->ShowMessageWindow(Line1, Line2, Line3, Pause);
  }

  void Presentation::ShowRoadGrade(float gradePerc) {
      display->ShowRoadGrade(gradePerc);
  }

