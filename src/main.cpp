#include "Arduino.h"
#include "Keyboard.h"

String buffer;
String command;
String argument;

unsigned long parse_time;

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
    // open the serial port:
    Serial.begin(9600);

    while(!Serial){}

    Serial.println("Keyboard Start");

    // Without this, readString() takes a loooong time, go figure
    Serial.setTimeout(100);

    // initialize control over the keyboard:
    Keyboard.begin();
}

void loop() {
    // check for incoming serial data:
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
