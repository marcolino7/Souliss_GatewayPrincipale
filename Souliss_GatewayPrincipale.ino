/**************************************************************************
	--- Souliss ---
    -- #BUILD:4 --

        Gateway Principale

		IP: 192.168.1.129

Modifiche a Souliss:

FRIARIELLO
--\conf\usart\usartUsrCfg.h
		#ifndef USARTBAUDRATE_INSKETCH
		# define USART_BAUD9k6			  0
		# define USART_BAUD19k2			 0
		# define	USART_BAUD57k6			1
		# define USART_BAUD115k2			0
		#endif
	Impostare la velocità a 57600

--\souliss\frame\vNet\drivers\nRF24\src\nRF24L01.h
	#define RF_SETUP da 0x06 a 0x26

--\conf\frame\vNetCfg.h
		#define VNET_RESETTIME		   0x000AFC80  = 840 minuti
		impostarlo a  0x85ED for 40 minutes and 0x42F6 for 20 minutes
		In Sketch

***************************************************************************/

#define SLEEPING_H						//Disabling Sleeping Library

#define	VNET_DEBUG_INSKETCH
#define VNET_DEBUG  		1

#define	MaCaco_DEBUG_INSKETCH
#define MaCaco_DEBUG  		1

#define USART_DEBUG  			0		//Disabilito il Debug

#define USARTDRIVER_INSKETCH
#define	USARTDRIVER				Serial1	//Dico al driver vNet di usare la seriale 3 del Mega la Serial1
#define USART_TXENABLE			0		//Non  uso il TX Enable con i moduli Ciseco

#define USARTBAUDRATE_INSKETCH
#define	USART_BAUD57k6			1
#define USART_BAUD115k2			0

//--\conf\frame\vNetCfg.h
#define	VNET_RESETTIME_INSKETCH
#define VNET_RESETTIME		   0x85ED


// Configure the framework
#include "bconf/StandardArduino.h"			// Use a standard Arduino
#include "conf/ethW5100.h"					// Ethernet through Wiznet W5100
#include "conf/nRF24L01.h"
#include "conf/usart.h"
#include "conf/Gateway_wPersistence.h"		// The main node is the Gateway
#include "conf/XMLinterface.h"
//#include "conf/DynamicAddressing.h"         // Use dynamic address


// Include framework code and libraries
#include <SPI.h>
#include "Souliss.h"
#include <EEPROM.h>
#include "Souliss.h"
#include <Voltmetro.h>

// Define the network configuration according to your router settings
uint8_t ip_address[4]  = {192, 168, 1, 129};
uint8_t subnet_mask[4] = {255, 255, 255, 0};
uint8_t ip_gateway[4]  = {192, 168, 1, 1};

// Define the network configuration
#define eth_address				ip_address[3]	// The last byte of the IP address (129) is also the vNet address
#define usart_address			0xCE01			// Indirizzo Locale vNet della Seriale
#define nrf24_address			0x6501			// Indirizzo locale interfaccia nrf24
#define caldaia_address			0xCE02			// Indirizzo scheda caldaia remota Seriale
#define ingresso_address		0xCE03			// Indirizzo scheda Ingresso remota Seriale
#define powersocket2_address	0x6502			// Indirizzo del Power Socket 2
#define powersocket3_address	0x6503			// Indirizzo del Power Socket 3
#define powersocket4_address	0x6504			// Indirizzo del Power Socket 4

#define myvNet_subnet		0xFF00
#define myvNet_supern		0x0000


//Pin Assignment
#define PIN_RELE_1		22
#define PIN_RELE_2		23
#define PIN_DIGIN_1		24
#define PIN_DIGIN_2		25
#define PIN_DIGIN_3		26
#define PIN_ADC_1		8	//ANALOG 8
#define PIN_ADC_2		9	//ANALOG 9

//NAS
#define NAS1_Switch		40
#define NAS1_Rele		41
#define NAS1_PwrSw		42
#define PIN_PC_RST		43
#define PIN_PC_PWR		44

