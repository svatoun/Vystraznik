
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
int prodlevaZvonec = kmitaniVystraha; // zvonec cikne pokazde pri prepnuti vystraznych svetel
int delkaZvonec = 200;

void setup() {
  Serial.begin(115200);
  pinMode(prepUzavreni, INPUT);
  pinMode(prepZvonec, INPUT);
  pinMode(pozitivni, OUTPUT);
  pinMode(rozdelovac, OUTPUT);
  pinMode(vystraha, OUTPUT);
  pinMode(zvonec, OUTPUT);
}

long posledniVystraha = 0;    // cas posledni zmeny vystraznych svetel [ms]
long posledniPozitiv = 0;     // cas posledni zmeny pozitivky [ms]
long posledniZvonec = 0;      // cas posledniho uderu zvonce [ms]
long zakmitPrepUzavreni = 0;  // cas 1. detekce zmeny stavu prepUzavreni
long zakmitPrepZvonec = 0;  // cas 1. detekce zmeny stavu prepZvonec
long posledniZvonecSepnuti = 0; // cas posledni aktivace zvonu, slouzi pro synchronizaci vystrazniku

boolean stavPrepUzavreni = false;  // false: kmitave bile svetlo, true: 
boolean stavPrepZvonec = false;  // false: kmitave bile svetlo, true: 
boolean stavVystraznik = false; 
boolean stavPozitiv = false;    
boolean stavZvonec = false;     // false - zvonec off, true - zvonec on. Ruzne prodlevy

long t = 0;

boolean kontrolaVstupu(boolean &stav, long& zakmit, int pin) {
  boolean noveSepnuto = false;
  // boolean vstup = digitalRead(pin); // Pro spinani pomoci 0V, a INPUT_PULLUP
  boolean vstup = !digitalRead(pin); // Pro spinani pomoci 5VV, a INPUT

  // vstup musi byt shodny v case "T" a "T+20ms", aby se vyhodnotilo jako sepnuti a ne nahodny prekmit.
  if (zakmit + 20 < t) {
    boolean s = !vstup;
    if (stav != s) {
      if (zakmit != 0) {
        // druhy odecet, souhlasny -> stav se meni
        stav = s;
        noveSepnuto = stav; 
      } else {
        zakmit = t;
      }
    } else {
      // Kontrolni stav nesouhlasi - resetujeme cas vzniku udalosti, zaroven slouzi jako indikator prvni / druhy odecet
      zakmit = 0;
    }
  }
  return noveSepnuto;
}



void loop() {
  t = millis();
  if (kontrolaVstupu(stavPrepUzavreni, zakmitPrepUzavreni, prepUzavreni)) {
    posledniVystraha = stavZvonec ? posledniZvonecSepnuti : 0;   // synchronizujemne se zvoncem
    stavVystraznik = false; // inverze vychoziho stavu vystrazniku, ihned se zmeni
  }

  if (kontrolaVstupu(stavPrepZvonec, zakmitPrepZvonec, prepZvonec)) {
    stavZvonec = false;
    posledniZvonec = stavPrepUzavreni ? (posledniVystraha + delkaZvonec) : 0; // synchronizujeme s vystrahou
  }

  // Pozitivní světlo
  if (stavPrepUzavreni) {
    digitalWrite(rozdelovac, HIGH);  // Aktivace rozdělovače
    digitalWrite(pozitivni, LOW);    //
    if ((t < kmitaniVystraha) || (posledniVystraha < t - kmitaniVystraha)) {
      // Uplynul cas, zmenima stav vystrazniku
      stavVystraznik = !stavVystraznik;
      digitalWrite(vystraha, stavVystraznik ? HIGH : LOW);
      posledniVystraha = t;
    }
  } else {
    digitalWrite(rozdelovac, LOW);
    if ((t < kmitaniPozitivni) || (posledniPozitiv < t - kmitaniPozitivni)) {
      stavPozitiv = !stavPozitiv;
      digitalWrite(pozitivni, stavPozitiv ? HIGH : LOW);
      posledniPozitiv = t;
    }
  }

  // zvoni se jen pri uzavreni (?)
  if (stavPrepZvonec && stavPrepUzavreni) {
    // stavZvonec = true -> kratka prodleva pro pritazeni zvonce. true -> dlouha prodleva pro pauzu mezi udery. 
    long limit = stavZvonec ? delkaZvonec : prodlevaZvonec - delkaZvonec;
    if (stavZvonec) {
      limit++;
    }
    if (posledniZvonec + limit < t) {
      stavZvonec = !stavZvonec;
      if (stavZvonec) {
        posledniZvonecSepnuti = t;
      }
      posledniZvonec = t;
      digitalWrite(zvonec, stavZvonec ? HIGH : LOW);
    }
  } else {
    stavZvonec = false;
    digitalWrite(zvonec, LOW);
  }
}
