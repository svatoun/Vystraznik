
const boolean DEBUG = true;

// Definice pinů
const int pozitivni = 3;
const int rozdelovac = 4;
const int vystraha = 5;
const int zvonec = 6;
const int prepUzavreni = 7;
const int prepZvonec = 8;

// true, pokud se rele spina 0V (LOW); false, pokud se spina 5V (HIGH)
const boolean releOnLow = true;

// Časové konstanty
int kmitaniPozitivni = 750;
int kmitaniVystraha = 500;
int prodlevaZvonec = kmitaniVystraha; // zvonec cikne pokazde pri prepnuti vystraznych svetel
int delkaZvonec = 200;

// spinaci/vypinaci hodnoty pro rele, podle releOnLow
const int releOn = releOnLow ? LOW : HIGH;
const int releOff = releOnLow ? HIGH : LOW;


void setup() {
  Serial.begin(115200);
  pinMode(prepUzavreni, INPUT_PULLUP);
  pinMode(prepZvonec, INPUT_PULLUP);

  digitalWrite(pozitivni, releOff);
  digitalWrite(rozdelovac, releOff);
  digitalWrite(vystraha, releOff);
  digitalWrite(zvonec, releOff);

  // nejprve nastavit vystupni uroven, a az pak piny na vystup, jinak by pri on=LOW mohlo kratkou dobu sepnout pri startu rele.
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

boolean kontrolaVstupu(boolean &stav, long& zakmit, int pin, const char* nazev) {
  boolean noveSepnuto = false;
  boolean vstup = digitalRead(pin); // Pro spinani pomoci 0V, a INPUT_PULLUP
  //boolean vstup = !digitalRead(pin); // Pro spinani pomoci 5VV, a INPUT

  // vstup musi byt shodny v case "T" a "T+20ms", aby se vyhodnotilo jako sepnuti a ne nahodny prekmit.
  if (zakmit + 20 < t) {
    boolean s = !vstup;
    if (stav != s) {
      if (zakmit != 0) {
        // druhy odecet, souhlasny -> stav se meni
        stav = s;
        noveSepnuto = stav; 
        if (DEBUG) {
          Serial.print(nazev);
          Serial.println(stav ? ": zapnuto" : ": vypnuto");
        }
        zakmit = 0;
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
  if (kontrolaVstupu(stavPrepUzavreni, zakmitPrepUzavreni, prepUzavreni, "Uzavreni")) {
    posledniVystraha = stavZvonec ? posledniZvonecSepnuti : 0;   // synchronizujemne se zvoncem
    stavVystraznik = false; // inverze vychoziho stavu vystrazniku, ihned se zmeni
  }

  if (kontrolaVstupu(stavPrepZvonec, zakmitPrepZvonec, prepZvonec, "Zvonec")) {
    stavZvonec = false;
    posledniZvonec = stavPrepUzavreni ? (posledniVystraha + delkaZvonec) : 0; // synchronizujeme s vystrahou
  }

  // Pozitivní světlo
  if (stavPrepUzavreni) {
    digitalWrite(rozdelovac, releOn);  // Aktivace rozdělovače
    digitalWrite(pozitivni, releOff);    //
    if ((t < kmitaniVystraha) || (posledniVystraha < t - kmitaniVystraha)) {
      if (DEBUG) {
        Serial.print("Zmena vystrazniku: "); Serial.println(t);
      }
      // Uplynul cas, zmenima stav vystrazniku
      stavVystraznik = !stavVystraznik;
      digitalWrite(vystraha, stavVystraznik ? releOn : releOff);
      posledniVystraha = t;
    }
  } else {
    digitalWrite(rozdelovac, releOff);
    if ((t < kmitaniPozitivni) || (posledniPozitiv < t - kmitaniPozitivni)) {
      if (DEBUG) {
        Serial.print("Zmena pozitivky: "); Serial.println(t);
      }
      stavPozitiv = !stavPozitiv;
      digitalWrite(pozitivni, stavPozitiv ? releOn : releOff);
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
        if (DEBUG) {
          Serial.println("Zvonec on");
        }
        posledniZvonecSepnuti = t;
      } else {
        if (DEBUG) {
          Serial.println("Zvonec off");
        }
      }
      posledniZvonec = t;
      digitalWrite(zvonec, stavZvonec ? releOn : releOff);
    }
  } else {
    stavZvonec = false;
    digitalWrite(zvonec, releOff);
  }
}
