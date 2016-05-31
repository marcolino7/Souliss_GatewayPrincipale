--- Gateway Principale

- Assegnazione PIN
7  - nrf24 CE
8  - nrf24 CSN
50  - nrf24 MISO
51  - nrf24 MOSI
52  - nrf24 SCK

22 - Relè OnBoard 1
23 - Relè OnBoard 2
24 - Dig IN 1 - 12 Volt - Stato Antifurto
25 - Dig IN 2 -  5 Volt - Presenza 220Volt
26 - Dig IN 3 - 12 Volt - Stato Sirena

A8 - Voltmento 1 30 Volt Max
A9 - Voltmento 2 15 Volt Max


- Indirizzi Souliss
eth_address				192.168.1.129	// The last byte of the IP address (129) is also the vNet address
usart_address			0xCE01			// Indirizzo Locale vNet della Seriale
nrf24_address			0x6501			// Indirizzo locale interfaccia nrf24
caldaia_address			0xCE02			// Indirizzo scheda caldaia remota Seriale
ingresso_address		0xCE03			// Indirizzo scheda Ingresso remota Seriale
powersocket2_address	0x6502			// Indirizzo del Power Socket 2
powersocket3_address	0x6503			// Indirizzo del Power Socket 3
powersocket4_address	0x6504			// Indirizzo del Power Socket 4

powersocket5_address	0x6504			// Indirizzo del Power Socket 4 IP

- Tipici / Slot

T11 - T_RELE_1			0
T11 - T_RELE_2			1
T13 - T_DIGIN_1			2
T13 - T_DIGIN_2			3
T13 - T_DIGIN_3			4
T55 - T_ADC_1			5
T55 - T_ADC_2			7

