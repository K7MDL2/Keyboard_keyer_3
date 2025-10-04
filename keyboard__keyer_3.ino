#include <PS2Keyboard.h>
#include <bt_keyboard.hpp>
#include "esp_err.h"
//#include "esp_system.h"
#include "nvs_flash.h"

//  _____ _            _          _      _            __  __                 
// |_   _| |_  ___    /_\  _ _ __| |_  _(_)_ _  ___  |  \/  |___ _ _ ___ ___ 
//   | | | ' \/ -_)  / _ \| '_/ _` | || | | ' \/ _ \ | |\/| / _ \ '_(_-</ -_)
//   |_| |_||_\___| /_/ \_\_| \__,_|\_,_|_|_||_\___/ |_|  |_\___/_| /__/\___|
//                                                                           
//  _  __                 
// | |/ /___ _  _ ___ _ _ 
// | ' </ -_) || / -_) '_|
// |_|\_\___|\_, \___|_|  
//           |__/   
//
// Version for the PS2 Keyboard - Modified to use BT keyboard
// using the code https://github.com/turgu1/bt-keyboard and converted toa library
// using the library from http://www.pjrc.com/teensy/td_libs_PS2Keyboard.html
// 
// Written by Mark VandeWettering K6HX
//
// This is just a quick Morse keyer program.
//

// BT keyboard setup functions
PS2Keyboard kbd ;
BTKeyboard bt_keyboard;

////////////////////////////////////////////////////////////////////////
int LED_BT_Connected_pin = 2;    // SHow BT keyboard connected state
int LED_pin = 25 ;              // blink the LED for dits and dahs
int tpin = 14 ;                 // tone pin
int spin = 26;                  // speed sense switch
//pinMode(7, INPUT_PULLUP);
//if (digitalRead(spin) == LOW) wpm = 12; 
// QUEUESIZE must be a power of two 
#define QUEUESIZE       (128)
#define QUEUEMASK       (QUEUESIZE-1)
#define DEBUG false
   
int aborted = 0 ;
int qhead = 0 ;
int qtail = 0 ;
char queue[QUEUESIZE] ;
 
unsigned int freq = 700;
unsigned int wpm = 18;
int ditlen = 1200 / wpm ;

void pairing_handler(uint32_t pid) {
    Serial.print(F("Please enter the following pairing code followed with ENTER on your keyboard: "));
    Serial.println(pid);
}

void keyboard_lost_connection_handler() {
    Serial.println(F("====> Lost connection with keyboard <===="));
    digitalWrite(LED_BT_Connected_pin, 0);
}

void keyboard_connected_handler() { 
    Serial.println(F("----> Connected to keyboard <----")); 
    digitalWrite(LED_BT_Connected_pin, 1);
}

////////////////////////////////////////////////////////////////////////
//
// Here is a queue to store the characters that I've typed.
// To simplify the code, it can store a maximum of QUEUESIZE-1 characters
// before it fills up.  What is a byte wasted between friends?
//
////////////////////////////////////////////////////////////////////////

void
queueadd(const char ch)
{
    queue[qtail++] = ch ;
    qtail &= QUEUEMASK ;
}
 
void
queueadd(const char *s)
{
  while (*s)
      queueadd(*s++) ;
}
 
char
queuepop()
{
    char ch ;
    ch = queue[qhead++] ;
    qhead &= QUEUEMASK ;
    return ch ;
}
 
int
queuefull()
{
    return (((qtail+1)%QUEUEMASK) == qhead) ;
}
 
int
queueempty()
{
    return (qhead == qtail) ;
}
 
