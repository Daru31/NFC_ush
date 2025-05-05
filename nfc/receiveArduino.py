import spidev
import gpiod
import time

# ====================
# 설정 부분
# ====================
CE_PIN = 17
CSN = 0  # spidev0.0 (CSN0)
GPIOCHIP = "/dev/gpiochip0"

# nRF24 레지스터/명령어 정의
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

# ====================
# SPI 초기화
# ====================
spi = spidev.SpiDev()
spi.open(0, CSN)  # SPI0.0 사용
spi.max_speed_hz = 10000000  # 최대 10MHz
spi.mode = 0

# ====================
# CE 핀 제어 준비 (gpiod 사용)
# ====================
chip = gpiod.Chip(GPIOCHIP)
ce_line = chip.get_line(CE_PIN)
ce_line.request(consumer="nrf24-ce", type=gpiod.LINE_REQ_DIR_OUT, default_vals=[0])

def ce_high(): ce_line.set_value(1)
def ce_low(): ce_line.set_value(0)

# ====================
# 유틸 함수
# ====================
def write_register(register, values):
    spi.xfer2([W_REGISTER | register] + values)

def read_register(register, length=1):
    return spi.xfer2([R_REGISTER | register] + [0x00] * length)[1:]

def flush_rx():
    spi.xfer2([0xE2])  # FLUSH_RX

# ====================
# nRF24 설정
# ====================
def nrf24_setup():
    ce_low()
    time.sleep(0.1)

    write_register(CONFIG, [0x0F])  # PWR_UP=1, PRIM_RX=1
    write_register(EN_AA, [0x01])
    write_register(EN_RXADDR, [0x01])
    write_register(SETUP_RETR, [0x00])
    write_register(RF_CH, [0x76])
    write_register(RF_SETUP, [0x06])
    write_register(RX_PW_P0, [32])
    write_register(FEATURE, [0x04])
    write_register(DYNPD, [0x01])

    # 수신 주소 (아두이노와 동일하게)
    address = [0xE1, 0xF0, 0xF0, 0xF0, 0xF0]
    write_register(W_RX_ADDR_P0, address)

    flush_rx()
    ce_high()
    time.sleep(0.1)

# ====================
# 수신 루프
# ====================
def listen():
    print("수신 시작...")
    while True:
        status = read_register(STATUS)[0]
        if status & 0x40:  # RX_DR
            print("📥 수신 데이터 도착")
            write_register(STATUS, [0x40])
            payload = spi.xfer2([R_RX_PAYLOAD] + [0x00] * 32)[1:]
            message = bytes(payload).rstrip(b'\x00').decode(errors='ignore')
            print("수신된 메시지:", message)
            flush_rx()
        elif status & 0x10:  # MAX_RT
            print("⚠️ 재전송 실패 (MAX_RT)")
            write_register(STATUS, [0x10])
            flush_rx()
        elif status & 0x20:  # TX_DS
            print("✅ 송신 완료 (TX_DS)")
            write_register(STATUS, [0x20])
        time.sleep(0.1)

# ====================
# 실행
# ====================
nrf24_setup()
listen()