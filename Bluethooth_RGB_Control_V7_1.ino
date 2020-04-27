#include <SoftwareSerial.h>
#include <FastLED.h>
#include <SPI.h>
#include <SD.h>

const int confArr[][3] = {{48}, {3}, {200, 0, 0}, {0, 0, 200}, {0, 200, 0}}; // Configuration array: number of LEDs, number of colors, colors
const int numOfLeds = confArr[0][0];                                         // Sets the number of LEDs from the configuration array
const int ledSignalPin = 11;                                                 // Signal pin for LED matrix
const int chipSelect = 53;                                                   // Pin number for the SD card chip select pin
int ledArr[numOfLeds][2];                                                    // Array for storing the LEDs being turned on, and the color they use, first ellement is the LED posstion, second ellement is LED deafult color
int ledCounter = 0;                                                          // Counts the number of LEDs turned on in one command
String boulders[20];                                                         // Array for storing boulders from the SD card

File myFile;

CRGB leds[numOfLeds]; //Sets's up the RGB LEDs

void setup()
{

  Serial.begin(9600);  //Initates serial cmunication with the PC
  Serial1.begin(9600); //Initates serial cmunication with the bluethooth module
  while (!Serial)
  {
    // wait for serial port to connect.
  }
  FastLED.addLeds<WS2812, ledSignalPin, GRB>(leds, numOfLeds); //Initates LEDs

  resetLeds();
  initializeSDCard();

  myFile = SD.open("/");
  if (!myFile)
  {
    Serial.println("No myFile");
  }
  else
  {
    Serial.println("All good");
  }

  readFileNames(myFile, boulders);
  Serial.println("Setup finished");
}

void loop()
{

  if (Serial1.available())
  {

    switch (Serial1.read())
    {

    case 76: //Poslati L i redne brojeve ledioda koje treba upaliti, LL C L C L C ... (L = LED possition, C = LED default color)
      Serial.println("Entered case 1");
      serialReadToArr(ledArr);
      controlLeds(ledArr, confArr, sizeof(ledArr));
      break;

    case 82: //Slanje R gasi sve lediode
      Serial.println("Entered case 2");
      resetLeds();
      break;

    case 83:
      Serial.println("Entered case 3");
      while (!Serial1.available())
      {
      };
      int fileIndex = Serial1.read();
      readSDToArr(ledArr, boulders, 5, fileIndex);
      printArr(ledArr, ledCounter);
      controlLeds(ledArr, confArr, sizeof(ledArr));
      break;
    };
  };
}

void resetLeds()
{ //Resets all LEDs

  for (int i = 0; i < numOfLeds; i++)
  {
    leds[i] = CRGB(0, 0, 0);
  };
  FastLED.show();
}

void controlLeds(int infArr[][2], int colorArr[][3], int infArrSize) //Turns on LEDs specified in the ledArr array
{

  resetLeds();
  Serial.println("Entered controlLeds function");

  for (int i = 0; i < ledCounter; i++)
  {
    leds[infArr[i][0]] = CRGB(colorArr[infArr[i][1] + 1][0], colorArr[infArr[i][1] + 1][1], colorArr[infArr[i][1] + 1][2]);
  };

  FastLED.show();
  memset(infArr, 0, infArrSize);
  ledCounter = 0;
}

void serialReadToArr(int fillArr[][2]) //Reads data from bluethooth module and stores them in a two dimensional array of an x by 2 size (only accepts numbers as high as the number of LEDs)
{

  Serial.println("Entered serialReadToArr function");
  Serial.println("");

  byte currValue = 0;         // Holds value of the current recieved byte
  byte tmpStr[3] = {0, 0, 0}; // Temporerly holds recieved values until mergied in one number
  int digitCount = 0;         // Counts the number of digits in a recieved number
  bool typeIndicator = false; // False when recieveing LED possitions, true when recieveing LED color

  do
  {
    while (!Serial1.available()) // Make sure there is available data
    {
    };
    currValue = Serial1.read();     // Reads next Value
    tmpStr[digitCount] = currValue; // Puts the values on hold
    //Serial.println(currValue);

    if (tmpStr[digitCount] == ' ' || tmpStr[digitCount] == '\n') // Check if whole number was recieved
    {
      for (int i = 0; i < digitCount; i++) // Convert tmpStr to a number
      {
        fillArr[ledCounter][typeIndicator] = fillArr[ledCounter][typeIndicator] + (tmpStr[i] - 48) * (pow(10, digitCount - (i + 1)));
        //Serial.println("fillArr[ledCounter][typeIndicator]");
        //Serial.println(fillArr[ledCounter][typeIndicator]);
      }
      typeIndicator = !typeIndicator;

      if (!typeIndicator)
      {
        ledCounter++;
      };
      digitCount = 0;
    }
    else
    {
      digitCount++;
    }
  } while (currValue != '\n');
  //printArr(fillArr, 49);
}

void printArr(int inArr[][2], int arrLen) //Prints the values stored in a two dimensional array of size x by 2
{

  Serial.println("Entered printArr function");

  for (int i = 0; i <= arrLen - 1; i++)
  {

    Serial.print(inArr[i][0]);
    inArr[i][0] < 10 ? Serial.print("    ") : Serial.print("   ");
    Serial.println(inArr[i][1]);
  }
}

void initializeSDCard()
{
  Serial.println("Initializing SD card...");
  if (!SD.begin(chipSelect))
  {
    Serial.println("initialization failed!");
  }
  else
  {
    Serial.println("initialization done.");
  }
}

void readFileNames(File dir, String filesArr[])
{
  int fileCounter = 0;

  while (true)
  {
    File entry = dir.openNextFile();
    if (!entry)
    {
      // no more files
      break;
    }
    if (fileCounter)
    {
      filesArr[fileCounter - 1] = entry.name();
      Serial.println(filesArr[fileCounter - 1]);
    }
    fileCounter++;
    entry.close();
  }

  dir.close();
}

void readSDToArr(int fillArr[][2], String filesArr[], int arrLen, int fileNum) //Reads data from bluethooth module and stores them in a two dimensional array of an x by 2 size (only accepts numbers as high as the number of LEDs)
{

  Serial.println("Entered readSDToArr function");
  Serial.println("");

  char tmpStr[3] = {0, 0, 0}; // Temporerly holds recieved values until mergied in on
  int digitCount = 0;         // Counts the number of digits in a recieved number
  bool typeIndicator = false; //false when recieveing LED possitions, true when recieveing LED color

  File currentFile = SD.open(filesArr[fileNum - 48]); // Open the specified file

  if (currentFile) // Check if the file was succefully opened
  {
    Serial.println("Current file good");
  }
  else
  {
    Serial.println("Problem with current file");
  }

  do
  {
    tmpStr[digitCount] = currentFile.read(); // Get the next byte from the file

    if (tmpStr[digitCount] == ' ' || tmpStr[digitCount] == '\r' || tmpStr[digitCount] == '\n' || tmpStr[digitCount] < 0) // Check if end of number
    {

      for (int i = digitCount - 1; i >= 0; i--) // Convert the digits in tmpStr and save them
      {
        fillArr[ledCounter][typeIndicator] = fillArr[ledCounter][typeIndicator] + ((int)tmpStr[i] - 48) * (pow(10, (digitCount - (i + 1))));
      }
      typeIndicator = !typeIndicator;

      if (!typeIndicator)
      {
        ledCounter++;
      };

      digitCount = 0;
    }
    else
    {
      digitCount++;
    }
  } while (tmpStr[digitCount] >= 0); // Check if end of file

  currentFile.close(); // Close the file
}