////////////////////////////////////////////////////////////////////////////
//
//  This file is part of RTIMULib
//
//  Copyright (c) 2014-2015, richards-tech, LLC
//
//  Permission is hereby granted, free of charge, to any person obtaining a copy of
//  this software and associated documentation files (the "Software"), to deal in
//  the Software without restriction, including without limitation the rights to use,
//  copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the
//  Software, and to permit persons to whom the Software is furnished to do so,
//  subject to the following conditions:
//
//  The above copyright notice and this permission notice shall be included in all
//  copies or substantial portions of the Software.
//
//  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
//  INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
//  PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
//  HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
//  OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
//  SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.


#include "RTIMULib.h"
#include <queue>
#include <iostream>
#include <list>
#include <time.h>

using namespace std;

int main()
{
    int sampleCount = 0;
    int sampleRate = 0;
    uint64_t rateTimer;
    uint64_t displayTimer;
    uint64_t now;


    //  using RTIMULib here allows it to use the .ini file generated by RTIMULibDemo.

    RTIMUSettings * settings = new RTIMUSettings("RTIMULib");

    RTIMU * imu = RTIMU::createIMU(settings);
    RTPressure * pressure = RTPressure::createPressure(settings);
    RTHumidity * humidity = RTHumidity::createHumidity(settings);

   
    
    
    //Create counter for determining if 10 trials have been reached
    int count = 0;
    
    //Create boolean to see if we had reached 10 values yet for averaging
    bool areYouOnTenYet = false;
    
    //SET PRESSURE VARIABLES
    list<float> pressureBuffer;
    float pressureSum = 0;
    float pressureAverage = 0;
    
    //SET HUMIDITY VARIABLES
    list<float> humidityBuffer;
    float humiditySum = 0;
    float humidityAverage = 0;
    
    //SET TEMPERATURE VARIABLES
    list<float> temperatureBuffer;
    float temperatureSum = 0;
    float temperatureAverage = 0;
    
    //SET GYROSCOPE VARIABLES
    list <float> rollBuffer;
    list <float> pitchBuffer;
    list <float> yawBuffer;
    float rollSum = 0;
    float pitchSum = 0;
    float yawSum = 0;
    float rollAverage = 0;
    float pitchAverage = 0;
    float yawAverage = 0;
    
    
    if ((imu == NULL) || (imu->IMUType() == RTIMU_TYPE_NULL)) 
	{
        printf("No IMU found\n");
        exit(1);
    }

    //  This is an opportunity to manually override any settings before the call IMUInit

    //  set up IMU

    imu->IMUInit();

    //  this is a convenient place to change fusion parameters

    imu->setSlerpPower(0.02);
    imu->setGyroEnable(true);
    imu->setAccelEnable(true);
    imu->setCompassEnable(true);

    //  set up pressure sensor

    if (pressure != NULL)
    {
        pressure->pressureInit();
    }

    //  set up humidity sensor

    if (humidity != NULL)
    {
        humidity->humidityInit();
    }

    //  set up for rate timer

    rateTimer = displayTimer = RTMath::currentUSecsSinceEpoch();

    //  now just process data

    while (1) 
	{
		/* HINT: The whole purpose of the weird way this is written is to poll the 
		 *       IMU at its recommended rate, but display the data at a rate that may be different.
         *		 Not polling at the correct rate can result in weird readings.
		 *
		 **/
		
        //  poll at the rate recommended by the IMU
		
        usleep(imu->IMUGetPollInterval() * 1000);

        while (imu->IMURead()) 
		{
            RTIMU_DATA imuData = imu->getIMUData();

            //  add the pressure data to the structure

            if (pressure != NULL)
            {
                pressure->pressureRead(imuData);
            }

            //  add the humidity data to the structure

            if (humidity != NULL)
            {
                humidity->humidityRead(imuData);
            }
            
            RTFLOAT roll = imuData.fusionPose.x() * RTMATH_RAD_TO_DEGREE;
            RTFLOAT pitch = imuData.fusionPose.y() * RTMATH_RAD_TO_DEGREE;
            RTFLOAT yaw = imuData.fusionPose.z() * RTMATH_RAD_TO_DEGREE;
            
            sampleCount++;

            now = RTMath::currentUSecsSinceEpoch();

            //  display 1 time per second
			//  HINT: The code inside this "if" statement executes once per second.
			//        Put all of your code to print/calculate current and average IMU data inside here.
			//        You shouldn't need to modify anything outside of this "if" statement.
			//        The IMU is polled at the recommended rate, but this "if" statement ensures that the  
            //        values are only displayed at a certain interval.			
            if ((now - displayTimer) > 1000000) 
			{ 
            
                cout<< "CURRENT DATA FROM SENSE HAT TRIAL: "<< count << endl;
                cout << "----------------------------------------------"<<endl;
                printf("Sample rate %d: %s\n", sampleRate, RTMath::displayDegrees("", imuData.fusionPose));

                if (pressure != NULL) 
				{
                    printf("Pressure: %4.1f, height above sea level: %4.1f, temperature: %4.1f",
                           imuData.pressure, RTMath::convertPressureToHeight(imuData.pressure), imuData.temperature);
                }
                
                //ADD VALUES TO THE LIST**************************
                pressureBuffer.push_front(imuData.pressure);
				humidityBuffer.push_front(imuData.humidity);
                temperatureBuffer.push_front(imuData.temperature);
                
                rollBuffer.push_front(roll);
                pitchBuffer.push_front(pitch);
                yawBuffer.push_front(yaw);
                //************************************************
                
                if (humidity != NULL) 
				{
                    printf(", humidity: %4.1f",
                           imuData.humidity);
                }
                printf("\n");
                cout << "----------------------------------------------"<<endl;
                printf("\n");
                
            
                /* UNCOMMENT IF YOU WANT TO SEE EACH VALUE IN THE CURRENT 10 VALUE BUFFER PER ITERATION
                cout << "Pressure Readings:  " << endl;
                * 
                for (auto iterator = pressureBuffer.begin(); iterator != pressureBuffer.end(); iterator ++ ) {
                    cout << "Pressure Reading " << sampleNumber<< ": " << *iterator << endl;
                    sampleNumber++;
                }
                */
                
                //KEEP THE BUFFERS TO ONLY THE LATEST 10 READINGS
                if (pressureBuffer.size() > 10) {
                    pressureBuffer.pop_back();
                }
                if (humidityBuffer.size() > 10) {
                    humidityBuffer.pop_back();
                }
                if (temperatureBuffer.size() > 10) {
                    temperatureBuffer.pop_back();
                }
                if (rollBuffer.size() > 10) {
                    rollBuffer.pop_back();
                }
                if (pitchBuffer.size() > 10) {
                    pitchBuffer.pop_back();
                }
                if (yawBuffer.size() > 10) {
                    yawBuffer.pop_back();
                }
                
                //*************CALCULATE PRESSURE AVERAGE FOR LAST 10 READINGS**************************************
                for (auto iterator = pressureBuffer.begin(); iterator != pressureBuffer.end(); iterator ++ ) {
                    pressureSum+= *iterator;
                    
                }
                if (areYouOnTenYet) {
                    pressureAverage = pressureSum/10; //CALCULATE AVERAGE
                    cout << "Average pressure of last 10 trials: " << pressureAverage <<endl;
                    
                
                }
                if (areYouOnTenYet == false) {
                    cout << "Not enough data for average yet, please standby until 10 seconds" << endl;
                }
                pressureSum = 0; //RESET PRESSURE SUM FOR NEXT BUFFER
                printf("\n");
                
                //*********************************************************************************************
                
                //*************CALCULATE HUMIDITY AVERAGE FOR LAST 10 READINGS**************************************
                for (auto iterator = humidityBuffer.begin(); iterator != humidityBuffer.end(); iterator ++ ) {
                    humiditySum+= *iterator;
                }
        
                if (areYouOnTenYet) {
                    humidityAverage = humiditySum/10; //CALCULATE AVERAGE
                    cout << "Average humidity of last 10 trials: " << humidityAverage <<endl;
                
                }
                
                humiditySum = 0; //RESET HUMIDITY SUM FOR NEXT BUFFER
                printf("\n");
                
                //*********************************************************************************************
                
                //*************CALCULATE TEMPERATURE AVERAGE FOR LAST 10 READINGS**************************************
                for (auto iterator = temperatureBuffer.begin(); iterator != temperatureBuffer.end(); iterator ++ ) {
                    temperatureSum+= *iterator;
                }
        
                if (areYouOnTenYet) {
                    temperatureAverage = temperatureSum/10; //CALCULATE AVERAGE
                    cout << "Average temperature of last 10 trials: " << temperatureAverage <<endl;
                
                }
                
                temperatureSum = 0; //RESET TEMPERATURE SUM FOR NEXT BUFFER
                printf("\n");
                
                //*********************************************************************************************
                
                //ROLL PITCH AND YAW
                //*************CALCULATE IMU DATA AVERAGE FOR LAST 10 READINGS**************************************
                
               
                for (auto iterator = rollBuffer.begin(); iterator != rollBuffer.end(); iterator ++ ) {
                    rollSum+= *iterator;
                }
                for (auto iterator = pitchBuffer.begin(); iterator != pitchBuffer.end(); iterator ++ ) {
                    pitchSum+= *iterator;
                }
                for (auto iterator = yawBuffer.begin(); iterator != yawBuffer.end(); iterator ++ ) {
                    yawSum+= *iterator;
                }
        
                if (areYouOnTenYet) {
                    rollAverage = rollSum/10; //CALCULATE AVERAGE
                    pitchAverage = pitchSum/10; //CALCULATE AVERAGE
                    yawAverage = yawSum/10; //CALCULATE AVERAGE
                    
                    cout << "Average IMU Readings of last 10 trials" <<endl;
                    cout << "Roll: " << rollAverage << endl;
                    cout << "Pitch: " << pitchAverage << endl;
                    cout << "Yaw: " << yawAverage << endl;
                }
                
                
                rollSum = 0; //RESET SUM FOR NEXT BUFFER
                pitchSum = 0; //RESET SUM FOR NEXT BUFFER
                yawSum = 0; //RESET SUM FOR NEXT BUFFER
                printf("\n");
                
                //*********************************************************************************************
                
                
                
                
                //ADD TO TRIAL COUNTER
                count++;
                
                //CHECK IF WE HAVE REACHED 10 TRIALS TO START CALCULATING AVERAGE
                if (count >=10) {
                    areYouOnTenYet = true;
                }
                fflush(stdout);
                
                
                
                
                displayTimer = now;//HINT: LEAVE THIS LINE ALONE!!!
                
            }

            //  update rate every second

            if ((now - rateTimer) > 1000000) 
			{
                sampleRate = sampleCount;
                sampleCount = 0;
                rateTimer = now;
            }
        }
    }
}

