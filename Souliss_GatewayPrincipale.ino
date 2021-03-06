/**************************************************************************
	--- Souliss Gateway Principale ---

		IP: 192.168.1.129

Modifiche a Souliss:

FRIARIELLO
--\souliss\frame\vNet\drivers\nRF24\src\nRF24L01.h
	#define RF_SETUP da 0x06 a 0x26

--\conf\frame\vNetCfg.h
		#define VNET_RESETTIME		   0x000AFC80  = 840 minuti
		impostarlo a  0x85ED for 40 minutes and 0x42F6 for 20 minutes
		In Sketch

***************************************************************************/

#define SLEEPING_H						//Disabling Sleeping Library

#define	VNET_DEBUG_INSKETCH
#define VNET_DEBUG  		0

#define	MaCaco_DEBUG_INSKETCH
#define MaCaco_DEBUG  		0

#define USART_DEBUG  			0		//Disabilito il Debug

#define USARTDRIVER_INSKETCH
#define	USARTDRIVER				Serial1	//Dico al driver vNet di usare la seriale 3 del Mega la Serial1
#define USART_TXENABLE			0		//Non  uso il TX Enable con i moduli Ciseco
#define USART_TXENPIN			3

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
//#include "conf/XMLinterface.h"

// Include framework code and libraries
#include <SPI.h>
#include "Souliss.h"
#include <EEPROM.h>
#include "Souliss.h"
#include <Voltmetro.h>
#include <OneWire.h>
#include <DallasTemperature.h>


// Define the network configuration according to your router settings
uint8_t ip_address[4]  = {192, 168, 1, 129};
uint8_t subnet_mask[4] = {255, 255, 255, 0};
uint8_t ip_gateway[4]  = {192, 168, 1, 1};

// Define the network configuration
#define eth_address					ip_address[3]	// The last byte of the IP address (129) is also the vNet address
#define usart_address				0xCE01			// Indirizzo Locale vNet della Seriale
#define nrf24_address				0x6501			// Indirizzo locale interfaccia nrf24
#define caldaia_address				0xCE02			// Indirizzo scheda caldaia remota Seriale
#define ingresso_address			0xCE03			// Indirizzo scheda Ingresso remota Seriale
#define powersocket2_address		0x0083			// IP 131 ex nrf 0x6502	- Indirizzo del Power Socket 2
#define powersocket3_address		0x0084			// IP 132 ex nrf 0x6503 - Indirizzo del Power Socket 3
#define powersocket4_address		0x0085			// IP 133 ex nrf 0x6504 - Indirizzo del Power Socket 4
#define powersocket5multi_address	0x0087			// IP 135 Power socket multi mansarda
#define bagnomansarda_address		0x0088			// IP 136 Bagno Mansarda

#define hvac1_address				0xCE04			// Condizionatori Camera Letto
#define hvac2_address				0xCE05			// Condizionatore Cucina

#define	RGB_1						0x0086			// IP 134 Modulo Luci RGB
#define sala_503					0x0089			// IP 137 Scatola 503 interruttori Sala
#define bosecontrol					0x008A			// IP 138 Controllo di Bose Sound Dock
#define technicscontrol				0x008B			// IP 139 Controllo dello stereo Technics in Mansarda
#define salotto_sx_503				0x008D			// IP 141 Scatola 503 Interruttori Salotto SX
#define salotto_dx_503				0x008F			// IP 143 Scatola 503 Interruttori Salotto DX
#define bagno_503					0x0090			// IP 144 Scatola 503 Interruttore Bagno

#define hvac_salotto				0x008C			// IP 140 Cond. Salotto e apricancello

//#define nrf24_dummy			0x6502			// Nodo NRF a batteria di Test
#define nrf24_dummy			0x008E				// IP 142

#define myvNet_subnet		0xFF00
#define myvNet_supern		0x0000

