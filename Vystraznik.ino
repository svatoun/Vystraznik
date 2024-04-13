// Definice pinů
const int pozitivni = 3;
const int rozdelovac = 4;
const int vystraha = 5;
const int zvonec = 6;
const int prepUzavreni = 7;
const int prepZvonec = 8;
// Časové konstanty
int kmitaniPozitivni = 750;
int kmitaniVystraha = 500;
int delkaZvonec = 200;

void setup() {
pinMode(prepUzavreni, INPUT);
pinMode(prepZvonec, INPUT);
pinMode(pozitivni, OUTPUT);
pinMode(rozdelovac, OUTPUT);
pinMode(vystraha, OUTPUT);
pinMode(zvonec, OUTPUT);
}
void loop() {
// Pozitivní světlo
if (digitalRead(prepUzavreni) == LOW) {
digitalWrite(pozitivni, HIGH);
delay(kmitaniPozitivni);
digitalWrite(pozitivni, LOW);
delay(kmitaniPozitivni);
}
// Vystraha
if (digitalRead(prepUzavreni) == HIGH) {
digitalWrite(rozdelovac, HIGH); // Aktivace rozdělovače
if (digitalRead(prepZvonec) == LOW) {
// Kmitání červeného světla
digitalWrite(vystraha, HIGH);
delay(kmitaniVystraha);
digitalWrite(vystraha, LOW);
delay(kmitaniVystraha);
} else if (digitalRead(prepZvonec) == HIGH) {
// Kmitání zvonku (přidat příslušný kód pro ovládání zvonku)
digitalWrite(vystraha, HIGH); // Simulace vystrahy při zvonění
delay(kmitaniVystraha);
digitalWrite(vystraha, LOW);
delay(kmitaniVystraha);
// Kód pro ovládání zvonku
digitalWrite(zvonec, HIGH);
delay(delkaZvonec);
digitalWrite(zvonec, LOW);
}
}
}
