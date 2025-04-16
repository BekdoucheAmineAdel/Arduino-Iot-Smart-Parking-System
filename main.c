bool isSubscriber() {
    // First, we get the number of vehicles that have registered
    int numofCars = Firebase.getInt(AppData, "ParkingIot/AppData/numberOfCars");
    // We obtain the last car plate to compare it with our list
    int carPlateCam = Firebase.getInt(CamData, "ParkingIot/CamData/carPlate" + String(f_ileCounter));
    bool subscriber = false;
    // Here we start the comparison plate by plate, normally this would take a lot of time so it was used after the gate closure
    for (int i = 0; i < numofCars; i++) {
        int carPlateApp = Firebase.getInt(AppData, "ParkingIot/AppData/carPlate" + String(i));
        if (carPlateApp == carPlateCam) {
            f_ileCounter += 1;
            subscriber = true;
            break;
        }
    }
    return subscriber;
}

void CloseGateCycle(Servo s, bool showMsg, bool &openBar, String msg, int &places, int Places[], int Sensors[], int numOfSensors, int decValue, int d) {
    int t = 0;
    while (t < 5) {
        // If our closure sensor detects something, then we close the barrier
        if (checkSingleSensor(Sensors, d) == 1) {
            // Turn the motor
            s.write(90);
            places -= decValue;
            // Clear the important parts of the LCD
            LCD_SetAndPrint(blank16, 0, 0);
            LCD_SetAndPrint(blank4, 4, 1);
            LCD_SetAndPrint(blank4, 12, 0);
            // The barrier is closed
            openBar = false;
            // Exit
            break;
        } else {
            // Display the entry message if needed
            if (showMsg == true)
                DisplayMessage(msg, Places, Sensors, 3, 1, 200, d);
            // Increment the counter
            t++;
            // Divide the sensor list into a small list of two sensors to check if our system has other important things to handle
            int Subsensor;
            for (int i = 1; i < 3; i++)
                Subsensor[i] = Sensors[i + 1];
            if (checkMultipleSensors(Subsensor, 2, 0.1 * d) == 1) {
                LCD_SetAndPrint(blank16, 0, 0);
                openBar = true;
                break;
            }
        }
        if (t == 5)
            openBar = true;
    }
}

void OpenGateCycle(Servo s, bool showMsg, bool &openBar, String msg, int &places, int Places[], int Sensors[], int numOfSensors, int decValue, int d) {
    // Check if the barrier is already open
    if (openBar == false) {
        // Test the opening sensor
        if (checkSingleSensor(Sensors, d) == 1) {
            // Open the barrier
            s.write(-90);
            // Trigger the closure process
            CloseGateCycle(s, showMsg, openBar, msg, places, Places, Sensors, numOfSensors, decValue, d);
        }
    } else
        CloseGateCycle(s, showMsg, openBar, msg, places, Places, Sensors, numOfSensors, decValue, d);
}

void Entry(Servo s, bool &openBar, int &Subscribers, bool mainEntry, String entryMsg, String fullMsg, int places[], int mPlaces, int Sensors[], int numOfSensors, int d) {
    if (mainEntry == true) {
        // This three represents the number of additional places
        if (places + places + 3 > 0) {
            // Trigger the opening process
            OpenGateCycle(s, true, openBar, entryMsg, places, places, Sensors, numOfSensors, 1, d);
            int p1 = places;
            // If the number of places decreases, then we test if the driver is a subscriber
            if (places == p1 - 1)
                // Check if the car has reserved or not
                Subscribers += isSubscriber();
        } else if (checkSingleSensor(Sensors, d) == 1)
            DisplayMessage(fullMsg, places, Sensors, numOfSensors, 1, 200, d);
    } else if (places > 0 && places < mPlaces && Subscribers > 0) {
        OpenGateCycle(s, false, openBar, entryMsg, places, places, Sensors, numOfSensors, 1, d);
        int p0 = places;
        // If the number of subscriber places decreases, then our subscriber counter decreases
        if (places == p0 - 1)
            Subscribers -= 1;
    }
}

void Exit(Servo s, bool &openBar, String exitMsg, int places[], int mPlaces[], int &exitCounter, int Sensors[], int numOfSensors, int d) {
    if (exitCounter == 0) {
        if (places < mPlaces) {
            OpenGateCycle(s, false, openBar, exitMsg, places, places, Sensors, numOfSensors, -1, d);
        }
    } else {
        // Ensure that the places have decreased to decrease the counter
        int tplaces = places;
        if (places < mPlaces) {
            OpenGateCycle(s, false, openBar, exitMsg, places, places, Sensors, numOfSensors, -1, d);
            if (places == tplaces + 1)
                exitCounter -= 1;
        }
    }
}

void Exit_Counter(bool &flag, int places, int mPlaces, int &exitCounter, int Sensor, int LEDs[], int d) {
    if (checkSingleSensor(Sensor, d) == 1 && places + exitCounter < mPlaces) {
        flag = true;
        RG_LED_Set(LEDs, LOW, HIGH);
    } else if (checkSingleSensor(Sensor, 0.1 * d) == 0 && places + exitCounter < mPlaces) {
        if (flag == true) {
            flag = false;
            exitCounter += 1;
        }
        RG_LED_Set(LEDs, HIGH, LOW);
    } else
        RG_LED_Set(LEDs, LOW, LOW);
}

void Exit_Return(bool &flag, int &places, int Sensor, int d) {
    if (checkSingleSensor(Sensor, d) == 1)
        flag = true;
    if (checkSingleSensor(Sensor, 0.1 * d) == 0 && flag == true) {
        flag = false;
        places++;
    }
}

void setup() {
    for (int i = 0; i < 2; i++)
        o_ldPlaces[i] = P_laces[i] = m_axPlaces[i];
    for (int i = 0; i < 3; i++)
        B_ars[i] = false;
    e_xitCounter = 0; a_dvCounter = 0; s_ubCounter = 0;
    flag0 = false; flag1 = false; blinker = true;
    timer0 = 0; timer1 = 7200;
    for (int i = 0; i < 8; i++)
        pinMode(S_ensors[i], INPUT);
    pinMode(l_eds, OUTPUT); pinMode(l_eds, OUTPUT);
    s0.attach(10); s1.attach(11); s2.attach(12);
    lcd.begin(16, 2);
    lcd.createChar(1, carChar0); lcd.createChar(2, carChar1); lcd.createChar(3, carChar2);
    lcd.createChar(4, smokeChar);
    lcd.clear();
}
