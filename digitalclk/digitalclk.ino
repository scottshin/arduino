//
//  Digital Clock 
//

#include <Adafruit_GFX.h>   // Core graphics library
#include <RGBmatrixPanel.h> // Hardware-specific library
                                          
#include <SPI.h>
#include <SD.h>
#include <SoftwareSerial.h>

#include <Wire.h>
#include <Time.h>
#include <DS1307RTC.h>

#include <EEPROM.h>


#define HC06 Serial3
File myFile;

uint8_t  img24[32*32*3];

tmElements_t tm;
tmElements_t tm_write;
int write_flag = 0;



// Similar to F(), but for PROGMEM string pointers rather than literals
#define F2(progmem_ptr) (const __FlashStringHelper *)progmem_ptr

#define SD_CS    (53)    // 7

#define CLK      (11)  // MUST be on PORTB! (Use pin 11 on Mega)
#define LAT      (10)
#define OE       (9)
#define A   (A0)
#define B   (A1)
#define C   (A2)
#define D   (A3)

//
RGBmatrixPanel matrix( A, B, C, D, CLK, LAT, OE, true, 32);

// Double-buffered mode consumes nearly all the RAM available on the
// Arduino Uno -- only a handful of free bytes remain.  Even the
// following string needs to go in PROGMEM:

const char str[] PROGMEM = "QUARTERNOETE 64x32 RGB LED Matrix With BLE module";
int    textX   = matrix.width(),
       textMin = sizeof(str) * -12,
       hue     = 0;
int8_t ball[3][4] = {
  {  3,  0,  1,  1 }, // Initial X,Y pos & velocity for 3 bouncy balls
  { 17, 15,  1, -1 },
  { 27,  4, -1,  1 }
};


#define GREEN     0
#define BLUE      1
#define RED       2
#define WHITE     3
#define GRAY      4
#define YELLOW    5

static const uint16_t PROGMEM ballcolor[6] = {
  0x0080, // Green=1
  0x0002,  // Blue=1
  0x1000,  // Red=1
  0x1082,  // white
   
  0x1052,   // GRAY

  0x1080  // YELLOW
};

struct PIXEL {
  int x;
  int y;
};

PIXEL pxHour;
PIXEL pxMin;


int nRotation = 0;

void processBMPFile( char * szFile )
{
  File myFile = SD.open(szFile, FILE_READ);
  int inx = 0;
  if ( myFile == false)
  {
        matrix.print("open err");
        Serial.println("No SD card");
    
        return;
  
  }
  else
  {
    Serial.println("read start...");
    if ( myFile.available())
         for ( int i  = 0; i < 0x37 -1; i++)
              myFile.read();


   int line = 0;
    while ( myFile.available() ) 
    {
          unsigned char b = myFile.read();
          unsigned char g = myFile.read();
          unsigned char r = myFile.read() ;          


        line += 3;
        if ( line >= 96 )   // skip  over 32pixel 
        {
          // myFile.read();
            //  myFile.read();
            //  myFile.read();
                       
          line = 0;
        }
 
        img24[ inx    ] = r;
        img24[ inx +1 ] = g;
        img24[ inx +2 ] = b;
        inx += 3;

        if ( inx >= 32*32*3 )
            goto EX;
      
    }
 }
EX:
    myFile.close();
    Serial.println("read ok...");

}



void setup() 
{
  pxHour.x = 0;
  pxHour.y = 0;
  pxMin.x = 0;
  pxMin.y = 0;

  // Open serial communications and wait for port to open:
  Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for Leonardo only
  }

  Serial.println("program start!");
  
  HC06.begin(9600); // set the data rate for the BT port


  Serial.println("bluetooth send on!");
  HC06.write("on!!");
  HC06.write("on!!");
  HC06.write("on!!");
  HC06.write("df n!!\n");
  
  Serial.println("Initializing SD card...");

  if (!SD.begin(SD_CS)) {
    Serial.println("initialization failed!");
  }
  Serial.println("initialization done.");
  // so you have to close this one before opening another.
  myFile = SD.open("test.txt", FILE_WRITE);
  // if the file opened okay, write to it:
    if (myFile) {
        Serial.print("Writing to test.txt...");
        myFile.println("testing 1, 2, 3.");
    // close the file:
    myFile.close();
    Serial.println("done.");
    } else {
        // if the file didn't open, print an error:
        Serial.println("error opening test.txt");
    }

  //
  //
  //
    matrix.begin();
    matrix.setTextWrap(false); // Allow text to run off right edge

    
  nRotation =3;// EEPROM.read( 0);
  matrix.setRotation( nRotation );

  #if 1
  attachInterrupt( digitalPinToInterrupt(2), mode_isr0, FALLING);  
  attachInterrupt(digitalPinToInterrupt(3), mode_isr1, FALLING);   // 1 is digital(3)
