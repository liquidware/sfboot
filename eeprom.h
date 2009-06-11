#ifndef EEPROM_H
#define EEPROM_H

void eepromInit(void);
int eepromWrite(uint16_t dataEEDest, uint8_t * const dataSrc, uint32_t length);
int eepromRead(uint8_t * dataDest, uint16_t dataEESrc, uint32_t length);

#endif