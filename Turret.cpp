#include "Arduino.h"
#include "Turret.h"

Turret::Turret()
{
	pinMode(AZIMUTH_DIR_PIN, OUTPUT);
	pinMode(AZIMUTH_STEP_PIN, OUTPUT);
	pinMode(ELEVATION_DIR_PIN, OUTPUT);
	pinMode(ELEVATION_STEP_PIN, OUTPUT);

	pinMode(AZIMUTH_CUTOUT_PIN, INPUT);
	pinMode(ELEVATION_CUTOUT_PIN, INPUT);

	pinMode(FIRE_PIN, OUTPUT);
	pinMode(MOTOR_POWER_PIN, OUTPUT);

	digitalWrite(FIRE_PIN, LOW);
	digitalWrite(MOTOR_POWER_PIN, HIGH);

	pos = {0, 0};
	_pos = {0, 0};

	_motorPowerState = false;

  	delay(500);
}


void Turret::goHome(void)
{
	motors(ON);
	reset();

	_elevate(ELEVATION_HOME);
	_rotate(AZIMUTH_HOME);

	motors(OFF);  
}


void Turret::reset(void)
{
	motors(ON);

	while(!_isElevationCutout()) 
		_elevate(-1, true);

	while(!_isAzimuthCutout())
		_rotate(-1, true);

	pos = {0, 0};
	_pos = {0, 0};

	_motorPowerState = true;
}


bool Turret::aim(void)
{
	return _rotate(pos.azimuth - _pos.azimuth) && _elevate(pos.elevation - _pos.elevation);
}


bool Turret::aim(int a, int e) 
{
	setTarget(a, e);
	return aim();	
}


bool Turret::setTarget(int a, int e)
{
	if(_checkElevationTarget(e) && _checkAzimuthTarget(a))
	{
		pos.elevation = e;
		pos.azimuth = a;
		return true;
	}
	else
	{
		return false;
	}
}


void Turret::aimAndFire(int a, int e)
{
	aim(a, e);
	delay(500);
	fire();
}


void Turret::fire(void)
{
	digitalWrite(FIRE_PIN, HIGH);
	delay(250);
	digitalWrite(FIRE_PIN, LOW);
}


bool Turret::_elevate(int delta)
{
	return _elevate(delta, false);
}


bool Turret::_elevate(int delta, bool overrideCutoutProtection)
{
	_checkMotors();

	_setElevationDirection(delta);

	if(!overrideCutoutProtection && _checkElevationTarget(delta))
	{
		return false;
	}

	for(int k = 0; k < abs(delta); k++)
	{
		digitalWrite(ELEVATION_STEP_PIN, LOW);
		delayMicroseconds(2);
		digitalWrite(ELEVATION_STEP_PIN, HIGH);
		delayMicroseconds(ELEVATION_MOTOR_DELAY); 

		if(_isElevationCutout()) return false;
	}

	_pos.elevation += delta;
	return true;
}


bool Turret::_checkElevationTarget(int delta)
{
	return (_pos.elevation + delta > EL_MAX || _pos.elevation + delta < EL_MIN);
}


bool Turret::_isElevationCutout(void)
{
    return digitalRead(ELEVATION_CUTOUT_PIN) == LOW;
}


void Turret::_setElevationDirection(int delta)
{
	if(delta < 0)
	{
		digitalWrite(ELEVATION_DIR_PIN, HIGH);
	}
	else
	{
		digitalWrite(ELEVATION_DIR_PIN, LOW);
	}
	delay(1);
}


bool Turret::_rotate(int delta)
{
	return _rotate(delta, false);
}


bool Turret::_rotate(int delta, bool overrideCutoutProtection)
{
	_checkMotors();

	_setAzimuthDirection(delta);

	if(!overrideCutoutProtection && _checkAzimuthTarget(delta))
	{
		return false;
	}

	for(int k = 0; k < abs(delta); k++)
	{
		digitalWrite(AZIMUTH_STEP_PIN, LOW);
		delayMicroseconds(2);
		digitalWrite(AZIMUTH_STEP_PIN,HIGH);
		delayMicroseconds(AZIMUTH_MOTOR_DELAY); 

		if(_isAzimuthCutout()) return false;
	}

	_pos.azimuth += delta;
	return true;
}

bool Turret::_checkAzimuthTarget(int delta)
{
	return (_pos.elevation + delta > EL_MAX || _pos.elevation + delta < EL_MIN);
}


bool Turret::_isAzimuthCutout(void)
{
    return digitalRead(AZIMUTH_CUTOUT_PIN) == LOW;
}


void Turret::_setAzimuthDirection(int delta)
{
	if(delta < 0)
	{
		digitalWrite(AZIMUTH_DIR_PIN, HIGH);
	}
	else
	{
		digitalWrite(AZIMUTH_DIR_PIN, LOW);
	}
	delay(1);
}

// If the motors have been off, then need to reset before we move.
// This ensures the system is in a known position state and avoids
// issues where the turret was moved manually whenthe motors were disengaged.
// It is assumed that when the motors are active that they can hold the system
// in place.
void Turret::_checkMotors()
{
	if(!_motorPowerState) reset();
}

void Turret::motors(bool mode)
{
	digitalWrite(MOTOR_POWER_PIN, mode);
	if(!mode) _motorPowerState = false;
}


String Turret::toString(void)
{
	String str = "Azimuth: ";
	str.concat(_pos.azimuth);
	str.concat(" Elevation: ");
	str.concat(_pos.elevation);
	str.concat(" Weapon: ");
	str.concat(_motorPowerState);
	return str;
}


Turret::~Turret(){} // Nothing to destruct






