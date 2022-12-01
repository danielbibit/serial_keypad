#include "Arduino.h"
#include "HID-Project.h"

#define DEBOUNCE_TIME 150

String buffer;
String command;
String argument;

// Switch -> pins
// 16 14 15
// 6 7 8

//Switch -> logical map
// 0 1 2
// 3 4 5

const int switch_keys[] = {
    KEY_F13,
    KEY_F14,
    KEY_F15,
    KEY_F16,
    KEY_F17,
    KEY_F18,
};

const unsigned int input_switches [] = {16, 14, 15, 6, 7, 8};

unsigned long debounce_switches[6];

// Switch between monitors
void f0(){
    Keyboard.press(KEY_LEFT_GUI);
    delay(200);
    Keyboard.write('p');
    delay(200);
    Keyboard.write('p');
    delay(200);
    Keyboard.releaseAll();
    Keyboard.write(KEY_ESC);
}

void clear_parsed_commands(){
    command.remove(0);
    argument.remove(0);
}

bool parse_serial_command(){
    clear_parsed_commands();

    bool separator_found = false;

    for(unsigned int i = 0; i < buffer.length(); i++ ){
        if(buffer[i] == ':'){
            separator_found = true;

        }else if(!separator_found){
            command += buffer[i];

        }else{
            argument += buffer[i];

        }
    }

    if(!separator_found){
        clear_parsed_commands();

        return false;
    }

    buffer.remove(0);

    return true;
}


void setup() {
    for (unsigned int i = 0; i<6; i++){
        pinMode(input_switches[i], INPUT_PULLUP);
        debounce_switches[i] = 0;
    }

    // open the serial port:
    Serial.begin(9600);

    // while(!Serial){}

    Serial.println("Keyboard Start");

    // Without this, readString() takes a loooong time, go figure
    Serial.setTimeout(100);

    // initialize control over the keyboard:
    BootKeyboard.begin();

    Consumer.begin();
}

void loop() {

    for(unsigned int i = 0; i<6; i++){
        if(digitalRead(input_switches[i]) == LOW){
            if(millis() - debounce_switches[i] > DEBOUNCE_TIME){
                debounce_switches[i] = millis();

                switch (i){
                    case 0:
                        // f0();
                        break;

                    case 1:
                        BootKeyboard.write(KEY_PRINTSCREEN);
                        break;

                    case 2:
                        Consumer.write(MEDIA_VOLUME_UP);
                        break;

                    case 3:
                        break;

                    case 4:
                        Consumer.write(MEDIA_VOL_MUTE);
                        break;

                    case 5:
                        Consumer.write(MEDIA_VOLUME_DOWN);
                        break;

                    default:
                        break;
                }
            }
        }
    }


    if(Serial.available() > 0) {
        buffer = Serial.readStringUntil('\n');

        if(parse_serial_command()){
            if(command == "write"){
                Keyboard.write(argument.toInt());

            }else if(command == "print"){
                Keyboard.print(argument);

            }

            clear_parsed_commands();
        }
    }

    delay(10);
}