//Pin Assignment
#define PIN_RELE_1		22
#define PIN_RELE_2		23
#define PIN_DIGIN_1		24
#define PIN_DIGIN_2		28
#define PIN_DIGIN_3		26
#define PIN_RJ1_1		31
#define PIN_RJ1_2		30
#define PIN_RJ1_3		32
#define PIN_TEMP_1		33
#define PIN_TEMP_2		34
#define PIN_VOLT_1		A8
#define PIN_VOLT_2		A9

//NAS
#define NAS1_Switch		40
#define NAS1_Rele		41
#define NAS1_PwrSw		42


//Souliss SLOT
#define T_RELE_1		0
#define T_RELE_2		1
#define T_DIGIN_1		2
#define T_DIGIN_2		3
#define T_DIGIN_3		4
#define T_ADC_1			5	//e 6  -- 2Slot
#define T_ADC_2			7	//e 8  -- 2Slot
#define PC_ARM_BTN		9	//T11 Virtuale che serve per armare il pulsante di reset e di Power
#define T_RJ1_1			10
#define T_RJ1_2			11
#define T_RJ1_3			12
#define T_TEMP_1		13	//e 14 -- 2Slot
#define T_TEMP_2		15	//e 16 -- 2Slot
#define T_SOULISS_CHECK 17


/*
#define NASCTL01_On		0	//T11 per comandare l'accensione del NAS
#define NASCTL01_Off	1	//T11 per comandare l'accensione del NAS
#define PC_RST_SECURE	4	//T11 per attivare il T14 che resetta il PC
#define PC_RST_RELE		5	//T14 che azione il relè che resetta il PC
#define PC_PWR_RELE		6	//T14 che aziona il Pulsante di accensione del PC
*/

// Variabili generali
//Variabili che gestiscono l'accensione del NAS1
long nas1_state = 0;
long nas1_count = 0;

//Variabili che Gestiscono il Voltmetro
// Voltmetro(pin,R1,R2.VRef)
Voltmetro voltmt1(PIN_VOLT_1,47000.0,4700.0,3.33); //4.80
float v_voltmt1;
Voltmetro voltmt2(PIN_VOLT_2,22000.0,4700.0,3.33); //4.80
float v_voltmt2;

//Sonda Temperatura
#define TEMPERATURE_PRECISION	9	
//---T PC
float f_temp_1;
OneWire oneWire1(PIN_TEMP_1);
DallasTemperature sensors1(&oneWire1);
DeviceAddress insideThermometer1;
//---T Interna Gateway
float f_temp_2;
OneWire oneWire2(PIN_TEMP_2);
DallasTemperature sensors2(&oneWire2);
DeviceAddress insideThermometer2;


#define DEADBAND      0.01	//Se la variazione è superiore del 0.01% aggiorno
#define DEADBAND01	  0.1	//Se la variazione è superiore del 0.1% aggiorno		
#define NODEADBAND	  0		//Se la variazione è superiore del 0 aggiorno


