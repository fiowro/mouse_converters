#include <Mouse.h>

// Mouse
#define mousePinYa SCK
#define mousePinXa 7
#define mousePinYb MISO
#define mousePinXb MOSI

// Mouse buttons (common for both)
#define buttonPinL (5)  /* left button (A1) */
#define buttonPinR (6)  /* right button (A3) */
#define buttonPinM (4)  /* middle button (A2) */

// on-board LED pin
// #define kLED (17)

// ----------------------------------------

// setup - initialize the hardware
void setup() {
  pinMode( LED_BUILTIN, OUTPUT );
  
  // set the mouse and button inputs
  pinMode( mousePinYa, INPUT );
  pinMode( mousePinXa, INPUT );
  pinMode( mousePinYb, INPUT );
  pinMode( mousePinXb, INPUT );
  pinMode( buttonPinM, INPUT );
  pinMode( buttonPinL, INPUT );
  pinMode( buttonPinR, INPUT );
  
 // Serial.begin( 9600 );
  Mouse.begin(); // for USB HID Mouse support
  initGrayMouse(); 

}

// mouse movement history, used for acceleration
char history_x[128];
char history_y[128];
int historyPos=0; // current write position in the history

// initialize for gray code mouse 
void initGrayMouse( void )
{
  // clear the history
  for( int h=0 ; h<128 ; h++ ) {
    history_x[h] = history_y[h] = 0;
  }
}

// provide a total of all X history values (for acceleration)
int total_x( void )
{
  int ret = 0;
  for( int i=0 ; i < 128 ; i++ )
  {
    ret += history_x[i];
  }
  return ret;
}

// provide a total of all Y history values (for acceleration)
int total_y( void )
{
  int ret = 0;
  for( int i=0 ; i < 128 ; i++ )
  {
    ret += history_y[i];
  }
  return ret;
}


// compare A and B to determine the delta
  // +  00 -> 01 -> 11 -> 10 -> 00 
  // -  00 -> 10 -> 11 -> 01 -> 00
int grayCompare( int a, int b )
{
  a = a & 0x03;
  b = b & 0x03;
  if( a == b ) return 0;
  switch( a ) {
    case( 0x00 ):
      if( b == 0x01) return +1;
      if( b == 0x10) return -1;
      break;
    case( 0x01 ):
      if( b == 0x11) return +1;
      if( b == 0x00) return -1;
      break;
    case( 0x11 ):
      if( b == 0x10) return +1;
      if( b == 0x01) return -1;
      break;
    case( 0x10 ):
      if( b == 0x00) return +1;
      if( b == 0x11) return -1;
      break;
  }
  return 0;
}

// the main gray mouse loop
void loopGrayMouse()
{
  bool changed = false; // did something change?
  
  // these are static so they persist between calls
  static int x_accum = 0; // accumulated X
  static int y_accum = 0; // accumulated Y

  static int last_hq = 0; // last h quadrature
  static int last_vq = 0; // last v quadrature
  
  // read in the quad/gray code
  int hq = 0;
  int vq = 0;
  
  hq = (digitalRead( mousePinXa ) << 1) | digitalRead( mousePinXb );
  vq = (digitalRead( mousePinYa ) << 1) | digitalRead( mousePinYb );
    
  // check horizontal delta
  if( hq != last_hq ) {
    x_accum += grayCompare( hq, last_hq );
    changed = true;
  }
  last_hq = hq;
  
  // and vertical delta
  if( vq != last_vq ) {
    y_accum += grayCompare( vq, last_vq );
    changed = true;
  }
  last_vq = vq;
    
  // generate the acceleration info from gray tick
  historyPos++;
  history_x[ (historyPos & 0x7f) ] = x_accum;
  history_y[ (historyPos & 0x7f) ] = y_accum;
  int tx = total_x() * 2;
  int ty = total_y() * 2;
  
  // if something changed, move the mouse
  if( x_accum || y_accum ) {
    Mouse.move( tx, ty, 0 );
    x_accum = y_accum = 0;
  }
  
}
 
//////////////////////////////////////////////////////
// handler to read the mouse buttons.
// broke this out since it's the same for mouse vs joystick
void handleButtonPresses()
{
  // b1 = left
  // b2 = right
  // b3 = middle
  
  // left
  static int lb1 = LOW;
  int b1 = digitalRead( buttonPinL );
  
  if( b1 != lb1 ) {
    if( b1 == LOW ) {
      Mouse.press( MOUSE_LEFT );
      delay( 50 ); // fakeo debounce
    }
    if( b1 == HIGH ) Mouse.release( MOUSE_LEFT );
  }
  
  // necessary to prevent mouse fighting
  //if( b1 == HIGH && Mouse.isPressed( MOUSE_LEFT )) {
  //  Mouse.release( MOUSE_LEFT );
  //}
  
  lb1 = b1;

  
  // right
  static int lb2 = LOW;
  int b2 = digitalRead( buttonPinR );
  
  if( b2 != lb2 ) {
    if( b2 == LOW ) {
      Mouse.press( MOUSE_RIGHT );
      delay( 50 ); // fakeo debounce
    }
    if( b2 == HIGH ) Mouse.release( MOUSE_RIGHT );
  }
  // necessary to prevent mouse fighting
  //if( b2 == HIGH && Mouse.isPressed( MOUSE_RIGHT )) {
  //  Mouse.release( MOUSE_RIGHT );
  //}
  lb2 = b2;
  
  
  // middle
  static int lb3 = LOW;
  int b3 = digitalRead( buttonPinM );
  
  if( b3 != lb3 ) {
    if( b3 == LOW ) {
      Mouse.press( MOUSE_MIDDLE );
      delay( 50 ); // fakeo debounce
    }
    if( b3 == HIGH ) Mouse.release( MOUSE_MIDDLE );
  }
  // necessary to prevent mouse fighting
  //if( b3 == HIGH && Mouse.isPressed( MOUSE_MIDDLE )) {
  //  Mouse.release( MOUSE_MIDDLE );
  //}
  lb3 = b3;
}

//////////////////////////////////////////////////////
// main loop
void loop() {
      loopGrayMouse();
      handleButtonPresses();
}
