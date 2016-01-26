/* Turret Control 
    Sebastien Eckersley-Maslin
*/
#ifndef Turret_h
#define Turret_h

    #include "Arduino.h"

	/* Arduino shield PIN declarations */
	#define AZIMUTH_DIR_PIN 4
	#define AZIMUTH_STEP_PIN 5
	#define ELEVATION_DIR_PIN 7
	#define ELEVATION_STEP_PIN 6
	#define AZIMUTH_CUTOUT_PIN 2
	#define ELEVATION_CUTOUT_PIN 3 
	#define FIRE_PIN 8
	#define MOTOR_POWER_PIN 10

	/* Motor controller STEP delay */
	#define AZIMUTH_MOTOR_DELAY 800  //  800 good
	#define ELEVATION_MOTOR_DELAY 1000  // 1000 good

	/* Azimuth and Elevation home positions */
	#define AZIMUTH_HOME 0  
	#define ELEVATION_HOME 2030 
	#define STEPS_PER_DEGREE 53.3333333

	/* Safe to Fire Zone (STFZ) cutouts */
	#define AZ_MAX 0
	#define AZ_MIN -14000
	#define EL_MAX 0
	#define EL_MIN -5000

	#define OFF LOW
	#define ON HIGH

	struct Position 
	{
		int elevation;
		int azimuth;
	};

	class Turret
	{
	public:
		Turret();
		~Turret();
		Position pos;
		void goHome(void);
		void reset(void);
		bool aim(void);
		bool aim(int a, int e);
		bool setTarget(int a, int e);
		void aimAndFire(int a, int e);
		void fire(void);
		void motors(bool mode);
		String toString(void);
	private:
		Position _pos;
		bool _motorPowerState;
		unsigned long _motorLastMove;
		bool _elevate(int delta);
		bool _elevate(int delta, bool override);
		bool _rotate(int delta);
		bool _rotate(int delta, bool override);
		bool _checkElevationTarget(int delta);
		bool _isElevationCutout(void);
		void _setElevationDirection(int delta);
		bool _checkAzimuthTarget(int delta);
		bool _isAzimuthCutout(void);
		void _setAzimuthDirection(int delta);
		void _checkMotors(void);
	};

#endif