void setup()
{	
	//Imposta gli stati delle porte con i relè a logica invertita
	digitalWrite(PIN_RJ1_1,HIGH);
	digitalWrite(PIN_RJ1_2,HIGH);

	Serial.begin(115200);
	Serial.println("Gateway INIT");

	analogReference(EXTERNAL);

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
	Souliss_SetRemoteAddress(memory_map, hvac1_address, 6);
	Souliss_SetRemoteAddress(memory_map, hvac2_address, 7);
	Souliss_SetRemoteAddress(memory_map, powersocket5multi_address, 8);
	Souliss_SetRemoteAddress(memory_map, bagnomansarda_address, 9);

	Souliss_SetRemoteAddress(memory_map, RGB_1, 10);
	Souliss_SetRemoteAddress(memory_map, nrf24_dummy, 11);	//Questo ID ha problemi con OH, ma va lasciato altrimenti non carica i nodi successivi
	Souliss_SetRemoteAddress(memory_map, sala_503, 12);
	Souliss_SetRemoteAddress(memory_map, bosecontrol, 13);
	Souliss_SetRemoteAddress(memory_map, technicscontrol, 14);
	Souliss_SetRemoteAddress(memory_map, hvac_salotto, 15);
	Souliss_SetRemoteAddress(memory_map, salotto_sx_503, 16);
	Souliss_SetRemoteAddress(memory_map, salotto_dx_503, 17);
	Souliss_SetRemoteAddress(memory_map, bagno_503, 18);


	//Pin Mode
	pinMode(PIN_VOLT_1,INPUT);
	pinMode(PIN_VOLT_2,INPUT);
	pinMode(PIN_RELE_1, OUTPUT);
	pinMode(PIN_RELE_2, OUTPUT);
	pinMode(PIN_DIGIN_1, INPUT);
	pinMode(PIN_DIGIN_2, INPUT);
	pinMode(PIN_DIGIN_3, INPUT);
	pinMode(PIN_RJ1_1,OUTPUT);
	pinMode(PIN_RJ1_2,OUTPUT);
	pinMode(PIN_RJ1_3,INPUT);
	pinMode(PIN_TEMP_1,INPUT);
	pinMode(PIN_TEMP_2, INPUT);


	//Tipici
	Souliss_SetT11(memory_map, T_RELE_1);	//Relè onboard 1
	Souliss_SetT11(memory_map, T_RELE_2);	//Rele onboard 2
	Souliss_SetT13(memory_map, T_DIGIN_1);	//Antifurto
	Souliss_SetT13(memory_map, T_DIGIN_2);	//220V
	Souliss_SetT13(memory_map, T_DIGIN_3);	//Sirena
	Souliss_SetT52(memory_map, T_ADC_1);	//UPS 30Volt
	Souliss_SetT52(memory_map, T_ADC_2);	//UPS 15 Volt
	Souliss_SetT11(memory_map, PC_ARM_BTN);	
	Souliss_SetT14(memory_map, T_RJ1_1);	//PC rele 1 - Reset
	Souliss_SetT11(memory_map, T_RJ1_2);	//PC rele 2	- Power
	Souliss_SetT13(memory_map, T_RJ1_3);	//PC Acceso
	Souliss_SetT52(memory_map, T_TEMP_1);	//Sonda Temperatura Interna PC
	Souliss_SetT52(memory_map, T_TEMP_2);	//Sonda Temperatura Gateway
	Souliss_SetT11(memory_map, T_SOULISS_CHECK);	//Tipico che serve a OpenHab per verificare la funzionalità di Souliss


	
	//Imposto la sonda DS18B20
	sensors1.setResolution(insideThermometer1, TEMPERATURE_PRECISION);
	sensors1.getAddress(insideThermometer1, 0);

	sensors2.setResolution(insideThermometer2, TEMPERATURE_PRECISION);
	sensors2.getAddress(insideThermometer2, 0);


	//Inizializzo il server HTML, usato solo per il servomotore
	//XMLSERVERInit(memory_map);

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

			Souliss_LowDigOut(PIN_RJ1_1, Souliss_T1n_Coil, memory_map, T_RJ1_1);	//La scheda è a logica invertita
			Souliss_LowDigOut(PIN_RJ1_2, Souliss_T1n_Coil, memory_map, T_RJ1_2);	//La scheda è a logica invertita

			/*
			// Logica di controllo del NAS
			Souliss_Logic_T14(memory_map, NASCTL01_On, &data_changed);
			Souliss_DigOutNAS1_On(Souliss_T1n_OnCoil, memory_map,NASCTL01_On);

			Souliss_Logic_T14(memory_map, NASCTL01_Off, &data_changed);
			Souliss_DigOutNAS1_Off(Souliss_T1n_OnCoil, memory_map,NASCTL01_Off);

			*/
		}
		FAST_50ms() {

		}

		FAST_70ms() {
			//XMLSERVERInterface(memory_map);
		}

		FAST_90ms() {
			// Logica per controllare i 2 VOLTMETRI
			Souliss_Logic_T52(memory_map, T_ADC_1, DEADBAND01, &data_changed);
			Souliss_Logic_T52(memory_map, T_ADC_2, DEADBAND01, &data_changed);
			//Logica per controllare le sonde di temperatura
			Souliss_Logic_T52(memory_map, T_TEMP_1, NODEADBAND, &data_changed);
			Souliss_Logic_T52(memory_map, T_TEMP_2, NODEADBAND, &data_changed);

			// Esegui Logic per controllare i T11
			Souliss_Logic_T11(memory_map, T_RELE_1, &data_changed);
			Souliss_Logic_T11(memory_map, T_RELE_2, &data_changed);

			Souliss_Logic_T11(memory_map, T_RJ1_1, &data_changed);
			Souliss_Logic_T11(memory_map, T_RJ1_2, &data_changed);

			Souliss_Logic_T11(memory_map, T_SOULISS_CHECK, &data_changed);
									
			//Logica T11 per l'attivazione del Reset e Power del PC
			Souliss_Logic_T11(memory_map, PC_ARM_BTN, &data_changed);

			//Inibizione del pulsante di Reset e di Power se il PC_ARM_BTN è in OFF
			if (!mOutput(PC_ARM_BTN)) mOutput(T_RJ1_2) = 0;
			if (!mOutput(PC_ARM_BTN)) mOutput(T_RJ1_1) = 0;
		}

		FAST_110ms() {
			//Leggo gli ingressi Digitali
			Souliss_LowDigIn2State(PIN_DIGIN_1,Souliss_T1n_OnCmd,Souliss_T1n_OffCmd,memory_map,T_DIGIN_1);
			Souliss_LowDigIn2State(PIN_DIGIN_2,Souliss_T1n_OnCmd,Souliss_T1n_OffCmd,memory_map,T_DIGIN_2);
			Souliss_LowDigIn2State(PIN_DIGIN_3,Souliss_T1n_OnCmd,Souliss_T1n_OffCmd,memory_map,T_DIGIN_3);
			Souliss_LowDigIn2State(PIN_RJ1_3,Souliss_T1n_OnCmd,Souliss_T1n_OffCmd,memory_map,T_RJ1_3);
			
		}

		FAST_510ms() {
			// Esegui la Logica per gli ingressi Digitali
			Souliss_Logic_T13(memory_map, T_DIGIN_1, &data_changed);
			Souliss_Logic_T13(memory_map, T_DIGIN_2, &data_changed);
			Souliss_Logic_T13(memory_map, T_DIGIN_3, &data_changed);
			Souliss_Logic_T13(memory_map, T_RJ1_3, &data_changed);

			//NAS01_Timing();
		}

		FAST_2110ms() {
			
			//Lascio il pulsante di reset premuto per 2 secondi se la sicura è armata
			Souliss_Logic_T14(memory_map, T_RJ1_1, &data_changed);
			if (!mOutput(PC_ARM_BTN)) mOutput(T_RJ1_1) = 0;

		}
		FAST_GatewayComms();
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

			//Leggo le sonde di temperatura
			sensors1.requestTemperatures();
			float f_temp_1 = sensors1.getTempC(insideThermometer1);
			Serial.print("Temp1:");
			Serial.println(f_temp_1);
			Souliss_ImportAnalog(memory_map, T_TEMP_1, &f_temp_1);

			sensors2.requestTemperatures();
			float f_temp_2 = sensors2.getTempC(insideThermometer2);
			Serial.print("Temp2:");
			Serial.println(f_temp_2);
			Souliss_ImportAnalog(memory_map, T_TEMP_2, &f_temp_2);

		}
		SLOW_510s() {
		} 
	}		
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
