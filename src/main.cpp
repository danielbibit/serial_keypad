#include "Arduino.h"
#include "HID-Project.h"

#define DEBOUNCE_TIME 30
#define SINGLE_PRESS_DEBOUNCE_TIME 200

String buffer;
String command;
String argument;

// Switch -> pins
// 16 14 15
// 6 7 8

//Switch -> logical map
// 0 1 2
// 3 4 5

enum status {
    PRESSED,
    HOLDING,
    RELEASED
};

const unsigned int input_switches [] = {16, 14, 15, 6, 7, 8};

unsigned long key_debounce_time[6];
unsigned long key_pressed_time[6];

status key_statuses[6];

typedef void (* GenericFP)(void);

GenericFP user_macros[6];

KeyboardKeycode keyboard_keys[6];
ConsumerKeycode media_keys[6];


// User defined macros
void user_func_switch_monitors(){
    BootKeyboard.press(KEY_LEFT_GUI);
    delay(200);
    BootKeyboard.write('p');
    delay(200);
    BootKeyboard.write('p');
    delay(200);
    BootKeyboard.releaseAll();
    BootKeyboard.write(KEY_ESC);
}

void user_func_switch_audio_output(){
    BootKeyboard.press(KEY_LEFT_CTRL);
    delay(50);
    BootKeyboard.press(KEY_LEFT_ALT);
    delay(50);
    BootKeyboard.press(KEY_F1);
    delay(150);
    BootKeyboard.releaseAll();
}

void user_func_mute_mic(){
    BootKeyboard.press(KEY_LEFT_CTRL);
    delay(50);
    BootKeyboard.press(KEY_LEFT_ALT);
    delay(50);
    BootKeyboard.press(KEY_M);
    delay(150);
    BootKeyboard.releaseAll();
}

// Setup keys
void setup_user_keys(){
    user_macros[0] = user_func_switch_audio_output;
    keyboard_keys[1] = KEY_PRINTSCREEN;
    media_keys[2] = MEDIA_VOLUME_UP;
    user_macros[3] = user_func_mute_mic;
    media_keys[4] = MEDIA_VOLUME_MUTE;
    media_keys[5] = MEDIA_VOLUME_DOWN;
}

// Handles keypress
void switch_action(int i){
    // Serial.println("Action!");

    if(user_macros[i]){
        user_macros[i]();
    }else if (keyboard_keys[i]){
        BootKeyboard.write(keyboard_keys[i]);
    }else if (media_keys[i]){
        Consumer.write(media_keys[i]);
    }
}

void process_key_pressed(int key_index){
    //Need to debounce release
    key_pressed_time[key_index] = millis();

    // RELEASED -> PRESSED
    if(key_statuses[key_index] == RELEASED){
        key_statuses[key_index] = PRESSED;
        key_debounce_time[key_index] = millis();

        switch_action(key_index);

        return;

    // PRESSED -> HOLDING
    } else if(key_statuses[key_index] == PRESSED){
        if(millis() - key_debounce_time[key_index] > SINGLE_PRESS_DEBOUNCE_TIME){
            key_statuses[key_index] = HOLDING;
            key_debounce_time[key_index] = millis();

            switch_action(key_index);
        }

        return;

    // HOLDING -> HOLDING
    } else if(key_statuses[key_index] == HOLDING){
        if(millis() -  key_debounce_time[key_index] > DEBOUNCE_TIME){
            key_debounce_time[key_index] = millis();

            switch_action(key_index);
        }
    }
}

void process_key_release(int key_index){
    // HOLDING/PRESSED -> RELEASED
    if(millis() - key_pressed_time[key_index] > DEBOUNCE_TIME){
        key_statuses[key_index] = RELEASED;
    }

    // Consumer.releaseAll();
}

// Serial Functions
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
        key_debounce_time[i] = 0;
        key_pressed_time[i] = 0;
        // user_macros[i] = NULL;
    }

    setup_user_keys();

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
            process_key_pressed(i);
        }else{
            process_key_release(i);
        }

    }

    if(Serial.available() > 0) {
        buffer = Serial.readStringUntil('\n');

        if(parse_serial_command()){
            if(command == "write"){
                BootKeyboard.write(argument.toInt());

            }else if(command == "print"){
                BootKeyboard.print(argument);
            }

            clear_parsed_commands();
        }
    }

    delay(5);
}