//Souliss SLOT
#define T_RELE_1		0
#define T_RELE_2		1
#define T_DIGIN_1		2
#define T_DIGIN_2		3
#define T_DIGIN_3		4
#define T_ADC_1			5
#define T_ADC_2			7


#define NASCTL01_On		0	//T11 per comandare l'accensione del NAS
#define NASCTL01_Off	1	//T11 per comandare l'accensione del NAS
#define PC_RST_SECURE	4	//T11 per attivare il T14 che resetta il PC
#define PC_RST_RELE		5	//T14 che azione il relè che resetta il PC
#define PC_PWR_RELE		6	//T14 che aziona il Pulsante di accensione del PC


// Variabili generali
//Variabili che gestiscono l'accensione del NAS1
long nas1_state = 0;
long nas1_count = 0;

//Variabili che Gestiscono il Voltmetro
// Voltmetro(pin,R1,R2.VRef)
Voltmetro voltmt1(8,47000.0,8200.0,1.10); //4.80
float v_voltmt1;
Voltmetro voltmt2(9,22000.0,10000.0,1.10); //4.80
float v_voltmt2;


#define DEADBAND      0.01 //Se la variazione è superio del 1% aggiorno
#define NODEADBAND	  0 //Se la variazione è superio del 0,1% aggiorno


void setup()
{	
	Serial.begin(115200);
	Serial.println("Gateway INIT");

	pinMode(A1,INPUT);
	analogReference(INTERNAL1V1);

	//Initialize();

	// Setup the network configuration
	Souliss_SetIPAddress(ip_address, subnet_mask, ip_gateway);
	SetAsGateway(eth_address);		//Set this node as gateway for SoulissApp	
	Souliss_SetAddress(eth_address, myvNet_subnet, myvNet_supern);		
	Souliss_SetAddress(usart_address, myvNet_subnet, myvNet_supern);
	Souliss_SetAddress(nrf24_address, myvNet_subnet, myvNet_supern);

	Souliss_SetRemoteAddress(memory_map, caldaia_address,1);
	Souliss_SetRemoteAddress(memory_map, ingresso_address,2);
	Souliss_SetRemoteAddress(memory_map, powersocket2_address,3);
	Souliss_SetRemoteAddress(memory_map, powersocket3_address,4);
	Souliss_SetRemoteAddress(memory_map, powersocket4_address,5);

	//Pin Mode
	pinMode(PIN_RELE_1, OUTPUT);
	pinMode(PIN_RELE_2, OUTPUT);
	pinMode(PIN_DIGIN_1, INPUT);
	pinMode(PIN_DIGIN_2, INPUT);
	pinMode(PIN_DIGIN_3, INPUT);
	pinMode(PIN_ADC_1, INPUT);
	pinMode(PIN_ADC_2, INPUT);
	
	//Tipici
	Souliss_SetT11(memory_map, T_RELE_1);
	Souliss_SetT11(memory_map, T_RELE_2);
	Souliss_SetT13(memory_map, T_DIGIN_1);
	Souliss_SetT13(memory_map, T_DIGIN_2);
	Souliss_SetT13(memory_map, T_DIGIN_3);
	Souliss_SetT55(memory_map, T_ADC_1);
	Souliss_SetT55(memory_map, T_ADC_2);


	/*
	//Tipico T14 per il controllo del NAS 1
	Souliss_SetT14(memory_map, NASCTL01_On);		//Tipico T11 per il controllo del NAS
	Souliss_SetT14(memory_map, NASCTL01_Off);		//Tipico T11 per il controllo del NAS

	// Tipici T11 e T14 per la gestione del reset del PC
	pinMode(PIN_PC_RST, OUTPUT);
	Souliss_SetT11(memory_map, PC_RST_SECURE);
	Souliss_SetT14(memory_map, PC_RST_RELE);
	Souliss_SetT11(memory_map, PC_PWR_RELE);

	//Inizializzo il server HTML, usato solo per il servomotore
	XMLSERVERInit(memory_map);

	//------NAS
	pinMode(NAS1_Switch, INPUT);      // Pulsante HW NAS01
	pinMode(NAS1_Rele, OUTPUT);     // Rele NAS01
	pinMode(NAS1_PwrSw, OUTPUT);     // Rele NAS01
	*/

	data_changed = 0;
}

