/**
 * @file uart_echo.ino
 * @brief Arduino-style UART benchmark - Serial
 */

void setup() {
    Serial.begin(9600);
    Serial.println("Arduino UART Ready");
}

void loop() {
    if (Serial.available()) {
        char c = Serial.read();
        Serial.write(c);
        if (c == '\r') Serial.write('\n');
    }
}