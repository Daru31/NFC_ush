import spidev
import gpiod
import time 

# module setup
CE_PIN = 17
CSN = 0 # spidev0.0 (CSN0)
GPIOCHIP = "/dev/gpiochip0"

# nRF24 register definition - DO NOT CHANGE 
R_REGISTER = 0x00
W_REGISTER = 0x20
R_RX_PAYLOAD = 0x61
W_RX_ADDR_P0 = 0x0A
CONFIG = 0x00
EN_AA = 0x01
EN_RXADDR = 0x02
RF_CH = 0x05
RF_SETUP = 0x06
SETUP_RETR = 0x04
STATUS = 0x07
RX_PW_P0 = 0x11
FEATURE = 0x1D
DYNPD = 0x1C

# spidev setup
spi = spidev.SpiDev()
spi.open(0, CSN) # SPI0.0 
spi.max_speed_hz = 10000000 # 10MHz maximum 
spi.mode = 0

# preparing CE pin 
chip = gpiod.Chip(GPIOCHIP)
ce_line = chip.get_line(CE_PIN)  
ce_line.request(consumer="nrf24-ce", type=gpiod.LINE_REQ_DIR_OUT, default_vals=[0]) 

def ce_high(): 
    ce_line.set_value(1)  
def ce_low(): 
    ce_line.set_value(0)  

# util function 
def write_register(register, values):
    spi.xfer2([W_REGISTER | register] + values)

def read_register(register, length=1):
    return spi.xfer2([R_REGISTER | register] + [0x00] * length)[1:]

def flush_rx():
    spi.xfer2([0xE2]) # RX clearing (I'm not sure if it really works or not..)

# nrf24l01 setup
def nrf24_setup():
    ce_low()
    time.sleep(0.1)

    # default settings 
    write_register(CONFIG, [0x0F]) # PWR_UP=1, PRIM_RX=1
    write_register(EN_AA, [0x01]) # Auto-ACK enable on pipe 0
    write_register(EN_RXADDR, [0x01]) # Enable pipe 0
    write_register(SETUP_RETR, [0x00]) # No retransmit
    write_register(RF_CH, [0x76]) # Channel 0x76
    write_register(RF_SETUP, [0x06]) # 1Mbps, 0dBm
    write_register(RX_PW_P0, [32]) # Max payload size
    write_register(FEATURE, [0x00]) # Disable dynamic payloads
    write_register(DYNPD, [0x00]) # Disable dynamic payloads on pipe 0 
    write_register(STATUS, [0x70]) # STATUS clear 

    # RF address (Should be same with Arduino.)
    address = [0xE1, 0xF0, 0xF0, 0xF0, 0xF0] 
    write_register(W_RX_ADDR_P0, address)

    flush_rx()
    ce_high()
    time.sleep(0.1)

# Loop for receiving 
def listen():
    print("Waiting for RF signal..")
    while True:
        status = read_register(STATUS)[0]
        if status & 0x40:  # if data is successfully received 
            write_register(STATUS, [0x70])  # STATUS register clear to prepare next RF signal 
            payload = spi.xfer2([R_RX_PAYLOAD] + [0x00]*32)[1:]
            message = bytes(payload).split(b'\x00', 1)[0].decode(errors='ignore').strip() 
            print(message)
            flush_rx()
        time.sleep(0.2)

# run 
nrf24_setup()
listen()