void loop()
{ 
	EXECUTEFAST() {						
		UPDATEFAST();	
		FAST_30ms() {

			//Uscite a Relè
			Souliss_DigOut(PIN_RELE_1, Souliss_T1n_Coil, memory_map, T_RELE_1);
			Souliss_DigOut(PIN_RELE_2, Souliss_T1n_Coil, memory_map, T_RELE_2);

			/*
			// Logica di controllo del NAS
			Souliss_Logic_T14(memory_map, NASCTL01_On, &data_changed);
			Souliss_DigOutNAS1_On(Souliss_T1n_OnCoil, memory_map,NASCTL01_On);

			Souliss_Logic_T14(memory_map, NASCTL01_Off, &data_changed);
			Souliss_DigOutNAS1_Off(Souliss_T1n_OnCoil, memory_map,NASCTL01_Off);

			//Logica per controllare il rele del reset e del power del PC
			Souliss_DigOut(PIN_PC_RST, Souliss_T1n_Coil, memory_map, PC_RST_RELE);
			Souliss_DigOut(PIN_PC_PWR, Souliss_T1n_Coil, memory_map, PC_PWR_RELE);
			*/
		}
		FAST_50ms() {
			// Retreive data from the MaCaco communication channel
            Souliss_CommunicationData(memory_map, &data_changed);
		}

		FAST_70ms() {
			XMLSERVERInterface(memory_map);
		}

		FAST_90ms() {
			// Logica per controllare i 2 VOLTMETRI
			Souliss_Logic_T55(memory_map, T_ADC_1, DEADBAND, &data_changed);
			Souliss_Logic_T55(memory_map, T_ADC_2, DEADBAND, &data_changed);

			// Esegui Logic per la linea FAN
			Souliss_Logic_T11(memory_map, T_RELE_1, &data_changed);
			Souliss_Logic_T11(memory_map, T_RELE_2, &data_changed);


			
			//Logica T11 per l'attivazione del Reset del PC
			//Souliss_Logic_T11(memory_map, PC_RST_SECURE, &data_changed);

			//Logica T11 per gestire il pulsante power del PC
			//Che non viene attivato se la sicura non è armata
			//Souliss_Logic_T11(memory_map, PC_PWR_RELE, &data_changed);
			//if (!mOutput(PC_RST_SECURE)) mOutput(PC_PWR_RELE) = 0;

		}

		FAST_110ms() {
            // Get logic typicals once and at every refresh
            Souliss_GetTypicals(memory_map);

			//Logiche per gestire gli ingressi Digitali
			Souliss_LowDigIn2State(PIN_DIGIN_1,Souliss_T1n_OnCmd,Souliss_T1n_OffCmd,memory_map,T_DIGIN_1);
			Souliss_LowDigIn2State(PIN_DIGIN_2,Souliss_T1n_OnCmd,Souliss_T1n_OffCmd,memory_map,T_DIGIN_2);
			Souliss_LowDigIn2State(PIN_DIGIN_3,Souliss_T1n_OnCmd,Souliss_T1n_OffCmd,memory_map,T_DIGIN_3);
			

			
		}

		FAST_510ms() {
			// Open a communication channel with remote nodes
            Souliss_CommunicationChannels(memory_map);

			// Esegui la Logica per gli ingressi Digitali
			Souliss_Logic_T13(memory_map, T_DIGIN_1, &data_changed);
			Souliss_Logic_T13(memory_map, T_DIGIN_2, &data_changed);
			Souliss_Logic_T13(memory_map, T_DIGIN_3, &data_changed);


			//NAS01_Timing();
		}

		FAST_2110ms() {
			
			//Lascio il pulsante di reset premuto per 2 secondi se la sicura è armata
			//Souliss_Logic_T14(memory_map, PC_RST_RELE, &data_changed);
			//if (!mOutput(PC_RST_SECURE)) mOutput(PC_RST_RELE) = 0;
		}

	}
	EXECUTESLOW() {
		UPDATESLOW();
		SLOW_10s() {
			
			//Leggo il voltmetro e aggiorno lo slot
			v_voltmt1 = voltmt1.getVoltage();
			Souliss_ImportAnalog(memory_map, T_ADC_1, &v_voltmt1);

			//Leggo il voltmetro e aggiorno lo slot
			v_voltmt2 = voltmt2.getVoltage();
			Souliss_ImportAnalog(memory_map, T_ADC_2, &v_voltmt2);

		}
		SLOW_510s() {
		} 
	}		
	//FAST_GatewayComms();
} 

