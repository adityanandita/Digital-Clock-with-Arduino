#include<avr/io.h>
#include<avr/interrupt.h>

int H, M, S = 0;        // Counter untuk mode jam digital
int H_a, M_a, S_a = 0;  // Counter untuk waktu alarm
int M_s, S_s, MS_s = 0; // Counter untuk stopwatch

float Hstate, HLstate, Mstate, MLstate = 0;
float Mode_State, Mode_Lstate = 0;
bool sw_start = false;
int mode_counter = 0;
int OVF = 0;
bool bPress = false;

void init_int(void){
    // Enable timer1 overflow interrupt
    TIMSK1 = (1 << TOIE1);

    // Set mode timer dengan prescaler 1024
    TCCR1A = 0x00;
    TCCR1B = (1<<CS10) | (1<<CS12);

    // Untuk 100 ms
    TCNT1 = 63974;
    
    // Enable interrupt
    sei();
}

ISR (TIMER1_OVF_vect){
	OVF++;
	if (sw_start){
		MS_s = MS_s + 10;
		if (MS_s >= 100){MS_s = 0; S_s++;}
		if (S_s >= 60){S_s = 0; M_s++;}
	}
	if (OVF >= 10) {S++; OVF = 0;}
	if (S >= 60) {S = 0; M++;} 
	if (M >= 60) {M = 0; H++;}
	if (H >= 24) {H = 0;}
	TCNT1 = 63974;
}

void setup(){
	init_int();
	DDRD = 0b11111111;
	DDRB = 0b00111111;
	H,M,S = 0;
  mode_counter = 0;
	pinMode(A0, INPUT_PULLUP);  // Tombol jam
	pinMode(A1, INPUT_PULLUP);  // Tombol menit
	pinMode(A2, INPUT_PULLUP);  // Tombol ganti mode
	pinMode(A3, INPUT_PULLUP);  // Alarm switch
	pinMode(A4, OUTPUT);        // Buzzer
}

void loop(){
  if (mode_counter%3 == 0){             // Mode jam digital
    mux(1, H/10); delay(5);	            // digit 1 jam
    mux(2, H - (H/10)*10); delay(5);  	// digit 2 jam
    mux(3, M/10); delay(5);	            // digit 1 menit
    mux(4, M - (M/10)*10); delay(5);	  // digit 2 menit
    mux(5, S/10); delay(5);	            // digit 1 detik
    mux(6, S - (S/10)*10); delay(5);	  // digit 2 detik
  }
  else if (mode_counter%3 == 1){          // Mode set alarm
    mux(1, H_a/10); delay(5);	            // digit 1 jam
    mux(2, H_a - (H_a/10)*10); delay(5);	// digit 2 jam
    mux(3, M_a/10); delay(5);	            // digit 1 menit
    mux(4, M_a - (M_a/10)*10); delay(5);	// digit 2 menit
  }
  else if (mode_counter%3 == 2){          // Mode stopwatch
    mux(1, M_s/10); delay(5);	            // digit 1 menit
    mux(2, M_s - (M_s/10)*10); delay(5);	// digit 2 menit
    mux(3, S_s/10); delay(5);	            // digit 1 detik
    mux(4, S_s - (S_s/10)*10); delay(5);	// digit 2 detik
	  mux(5, MS_s/10); delay(5);	          // digit 1/10 detik
  }
  checkH(); checkM(); checkMode(); checkAlarm();
}

void incH(){ //Fungsi Increment Jam pada mode Jam
	H++;
	if (H >= 24){H = 0;}
}

void incH_a(){ //Fungsi Increment Jam pada mode Alarm
	H_a++;
	if (H_a >= 24){H_a = 0;}
}

void incM(){ //Fungsi Increment Menit pada Mode Jam
	M++;
	if (M >= 60){M = 0; incH();}
}

void incM_a(){ //Fungsi Increment Menit pada Mode Alarm
	M_a++;
	if (M_a >= 60){M_a = 0; incH_a();}
}

void changeMode(){ //Fungsi untuk menentukan mode
	mode_counter++;
	if (mode_counter >= 3){mode_counter = 0;}
}

void checkMode(){ //Fungsi untuk mengubah mode
	Mode_State = digitalRead(A2);
	if (Mode_State != Mode_Lstate){
		bPress = true;
		changeMode();
	}
	Mode_Lstate = Mode_State;
}

void checkH(){ //Fungsi untuk mengecek push button hour
	Hstate = digitalRead(A0);
	if (Hstate != HLstate){
		if (Hstate == LOW){
			bPress = true;
			if (mode_counter == 0) incH(); //Melakukan increment Jam pada mode jam
			else if (mode_counter == 1) incH_a(); //Melakukan increment Jam pada mode Alarm
			else if (mode_counter == 2) {M_s = 0; S_s = 0; MS_s = 0;} //Mereset hitungan pada stopwatch
		}
	}
	HLstate = Hstate;
}

void checkM(){ //Fungsi untuk mengecek push button minute
	Mstate = digitalRead(A1);
	if (Mstate != MLstate){
		if (Mstate == LOW){
			bPress = true;
			if (mode_counter == 0) incM(); //Melakukan increment menit pada mode Jam
			else if (mode_counter == 1) incM_a(); //Melakukan increment menit pada mode Alarm
			else if (mode_counter == 2) sw_start = !sw_start; //Melakukan start/stop pada mode stopwatch
		}
	}
	MLstate = Mstate;
}

void checkAlarm(){ //Fungsi untuk menghidupkan Alarm
  if (digitalRead(A3)==LOW){
    if (H == H_a && M == M_a) digitalWrite(A4, HIGH);
    else digitalWrite(A4, LOW);
  }
  else digitalWrite(A4, LOW);
}

void mux(int digit, int angka){
	// Multiplexer untuk kombinasi output 7 segment
	switch(angka){
		case 0: PORTD = 0b11000000; break;
		case 1: PORTD = 0b11111001; break;
		case 2: PORTD = 0b10100100; break;
		case 3: PORTD = 0b10110000; break;
		case 4: PORTD = 0b10011001; break;
		case 5: PORTD = 0b10010010; break;
		case 6: PORTD = 0b10000010; break;
		case 7: PORTD = 0b11111000; break;
		case 8: PORTD = 0b10000000; break;
		case 9: PORTD = 0b10010000; break;
		default: PORTD = 0b10111111; break;
	}
	// Multiplexer untuk memilih digit
	switch(digit){
		case 6: PORTB = 0b00100000; break;
		case 5: PORTB = 0b00010000; break;
		case 4: PORTB = 0b00001000; break;
		case 3: PORTB = 0b00000100; break;
		case 2: PORTB = 0b00000010; break;
		case 1: PORTB = 0b00000001; break;
	}
}