#endif



    processBMPFile("/clock/bg.bmp");
}



void mode_isr0()
{
    Serial.println("rotation....");
 
    nRotation = ++nRotation %4;
    matrix.setRotation( nRotation );
      
    EEPROM.write( 0, nRotation );
  
}
void mode_isr1()
{
  // set_time



  tm_write = tm;
  


    if ( tm_write.Minute < 55 )
    {
      tm_write.Minute += 5;
    }
    else
    {
      tm_write.Hour = (++tm_write.Hour) %24;
      tm_write.Minute = 0;
    }
  //   RTC.write(tm);


    write_flag = 1;

         Serial.println("time ....");
              Serial.println(tm_write.Hour);
           Serial.println(tm_write.Minute);

 
}




void drawShape(int x, int y, int sharp)
{
    switch ( sharp )
    {
      case 0:    //[][][][]
          matrix.drawLine( x, y, x, y-3, matrix.Color888(255,0,0) );
          break;
      case 1:   // [][]
                // [][]
          matrix.drawRect( x, y, 2, 2, matrix.Color888(0,0,255) );
          break;

      case 2:   // [][][]
                //   []
          matrix.drawLine( x, y, x+2, y, matrix.Color888(255,255,0) );
           matrix.drawPixel( x+1, y+1, matrix.Color888(255,255,0) );
          break;

     case 3:   // [][][]
               // []
          matrix.drawLine( x, y, x+2, y, matrix.Color888(0,255,0) );
           matrix.drawPixel( x, y+1, matrix.Color888(0,255,0) );
          break;

          
     case 4:   // [][][]
               //     []
          matrix.drawLine( x, y, x+2, y, matrix.Color888(0,255,255) );
           matrix.drawPixel( x+1, y+1, matrix.Color888(0,255,255) );
          break;

          
     case 5:   // [][]
               //   [][]
          matrix.drawLine( x, y, x+1, y, matrix.Color888(0,255,128) );
           matrix.drawLine( x+1, y+1, x+2, y+1,  matrix.Color888(0,255,255) );
          break;

       default:  //   [][]
               //   [][]
          matrix.drawLine( x+1, y, x+2, y, matrix.Color888(255,0,255) );
           matrix.drawLine( x, y+1, x+1, y+1,  matrix.Color888(255,0,255) );
          break;
    }
}


int x = 10;
int y = 0;
int sharp = 0;
void drawTetris()
{
    //matrix.drawPixel( x, y, matrix.Color888( 255, 0, 0 ) );
    drawShape(x, y, sharp);
    y++;


    if (y > 32 )
    {
      y = 0;  x = random(30);
      sharp= random(7);
    }
  
}




void drawBMP()
{
  
    //matrix.swapBuffers(true);
    for ( int x = 0; x <32; x++)
    for ( int y = 0; y <32; y++)
    {
          matrix.drawPixel( x, y,
                            matrix.Color888( 
                                 img24[(31-y)*(32*3) + x*3], 
                                 img24[(31-y)*(32*3) + x*3+1], 
                                 img24[(31-y)*(32*3) + x*3+2],
                                 true    // gamma
                           )

         );  
    }

}