void
queueflush()
{
    qhead = qtail ;
}
 

 
inline void
ps2poll()
{
    
    char ch ;
    while (kbd.available()) {
        if (queuefull()) {
            Serial.print("") ;
        } else {
            switch (ch=kbd.read()) {
            case PS2_ENTER:
                break ;
            case PS2_TAB:
                break ;
            case PS2_PAGEDOWN:
                break ;
            case PS2_PAGEUP:
                break ;
            case PS2_LEFTARROW:
                if (freq < 300) break;
                freq -= 50;
#if DEBUG 
                Serial.print(F("FREQUENCY = ")) ;
                Serial.println(freq) ;
#endif                
                break ;
            case PS2_RIGHTARROW:
                if (freq > 2000) break;
                freq += 50; 
#if DEBUG 
                Serial.print(F("FREQUENCY = ")) ;
                Serial.println(freq) ;                
#endif                
                break ;
            case PS2_UPARROW:
                if (wpm < 5) break;
                wpm -= 1; 
                ditlen = 1200 / wpm ;
#if DEBUG 
                Serial.print(F("WPM = ")) ;
                Serial.println(wpm) ;                
#endif                
                break ;
            case PS2_DOWNARROW:
                if (wpm > 30) break;
                wpm += 1; 
                ditlen = 1200 / wpm ;
#if DEBUG 
                Serial.print(F("WPM = ")) ;
                Serial.println(wpm) ;                
#endif                
                break ;
            case PS2_DELETE:
                break ;
            case PS2_ESC:
                queueflush() ;
                Serial.flush() ;
                Serial.println(F("== FLUSH ==")) ;
                aborted = 1 ;
                break ;
            case '!':
                queueadd("CQ CQ CQ DE W7RNB W7RNB BK\r\n") ;
                break ;
            case '@':
                queueadd("UR RST IS BK\r\n") ;
                break;   
            case '#':
                queueadd("RRR 73 RRR 73 TU  SK E E\r\n") ;
                break;
            case '^' :
                queueadd("CQ FD CQ FD DE W7RNB W7RNB BK \r\n") ;
                break;
            case '&' :
                queueadd("DE W7RNB 1D 1D WWA WWA BK\r\n") ;
                break; 
            case '*' : 
                queueadd("AGN? AGN?") ;
                break;
            case '(' :
                queueadd("QRZ QRZ DE W7RNB K\r\n") ;  
                break;    
            default:
                queueadd(ch) ;
                break ;
            }
        }
    }
}

void mydelay(unsigned long ms)
{
    unsigned long t = millis() ;
    while (millis()-t < ms)
        ps2poll() ;
}
  
void scale()
{
  long f = 220L ;
  int i ;
   
  for (i=0; i<=12; i++) {
      tone(tpin, (int)f) ;
      f *= 1059L ;
      f /= 1000L ;
      Serial.println(f) ;
      delay(300) ;
  }
  noTone(tpin) ;
       
}
 
void dit()
{
    digitalWrite(LED_pin, HIGH) ;
    tone(tpin, freq) ;
    mydelay(ditlen) ;
    digitalWrite(LED_pin, LOW) ;
    noTone(tpin) ;
    mydelay(ditlen) ;
 
}
 
void dah()
{
    digitalWrite(LED_pin, HIGH) ;
    tone(tpin, freq) ;
    mydelay(3*ditlen) ;
    digitalWrite(LED_pin, LOW) ;
    noTone(tpin) ;
    mydelay(ditlen) ;
}
 
void lspace()
{
    mydelay(2*ditlen) ;
}
 
void space()
{
    mydelay(4*ditlen) ;
}
  
const char ltab[] = {
    0b101,              // A
    0b11000,            // B 
    0b11010,            // C
    0b1100,             // D
    0b10,               // E
    0b10010,            // F
    0b1110,             // G
    0b10000,            // H
    0b100,              // I
    0b10111,            // J
    0b1101,             // K
    0b10100,            // L
    0b111,              // M
    0b110,              // N
    0b1111,             // O
    0b10110,            // P
    0b11101,            // Q
    0b1010,             // R
    0b1000,             // S
    0b11,               // T
    0b1001,             // U
    0b10001,            // V
    0b1011,             // W
    0b11001,            // X
    0b11011,            // Y
    0b11100             // Z
} ;
 
const char ntab[] = {
    0b111111,           // 0
    0b101111,           // 1
    0b100111,           // 2
    0b100011,           // 3
    0b100001,           // 4
    0b100000,           // 5
    0b110000,           // 6
    0b111000,           // 7
    0b111100,           // 8
    0b111110            // 9
} ;
 
void sendcode(char code)
{
    int i ;
 
    for (i=7; i>= 0; i--)
        if (code & (1 << i))
            break ;
 
    for (i--; i>= 0; i--) {
        if (code & (1 << i))
            dah() ;
        else
            dit() ;
    }
    lspace() ;
}
 