void My_Set_Aux(U8 value, U8 *memory_map, U8 slot) {
	memory_map[MaCaco_AUXIN_s + slot] = value;
}

void Souliss_DigOutNAS1_On(U8 value, U8 *memory_map, U8 slot)
{
	//Serial.print("DigOutNAS st=");
	//Serial.println(nas1_state);

		// Routine di Gestione uscita digitale, modificata per la gestione di un NAS Promise
		if(memory_map[MaCaco_OUT_s + slot] == value) {
			// Procedura di ON
			// Devo dare corrente e premere il pulsante di ON del NAS per circa un secondo
			digitalWrite(NAS1_Rele, HIGH); //Do Corrente al NAS
			digitalWrite(NAS1_PwrSw, HIGH); //Premo il pulsante Power On del NAS
			nas1_state = 1;	//Imposto lo stato del pulsante a premuto
			//Serial.print("DigOutNAS ON st=");
			//Serial.println(nas1_state);	
		}

}



void Souliss_DigOutNAS1_Off(U8 value, U8 *memory_map, U8 slot)
{
	//Serial.print("DigOutNAS st=");
	//Serial.println(nas1_state);

		// Routine di Gestione uscita digitale, modificata per la gestione di un NAS Promise
		if(memory_map[MaCaco_OUT_s + slot] == value) {
			// Procedura di OFF
			// Devo premere il pulsante di off per 6 secondi, e togliere corrente dopo 30 secondi
			digitalWrite(NAS1_PwrSw, HIGH); //Premo il pulsante Power On del NAS
			nas1_state = 2;	//Imposto lo stato del pulsante a premuto
			//Serial.print("DigOutNAS OFF st=");
			//Serial.println(nas1_state);
	
		}

}



void NAS01_Timing() {
	//Serial.print("Timing st=");
	//Serial.println(nas1_state);

	if (nas1_state == 1) {
		nas1_count++;	//Se il pulsante � premuto incremento il contatore
		if (nas1_count == 2) {	//Se il contatore � a 2, 510ms * 2 Volte= 1,2 Secondo circa
			digitalWrite(NAS1_PwrSw, LOW); // Rilascio il Pulsante del NAS
			nas1_state = 0;	//Imposto lo stato del pulsante a NON premuto
			nas1_count = 0; //Resetto il contatore
		}
		//Serial.print("Timing st=");
		//Serial.println(nas1_state);
		//Serial.println(nas1_count);
	}	
	if (nas1_state == 2) {
		nas1_count++;	//Se il pulsante � premuto incremento il contatore
		if (nas1_count == 12) {	//Se il contatore � a 12, 510ms * 12 Volte= 6 Secondi circa
			digitalWrite(NAS1_PwrSw, LOW); // Rilascio il Pulsante del NAS
			// Ma continuo a contare fino a 30 secondi dopo di che stacco il power socket
		}
		if (nas1_count == 59) {	//Se il contatore � a 59, 110ms * 59 Volte= 30 Secondi circa
			digitalWrite(NAS1_Rele, LOW); // Tolgo corrente al NAS
			nas1_state = 0;	//Imposto lo stato del pulsante a NON premuto
			nas1_count = 0; //Resetto il contatore
		}
		//Serial.print("Timing st=");
		//Serial.println(nas1_state);
		//Serial.println(nas1_count);
	}
}