void loop()
{
    byte i = 0;
    int step = 0;
    int old_step = 0;
    tmElements_t tm;


    int xpos = 16;
    int ypos = 16;
    uint16_t color [ 10 ];


    color [0] = matrix.Color888(0x93, 0x4A, 0x5f ,true);
    color [1] = matrix.Color888(0x57, 0x64, 0x8C, true );
    color [2] = matrix.Color888(0xC2, 0xB4, 0xD6, true );
    color [3] = matrix.Color888(0xE5, 0xE5, 0xE5, true );

    color [4] = matrix.Color888(0x4c, 0x45, 0x56 ,true);
    color [5] = matrix.Color888(0x87, 0x26, 0x42, true );
    color [6] = matrix.Color888(0xf6, 0xc0, 0x26, true );
    color [7] = matrix.Color888(0xa0, 0xd3, 0xf9, true );
    
     
 //   color [0] = matrix.Color888(0xff,0xc0,0xCB);
 //   color [1] = matrix.Color888(0xff,0xff,0x00);
 //   color [2] = matrix.Color888(0x93,0x70,0xdb);
 //   color [3] = matrix.Color888(0x7f,0xff,0x00);
 //   color [4] = matrix.Color888(0x7f,0xff,0xd4);
//    color [5] = matrix.Color888(0xfa,0xf0,0xe6);
//    color [6] = matrix.Color888(0xf5,0xde,0xb3);
//    color [7] = matrix.Color888(0xf0,0xff,0xff);
    color [8] = matrix.Color888(0xee,0x82,0xee);
    color [9] = matrix.Color888(0xbd,0xb7,0x6b);



    
    while ( 1 )
    {   
        // Clear background
        matrix.fillScreen( 0 );

        if (i++  > 42 )
        {
            old_step = step;
            step = (++step % 8);
            i = 0;

            xpos = random(32);
            ypos = random(32);
        }

        matrix.fillScreen( color[old_step] );
        matrix.fillCircle( xpos, ypos, i, color[step] );
        
   //     drawBMP();
   //     drawTetris();

        
        char buf_hour[10];
        char buf_min[10];
        char sec[4];
        if ( RTC.read(tm) )
        {
            int h = tm.Hour;
            if ( tm.Hour > 12 )
                h = tm.Hour -12;

            sprintf( buf_hour, "%02d",h );
            sprintf( buf_min, "%02d", tm.Minute );
            sprintf( sec, "%02d", tm.Second);
        }
        else
        {
            tm.Hour = 22; tm.Minute = 10;
            RTC.write(tm);   
        }

#if 0      
        // _______________________________________________
        matrix.setTextSize(3);  
        matrix.setTextColor(matrix.Color888(32,45,45));
        matrix.setCursor(0, 5);
        matrix.print(sec[0] );
        matrix.setCursor(17, 5);
        matrix.print(sec[1]);
#endif
  
        matrix.setTextSize(9);

        // Draw big scrolly text on top
       // matrix.setTextColor(matrix.ColorHSV(255/*hue*/, 255, 255, true));
        matrix.setTextColor( matrix.Color888( 1,3,1) );
        matrix.setCursor(11, 24);
        matrix.print(buf_hour);
      
        matrix.setCursor(22, 24);
        matrix.print(buf_min);

        matrix.setCursor(18, 24);
        matrix.print(":");
  



/*
  matrix.setTextSize(0);
  matrix.setTextColor(matrix.Color888(255, 255, 255, true));
  matrix.setCursor(0, 0);
  matrix.print("12/23 SAT");
*/
        // Move text left (w/wrap), increase hue
        if ((--textX) < textMin) 
            textX = matrix.width();
        hue += 7;
        if (hue >= 1536)
            hue -= 1536;

        // Update display
        matrix.swapBuffers(false);
        delay(1);
    }
}
     




void loop_() {

  byte i;

  tmElements_t tm;
  while ( 1 )
  {
    // Clear background
    matrix.fillScreen(0);
    drawBMP();
    drawTetris();

  char buf_hour[10];
  char buf_min[10];
  char sec[4];
  if ( RTC.read(tm) )
  {
      int h = tm.Hour;
      if ( tm.Hour > 12 )
        h = tm.Hour -12;

    sprintf( buf_hour, "%02d",h );
    sprintf( buf_min, "%02d", tm.Minute );
    sprintf( sec, "%02d", tm.Second);
  }
  else
  {
      tm.Hour = 22; tm.Minute = 10;
      RTC.write(tm);   
  }





  
  // _______________________________________________
  matrix.setTextSize(3);  matrix.setTextSize(3);
  matrix.setTextColor(matrix.Color888(32,45,45));
  matrix.setCursor(0, 5);
  matrix.print(sec[0] );
  matrix.setCursor(17, 5);
  matrix.print(sec[1]);

  

  matrix.setTextSize(1);

  
  // Draw big scrolly text on top
  matrix.setTextColor(matrix.ColorHSV(hue, 255, 255, true));
  matrix.setCursor(2, 12);
  matrix.print(buf_hour);
  matrix.setCursor(19, 12);
  matrix.print(buf_min);


  matrix.setCursor(14, 12);
  matrix.print(":");
  matrix.setCursor(13, 12);
  matrix.print(":");

  
/*
  matrix.setTextSize(0);
  matrix.setTextColor(matrix.Color888(255, 255, 255, true));
  matrix.setCursor(0, 0);
  matrix.print("12/23 SAT");
*/
  // Move text left (w/wrap), increase hue
  if((--textX) < textMin) 
      textX = matrix.width();
  hue += 7;
  if(hue >= 1536)
    hue -= 1536;

  // Update display
  matrix.swapBuffers(false);


  char data;
  while(HC06.available()) {
      data = HC06.read();
      if(data == -1) break;
      Serial.print(data);
      delay(1);
   }  
/*
  // BT –> Data –> Serial
  if (HC06.available()) {
    Serial.write(HC06.read());
  }
*/
  // Serial –> Data –> BT
  if (Serial.available()) {
    HC06.write(Serial.read());
  }  

    delay(1);
  }
     
}