void send(char ch)
{
    if (isalpha(ch)) {
        if (islower(ch)) ch = toupper(ch) ;
        sendcode(ltab[ch-'A']) ;
    } else if (isdigit(ch))
        sendcode(ntab[ch-'0']) ;
    else if (ch == ' ' || ch == '\r' || ch == '\n')
        space() ;
    else if (ch == '.')
        sendcode(0b1010101) ;
    else if (ch == ',')
        sendcode(0b1110011) ;
    else if (ch == '!')
        sendcode(0b1101011) ;
    else if (ch == '?')
        sendcode(0b1001100) ;
    else if (ch == '/')
        sendcode(0b110010) ;
    else if (ch == '+')
        sendcode(0b101010) ;
    else if (ch == '-')
        sendcode(0b1100001) ;
    else if (ch == '=')
        sendcode(0b110001) ;
    else if (ch == '@')         // hardly anyone knows this!
        sendcode(0b1011010) ;
    else
        return ;                // ignore anything else
 
    if (!aborted) {
      Serial.print(ch) ;
      if (ch == 13) Serial.print((char) 10) ;
    }
    aborted = 0 ;
}
 
//////////////////////////////////////////////////////////////////////// 
void setup()
{
    esp_err_t ret;
    pinMode(LED_pin, OUTPUT) ;
    pinMode(LED_BT_Connected_pin, OUTPUT);
    pinMode(tpin, OUTPUT) ; 
    digitalWrite(LED_pin, 0);
    Serial.begin(115200) ;
    kbd.begin(4, 3) ;
    Serial.println(F("Morse Code Keyboard by K6HX, modified by K7MDL for BT keyboard 2025")) ;
    // To test the Pairing code entry, uncomment the following line as pairing info is
    // kept in the nvs. Pairing will then be required on every boot.
    ESP_ERROR_CHECK(nvs_flash_erase());

    ret = nvs_flash_init();
    if ((ret == ESP_ERR_NVS_NO_FREE_PAGES) || (ret == ESP_ERR_NVS_NEW_VERSION_FOUND)) {
        nvs_flash_erase();
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    Serial.println("BT and BLE device Scan Setup");
    
    btStarted();

    if (bt_keyboard.setup(pairing_handler, keyboard_connected_handler,
                            keyboard_lost_connection_handler)) { // Must be called once
        bt_keyboard.devices_scan(); // Required to discover new keyboards and for pairing
                                    // Default duration is 5 seconds
    }
}
////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////

void loop()
{
    char c, ch, ch_digit;
    static bool keyDN = false;
    static bool last_key = true;

    #if 0 // 0 = scan codes retrieval, 1 = augmented ASCII retrieval
        uint8_t ch = bt_keyboard.wait_for_ascii_char();
        // uint8_t ch = bt_keyboard.get_ascii_char(); // Without waiting

        if ((ch >= ' ') && (ch < 127)) std::cout << ch << std::flush; 
        else if (ch > 0) {
            Serial.print("0x%X, ",inf.keys[2]);  // do not print key up events
            std::cout << '[' << +ch << ']' << std::flush;
        }
    #else
        BTKeyboard::KeyInfo inf;

        bt_keyboard.wait_for_low_event(inf,1);

        // simple decoding fdor Rii K08 BLE mini keyboard, aka Rii model i8+
        //std::cout << "RECEIVED KEYBOARD EVENT: ";
        //for (int n = 0; n < inf.size; n++) {
        if (inf.size == 8) {   // keyboard chars are len = 8, mousr and others are len=4
        c = inf.keys[2];
        #ifdef DEBUG_BT_KEYBOARD
            //Serial.print(ch);
            //Serial.print('-');
        #endif
        if (c != 0 && keyDN != true) 
            keyDN = true;  // this is a valid alphanumeric key

        if (keyDN != last_key) // only process new key events separated by key-up
        {
            if (c == 0) // ignore keyups
            {
                keyDN == false;
                //Serial.print(ch);
            } 
            else 
            {
                keyDN = true;  // this is a valid alphanumeric key - send to processing
                #ifdef DEBUG_BT_KEYBOARD
                    Serial.print(',0x');
                    Serial.print(c,HEX);  // print our valid char
                    Serial.print(',');
                #endif

                // inf.keys[x] where
                // x=0 is normal key
                // x=2 is Sfift+key
                // x=1 is ctl+key
                // x=4 is Alt+key
                    
                if (inf.keys[0] == 2) // shift key pressed
                {
                    switch (c) {     
                        case 0x04 ... 0x1d : ch = c + 0x5D;  // convert to lower case letters
                                    if (isalpha(ch)) {
                                        if (islower(ch)) 
                                            ch = toupper(ch);
                                    } else ch = 0;
                                    break;                     
                        case 0x1e : ch = '!'; break;      // '!'  key
                        case 0x1f : ch = '@'; break;      // '@'  key
                        case 0x20 : ch = '#'; break;      // '#'  key
                        case 0x21 : ch = '$'; break;      // '$'  key
                        case 0x22 : ch = '%'; break;      // '%'  key
                        case 0x23 : ch = '^'; break;      // '^'  key
                        case 0x24 : ch = '&'; break;      // '&'  key
                        case 0x25 : ch = '*'; break;      // '*'  key                            
                        case 0x26 : ch = '('; break;      // '('  key
                        case 0x27 : ch = ')'; break;      // ')'  key                            
                        case 0x2D : ch = '_'; break;      // '_'  key
                        case 0x2E : ch = '+'; break;      // '+'  key
                        case 0x2F : ch = '{'; break;      // '{'  key
                        case 0x30 : ch = '}'; break;      // '}'  key    
                        case 0x31 : ch = '|'; break;      // '|' key                        
                        case 0x33 : ch = ':'; break;      // ':'  key
                        case 0x34 : ch = '"'; break;      // '"'  key
                        case 0x36 : ch = '<'; break;      // '<'  key
                        case 0x37 : ch = '>'; break;      // '>'  key
                        case 0x38 : ch = '?'; break;      // '?' cursor key
                    }
                } 
                else
                {
                    switch (c) {
                        case 0x04 ... 0x1d : ch = c + 0x5D;  // convert to lower case letters
                                    if (isalpha(ch)) {
                                        if (islower(ch)) 
                                            ch = toupper(ch);
                                    } else ch = 0;
                                    break;
                        case 0x1e ... 0x26 : ch = c + 0x13;  // numbers 1-9
                                    if (!isdigit(ch)) ch = 0;
                                    break;
                        case 0x27 : ch = c+ 0x09;  // number 0
                                    if (!isdigit(ch)) ch = 0;
                                    break;
                        case 0x28 : ch = '\n'; break;   // enter key
                        case 0x29 : ch = '\n'; break;   // ESC key - figure out how to erase a queue or stop senbding with this
                        case 0x2A : ch = '\n'; break;   // BACK key
                        case 0x2B : ch = '\t'; break;   // TAB key
                        case 0x2c : ch = ' '; break;    // space
                        case 0x2D : ch = '-'; break;    // '-'  key
                        case 0x2E : ch = '='; break;    // '='  key
                        case 0x2F : ch = '['; break;    // '['  key                                                  
                        case 0x30 : ch = '['; break;    // '['  key
                        case 0x31 : ch = '\\'; break;   // '\' key
                        case 0x33 : ch = ';'; break;    // ';'  key
                        case 0x34 : ch = '\''; break;   // '''  key
                        case 0x36 : ch = ','; break;    // ','  key
                        case 0x37 : ch = '.'; break;    // '.'  key
                        case 0x38 : ch = '/'; break;    // '/' cursor key
                        case 0x39 : ch = '\n'; break;   // CAP LOCK toggle
                        case 0x3A ...0x43: ch = '\n'; break; // F1-F10 keys
                        case 0x4F : ch = '\n'; break;   // right cursor key
                        case 0x50 : ch = '\n'; break;   // left cursor key
                        case 0x51 : ch = '\n'; break;   // down cursor key
                        case 0x52 : ch = '\n'; break;   // up cursor key
                        default   : break; //debug_serial_port->print(ch);   // filter out key up events
                    }  // end switch ch
                }
                #ifdef DEBUG_BT_KEYBOARD
                    Serial.print(ch);  // print our valid char
                    Serial.print(',');
                #endif

                if (ch == '\n') {
                   Serial.println("");  // print new line
                } else if ((ch > 31) && (ch < 255 /*123*/)) {
                    send(ch);
                }
            }
        }
    }
    last_key = keyDN;
    keyDN = false;

    #endif

    ps2poll() ;   //can leave active for USB wired keyboad

    if (!queueempty())
        send(queuepop()) ;
    
    /*  Test sending CQ to buzzer
    char c1 = 'C';
    char c2 = 'Q';
    char c3 = ' ';
    send(c1);
    send(c2);
    send(c3);
    //delay(1000);
    */
}
