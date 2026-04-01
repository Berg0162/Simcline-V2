# Changelog

All notable changes to this project will be documented in this file.
## [1.1.1] - 2026-01-04
### Fixed
- Legacy trainer **Zwift Hub One** is using a deviating ZVS protocol.
- Added (auto-detect) support for this one-off **Hub One ZVS protocol**!
## [1.1.0] - 2026-01-03
### Fixed

### Changed
- Documentation regarding ZVS integration
### Added
Added code support for the **Zwift Virtual Shifting** BLE Service.
- New class added *VirtualShifting* (ZVS) and integrated with Server- and Client-side
- **ENABLE_ZVS** selection added in `src/config/configNimBLE.h`
- Added explanation and instructions `docs/Setup_Zwift_Virtual_Shifting.md`
 
## [1.0.4] - 2025-07-20
### Fixed
 - NimBLE-Arduino turned out to be vulnerable to _race conditions_ if both client and server initiate the same GATT procedure.
BLE peripherals/trainers expect the client to initiate MTU exchange at connection time, however some (like **Tacx Neo 2T**, with Zwift-oriented firmware update) initiate it proactively! Fixed this (potential) connection conflict in `ClientSide` class and inserted more robust `updateConnParameters` handling in `ServerSide` class.
 - Improved the presentation of some setup settings (from `configNimBLE.h`) in MITM/Simcline code Examples
### Changed

### Added

## [1.0.3] - 2025-06-11
### Fixed

### Changed

### Added
Added support for Legacy Tacx Smart Trainers with only proprietary **(ANT+) FE-C over BLE protocol**
- New class added for FitnessEquipmentCycling (FEC) and integrated with Server- and Client-side
- ENABLE_TACXFEC selection added in `src/config/configNimBLE.h`

## [1.0.2] - 2025-05-29
### Fixed
Since the release of NimBLE-Arduio 2.3.0, the compiler complained about const-type of pChar->readValue(),
as a consequence `const NimBLERemoteCharacteristic*` was changed to a non-const pointer.
### Changed

### Added
Support for XIAO ESP32S3 has been documented
- Added docs/XIAO_ESP32S3_Sense.md plus supporting images

## [1.0.1] - 2025-03-16
### Fixed

### Changed
Revisited and updated Lifter Class
- More reliable Actuator and VL8106X testing
- Optional selection of SMA and EMA filter for range readings
- Improved responsiveness of Actuator control

Updated Test_Board_plus_VL6180X
- Incorporated use of Lifter Class

### Added

## [1.0.0] - 2025-03-04
### Fixed

### Changed

### Added
- Initial release with core functionality for simulating indoor bike lifter operations.
- Complete Arduino library structure, including `src/` and `examples/` directories.
- Comprehensive `README.md` with installation and usage instructions.
- Additional detailed setup and installation instructions in `docs/` directory.
- Initial Android companion app v5.2.2 (`Android/SimclineApp.aia` and `SimclineApp.apk`).

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/), 
and this project adheres to [Semantic Versioning](https://semver.org/).


