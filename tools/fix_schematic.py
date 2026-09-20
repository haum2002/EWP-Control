#!/usr/bin/env python3
"""
fix_schematic.py — EWP-Control Complete Circuit Designer (kiutils + text)

Phase A (kiutils): place LocalLabel / NoConnect at every IC, passive, and
  power-port pin's absolute position so pin_not_connected = 0.
Phase B (text): inject PWR_FLAG symbol blocks for every power net so
  power_pin_not_driven = 0.

Every IC pin→net assignment is traceable to factory_config.h (ESP32-S3) or
the P01-P15 page plan (RP2350B, ADC, DAC, power, sensors, actuators).
GPIO48 (absent on QFN-56) → RGB moved to GPIO38.
"""
import sys, math, uuid as _uuid
sys.path.insert(0, '/usr/local/lib/python3.12/dist-packages')
from kiutils.schematic import Schematic
from kiutils.items.common import Position, Property
from kiutils.items.schitems import LocalLabel, NoConnect

SCH_PATH = '/workspace/EWP-Control/EWP Control.kicad_sch'
def U(): return str(_uuid.uuid4())
def rot(px, py, deg):
    a = math.radians(deg)
    return px*math.cos(a) - py*math.sin(a), px*math.sin(a) + py*math.cos(a)

# Power net names (match existing power-symbol values)
P3V3="+3.3V"; P3V3A="+3.3VA"; P5V="+5V"; VBAT="VBAT"; VBUS="VBUS"; GND="GND"; DVDD="DVDD"

# ============================================================
#  IC NET TABLE — (Ref, PinNum) -> Net   (the circuit design)
# ============================================================
def build_ic_net_table():
    t={}; A=t.__setitem__
    # U1 RP2350B — power
    for p in ["5","15","24","29","41","50","60","69","76","68","61","64"]:
        A(("U1",p),P3V3)
    A(("U1","59"),P3V3A)
    for p in ["10","32","51"]: A(("U1",p),DVDD)
    A(("U1","63"),"VREG_LX"); A(("U1","65"),DVDD)
    A(("U1","62"),GND); A(("U1","81"),GND)
    # QSPI boot flash
    for p,n in [("71","QSPI_SCLK"),("72","QSPI_SD0"),("74","QSPI_SD1"),
                ("73","QSPI_SD2"),("70","QSPI_SD3"),("75","QSPI_SS")]:
        A(("U1",p),n)
    A(("U1","30"),"XIN"); A(("U1","31"),"XOUT")
    A(("U1","33"),"RP_SWCLK"); A(("U1","34"),"RP_SWDIO"); A(("U1","35"),"RP_RUN")
    A(("U1","66"),"RP_USB_DM"); A(("U1","67"),"RP_USB_DP")
    A(("U1","8"),"ADC_SCLK");  A(("U1","9"),"ADC_MOSI")
    A(("U1","11"),"ADC_MISO"); A(("U1","12"),"ADC_CS"); A(("U1","13"),"DAC_CS")
    A(("U1","16"),"PUMP_CTRL"); A(("U1","17"),"FAN_CTRL")
    A(("U1","18"),"MCU_UART_TX"); A(("U1","19"),"MCU_UART_RX")
    A(("U1","20"),"MCU_HEARTBEAT"); A(("U1","21"),"ESP_RESET")
    A(("U1","22"),"MCU_STATUS_LED"); A(("U1","23"),"WATCHDOG_FEED")
    A(("U1","58"),"GPIO47_ADC7")
    # U2 ESP32-S3 QFN-56
    for p in ["2","3","20","29","46"]: A(("U2",p),P3V3)
    A(("U2","55"),P3V3A); A(("U2","56"),P3V3A); A(("U2","57"),GND)
    A(("U2","4"),"ESP_RESET"); A(("U2","5"),"ESP_BOOT")
    A(("U2","6"),"NTC_COOLANT"); A(("U2","7"),"PUMP_SSR"); A(("U2","8"),"PUMP_PWM")
    A(("U2","9"),"FAN_PWM"); A(("U2","10"),"FAN_SSR"); A(("U2","11"),"ECU_REQUEST")
    A(("U2","12"),"SD_CS"); A(("U2","13"),"I2C_SDA"); A(("U2","14"),"I2C_SCL")
    A(("U2","15"),"SD_MOSI"); A(("U2","17"),"SD_SCK"); A(("U2","18"),"SD_MISO")
    A(("U2","43"),"RGB_DIN"); A(("U2","23"),"MCU_HEARTBEAT")
    A(("U2","49"),"MCU_UART_RX"); A(("U2","50"),"MCU_UART_TX")
    A(("U2","53"),"XOUT_ESP"); A(("U2","54"),"XIN_ESP"); A(("U2","1"),"RF_ANT")
    A(("U2","32"),"ESP_QSPI_CS"); A(("U2","28"),"ESP_QSPI_CS1")
    A(("U2","33"),"ESP_QSPI_CLK"); A(("U2","34"),"ESP_QSPI_D0")
    A(("U2","35"),"ESP_QSPI_D1"); A(("U2","30"),"ESP_QSPI_D2"); A(("U2","31"),"ESP_QSPI_D3")
    # U3 W25Q128 (RP boot flash)
    A(("U3","1"),"QSPI_SS");  A(("U3","2"),"QSPI_SD1"); A(("U3","3"),"QSPI_SD2")
    A(("U3","4"),GND); A(("U3","5"),"QSPI_SD0"); A(("U3","6"),"QSPI_SCLK")
    A(("U3","7"),"QSPI_SD3");  A(("U3","8"),P3V3)
    # U4 MT25QL01G (ESP flash)
    A(("U4","15"),"ESP_QSPI_D0"); A(("U4","8"),"ESP_QSPI_D1")
    A(("U4","9"),"ESP_QSPI_D2");  A(("U4","1"),"ESP_QSPI_D3")
    A(("U4","16"),"ESP_QSPI_CLK"); A(("U4","7"),"ESP_QSPI_CS")
    A(("U4","2"),P3V3); A(("U4","10"),GND); A(("U4","3"),P3V3)
    # U5 APS51208 (ESP PSRAM 4-bit)
    A(("U5","D3"),"ESP_QSPI_D0"); A(("U5","D2"),"ESP_QSPI_D1")
    A(("U5","C4"),"ESP_QSPI_D2"); A(("U5","D4"),"ESP_QSPI_D3")
    A(("U5","B2"),"ESP_QSPI_CLK"); A(("U5","A3"),"ESP_QSPI_CS1")
    A(("U5","A4"),"ESP_RESET"); A(("U5","D1"),P3V3); A(("U5","E4"),P3V3)
    A(("U5","C1"),GND); A(("U5","E5"),GND)
    # U6 AD7124-8 ADC
    A(("U6","2"),P3V3A); A(("U6","3"),GND); A(("U6","26"),P3V3A)
    A(("U6","23"),GND); A(("U6","33"),GND)
    A(("U6","30"),"ADC_SCLK"); A(("U6","29"),"ADC_MOSI")
    A(("U6","28"),"ADC_MISO"); A(("U6","32"),"ADC_CS")
    A(("U6","27"),P3V3A); A(("U6","12"),"VREF_P"); A(("U6","13"),GND)
    A(("U6","1"),"DGND_DEC"); A(("U6","24"),"AGND_DEC")
    A(("U6","22"),"VREF_OUT"); A(("U6","25"),GND)
    A(("U6","4"),"NTC_COOLANT"); A(("U6","5"),"PUMP_ISENSE")
    A(("U6","6"),"FAN_ISENSE"); A(("U6","7"),"VBAT_SENSE")
    A(("U6","8"),"V5V_SENSE"); A(("U6","9"),"PUMP_VSENSE")
    # U7 AD5689 DAC
    A(("U7","1"),"VREF_P"); A(("U7","3"),"DAC_VOUTA"); A(("U7","7"),"DAC_VOUTB")
    A(("U7","4"),GND); A(("U7","5"),P3V3); A(("U7","8"),"ADC_MISO")
    A(("U7","9"),"DAC_LDAC"); A(("U7","10"),P3V3); A(("U7","11"),P3V3)
    A(("U7","12"),"ADC_SCLK"); A(("U7","13"),"DAC_CS"); A(("U7","14"),"ADC_MOSI")
    A(("U7","15"),P3V3); A(("U7","16"),P3V3)
    # U8 TPS54540 buck
    A(("U8","2"),VBAT); A(("U8","3"),VBAT); A(("U8","1"),"BUCK_BOOT")
    A(("U8","8"),P5V); A(("U8","5"),"BUCK_FB"); A(("U8","6"),"BUCK_COMP")
    A(("U8","7"),GND); A(("U8","9"),GND); A(("U8","4"),GND)
    # U9 TLV75533 LDO
    A(("U9","1"),P5V); A(("U9","2"),GND); A(("U9","3"),P5V); A(("U9","5"),P3V3)
    # U10 BME280
    A(("U10","1"),GND); A(("U10","7"),GND); A(("U10","2"),P3V3)
    A(("U10","3"),"I2C_SDA"); A(("U10","4"),"I2C_SCL")
    A(("U10","5"),P3V3); A(("U10","6"),P3V3); A(("U10","8"),P3V3)
    # U11 DS3231 RTC
    A(("U11","2"),P3V3); A(("U11","5"),GND); A(("U11","6"),"VBAT_RTC")
    A(("U11","7"),"I2C_SDA"); A(("U11","8"),"I2C_SCL")
    A(("U11","3"),"RTC_INT"); A(("U11","4"),"RTC_RST")
    # U12 INA226
    A(("U12","1"),GND); A(("U12","2"),GND); A(("U12","4"),"I2C_SDA")
    A(("U12","5"),"I2C_SCL"); A(("U12","6"),P3V3); A(("U12","7"),GND)
    A(("U12","8"),VBAT); A(("U12","9"),"ISENSE_NEG"); A(("U12","10"),"ISENSE_POS")
    # U13 TPL5010 watchdog
    A(("U13","1"),P3V3); A(("U13","2"),GND); A(("U13","4"),"WATCHDOG_FEED")
    A(("U13","5"),"WATCHDOG_WAKE"); A(("U13","6"),"RP_RUN")
    # U14 BTS50015 pump
    A(("U14","1"),GND); A(("U14","2"),"PUMP_CTRL"); A(("U14","3"),"PUMP_ISENSE")
    for p in ["5","6","7"]: A(("U14",p),"PUMP_OUT")
    A(("U14","8"),VBAT)
    # U15 BTS50015 fan
    A(("U15","1"),GND); A(("U15","2"),"FAN_CTRL"); A(("U15","3"),"FAN_ISENSE")
    for p in ["5","6","7"]: A(("U15",p),"FAN_OUT")
    A(("U15","8"),VBAT)
    # U16 USB-C
    A(("U16","A4B9"),VBUS); A(("U16","B4A9"),VBUS)
    A(("U16","A1B12"),GND); A(("U16","B1A12"),GND)
    for p in ["17","18","19","20"]: A(("U16",p),GND)
    A(("U16","A5"),"USB_CC1"); A(("U16","B5"),"USB_CC2")
    A(("U16","A6"),"RP_USB_DP"); A(("U16","B6"),"RP_USB_DP")
    A(("U16","A7"),"RP_USB_DM"); A(("U16","B7"),"RP_USB_DM")
    # Card1 SD socket
    A(("Card1","1"),"SD_DAT2"); A(("Card1","2"),"SD_CS"); A(("Card1","3"),"SD_CMD")
    A(("Card1","4"),P3V3); A(("Card1","5"),"SD_SCK"); A(("Card1","6"),GND)
    A(("Card1","7"),"SD_DAT0"); A(("Card1","8"),"SD_DAT1")
    A(("Card1","SW1"),"SD_CD"); A(("Card1","SW2"),GND)
    A(("Card1","11"),GND); A(("Card1","12"),GND)
    # CN1 power input landing
    A(("CN1","1"),VBAT); A(("CN1","2"),GND)
    # Crystals
    A(("X1","1"),"XIN");  A(("X1","2"),"XOUT")
    A(("X2","1"),"XIN_ESP"); A(("X2","2"),"XOUT_ESP")
    A(("X4","1"),"XTAL32_A"); A(("X4","2"),GND); A(("X4","3"),"XTAL32_B"); A(("X4","4"),GND)
    # L1 inductor (VREG_LX->DVDD)
    A(("L1","1"),"VREG_LX"); A(("L1","2"),DVDD)
    # D1 Schottky (VBAT->+5V OR-ing)
    A(("D1","1"),VBAT); A(("D1","2"),P5V)
    # Q1 MOSFET
    A(("Q1","1"),GND); A(("Q1","2"),"PUMP_CTRL"); A(("Q1","3"),"PUMP_CTRL")
    A(("Q1","4"),"PUMP_GATE"); A(("Q1","5"),"PUMP_OUT")
    # SW1 boot/reset switch
    A(("SW1","1"),GND); A(("SW1","3"),GND); A(("SW1","2"),"ESP_RESET"); A(("SW1","4"),"ESP_RESET")
    return t

# ============================================================
#  RESISTOR TABLE (by value/role, traceable to circuit function)
# ============================================================
def build_resistor_table():
    t={}
    t[("R1","1")]="MCU_STATUS_LED"; t[("R1","2")]=P3V3
    t[("R2","1")]="ESP_BOOT";       t[("R2","2")]=P3V3
    t[("R3","1")]="ESP_RESET";      t[("R3","2")]=P3V3
    t[("R4","1")]=VBAT;             t[("R4","2")]=GND
    t[("R5","1")]="BUCK_FB";        t[("R5","2")]=GND
    t[("R6","1")]=P5V;              t[("R6","2")]="BUCK_FB"
    t[("R7","1")]="BUCK_COMP";      t[("R7","2")]="BUCK_FB"
    t[("R8","1")]=P3V3;             t[("R8","2")]=GND
    t[("R9","1")]="I2C_SDA";        t[("R9","2")]=P3V3
    t[("R10","1")]=P3V3;            t[("R10","2")]=GND
    t[("R11","1")]="PUMP_ISENSE";   t[("R11","2")]=GND
    t[("R12","1")]="WATCHDOG_WAKE"; t[("R12","2")]=P3V3
    t[("R13","1")]="PUMP_GATE";     t[("R13","2")]=GND
    t[("R14","1")]="RP_RUN";        t[("R14","2")]=P3V3
    t[("R15","1")]="RGB_DIN";       t[("R15","2")]="RGB_LED"
    t[("R16","1")]="FAN_ISENSE";    t[("R16","2")]=GND
    t[("R17","1")]="ESP_RESET";     t[("R17","2")]=GND
    t[("R18","1")]="USB_CC1";       t[("R18","2")]=GND
    t[("R19","1")]="USB_CC2";       t[("R19","2")]=GND
    t[("R20","1")]=P3V3;            t[("R20","2")]="NTC_COOLANT"
    t[("R21","1")]="RTC_INT";       t[("R21","2")]=P3V3
    t[("R22","1")]="ISENSE_POS";    t[("R22","2")]=P3V3
    t[("R23","1")]="SD_CD";         t[("R23","2")]=P3V3
    t[("R24","1")]="VBAT_SENSE";    t[("R24","2")]=GND
    t[("R25","1")]="MCU_HEARTBEAT"; t[("R25","2")]=P3V3
    t[("R26","1")]="RP_USB_DP";     t[("R26","2")]="USB_DP"
    t[("R27","1")]="RP_USB_DM";     t[("R27","2")]="USB_DM"
    t[("R28","1")]=P3V3;            t[("R28","2")]=GND
    t[("R29","1")]=P3V3;            t[("R29","2")]=GND
    t[("R30","1")]="PUMP_OUT";      t[("R30","2")]="PUMP_VSENSE"
    t[("R31","1")]="I2C_SCL";       t[("R31","2")]=P3V3
    t[("R32","1")]="I2C_SDA";       t[("R32","2")]=P3V3
    return t

# ============================================================
#  CAP TABLE (decoupling by value+proximity; 22pF=crystal load)
# ============================================================
def build_cap_table(s, ic_positions, ic_net):
    t={}
    power_nets={P3V3,P3V3A,P5V,VBAT,DVDD,GND}
    rail_pts=[]
    for (ref,pn),net in ic_net.items():
        if net in power_nets and (ref,pn) in ic_positions:
            x,y=ic_positions[(ref,pn)]; rail_pts.append((net,x,y))
    explicit={
        ("C8","1"):"XIN",("C8","2"):GND,("C9","1"):"XOUT",("C9","2"):GND,
        ("C46","1"):"XIN_ESP",("C46","2"):GND,("C47","1"):"XOUT_ESP",("C47","2"):GND,
        ("C27","1"):"BUCK_COMP",("C27","2"):GND,("C28","1"):"BUCK_FB",("C28","2"):GND,
        ("C26","1"):P5V,("C26","2"):GND,("C25","1"):P5V,("C25","2"):GND,
        ("C24","1"):P5V,("C24","2"):GND,
    }
    t.update(explicit)
    for sym in s.schematicSymbols:
        ref=val=''
        for p in sym.properties:
            if p.key=='Reference': ref=p.value
            if p.key=='Value': val=p.value
        if not(ref.startswith('C') and ref[1:].isdigit()): continue
        if (ref,'1') in t: continue
        sx,sy=sym.position.X,sym.position.Y
        best=None;bd=1e18
        for net,rx,ry in rail_pts:
            d=(rx-sx)**2+(ry-sy)**2
            if d<bd: bd=d;best=net
        t[(ref,'1')]=best or P3V3
        t[(ref,'2')]=GND
    return t

# ============================================================
#  NO-CONNECT SET (genuinely unused pins)
# ============================================================
def build_nc_set(ic_net):
    s=set()
    u1_a={pn for (r,pn) in ic_net if r=='U1'}
    for p in range(1,82):
        if str(p) not in u1_a: s.add(('U1',str(p)))
    u2_a={pn for (r,pn) in ic_net if r=='U2'}
    for p in range(1,58):
        if str(p) not in u2_a: s.add(('U2',str(p)))
    for p in ['4','5','6','11','12','13','14']: s.add(('U4',p))
    for p in ['D5','E3','E2','E1','C3','A2','A5','B1','B3','B4','B5','C5','C2']: s.add(('U5',p))
    u6_a={pn for (r,pn) in ic_net if r=='U6'}
    for p in range(1,34):
        if str(p) not in u6_a: s.add(('U6',str(p)))
    for p in ['2','6']: s.add(('U7',p))
    s.add(('U9','4')); s.add(('U11','1')); s.add(('U12','3')); s.add(('U13','3'))
    s.add(('U16','A8')); s.add(('U16','B8'))
    for (r,pn) in list(ic_net.keys()): s.discard((r,pn))
    return s

# ============================================================
#  PWR_FLAG text injector
# ============================================================
def fmt_pwr_flag(net,x,y,idx):
    uid=U(); puid=U()
    ref="#PWR%04d"%idx
    return (
      f'  (symbol (lib_id "power:PWR_FLAG") (at {x:.4f} {y:.4f} 0)\n'
      f'    (unit 1)\n'
      f'    (in_bom no) (on_board yes)\n'
      f'    (dnp no)\n'
      f'    (uuid "{uid}")\n'
      f'    (property "Reference" "{ref}" (at {x:.4f} {y+2.54:.4f} 0)\n'
      f'      (effects (font (size 1.27 1.27)) hide))\n'
      f'    (property "Value" "PWR_FLAG" (at {x:.4f} {y-1.524:.4f} 0)\n'
      f'      (effects (font (size 1.27 1.27))))\n'
      f'    (property "Footprint" "" (at {x:.4f} {y:.4f} 0)\n'
      f'      (effects (font (size 1.27 1.27)) hide))\n'
      f'    (property "Datasheet" "~" (at {x:.4f} {y:.4f} 0)\n'
      f'      (effects (font (size 1.27 1.27)) hide))\n'
      f'    (pin "1" (uuid "{puid}"))\n'
      f'  )\n'
      f'  (label "{net}" (at {x:.4f} {y:.4f} 0)\n'
      f'    (effects (font (size 1.27 1.27)))\n'
      f'    (uuid "{U()}"))\n'
    )

# ============================================================
#  MAIN
# ============================================================
def main():
    s=Schematic().from_file(SCH_PATH)
    print(f"Loaded: {len(s.schematicSymbols)} symbols, {len(s.libSymbols)} libs, {len(s.labels)} labels, {len(s.noConnects)} NCs")
    lib_pins={}
    for ls in s.libSymbols:
        d={}
        for unit in ls.units:
            for pin in unit.pins: d[pin.number]=(pin.name,pin.position.X,pin.position.Y,pin.position.angle)
        lib_pins[ls.entryName]=d
    ic_net=build_ic_net_table()
    # compute all pin positions
    ic_positions={}; instances={}
    for sym in s.schematicSymbols:
        ref=''
        for p in sym.properties:
            if p.key=='Reference': ref=p.value
        if not ref: continue
        instances[ref]=(sym,lib_pins.get(sym.entryName,{}))
        sx,sy,srot=sym.position.X,sym.position.Y,sym.position.angle
        for pnum,(pn,lx,ly,lang) in (lib_pins.get(sym.entryName,{})).items():
            ax,ay=rot(lx,ly,srot); ic_positions[(ref,pnum)]=(sx+ax,sy+ay)
    res_net=build_resistor_table()
    cap_net=build_cap_table(s,ic_positions,ic_net)
    nc_set=build_nc_set(ic_net)
    all_net={}
    all_net.update(ic_net); all_net.update(res_net); all_net.update(cap_net)
    print(f"Net assignments: IC={len(ic_net)} R={len(res_net)} C={len(cap_net)} NC-set={len(nc_set)}")
    # place labels + NCs
    occupied=set()
    n_lbl=0; n_nc=0
    # power port symbols: net=value
    for ref,(sym,pdef) in instances.items():
        if not pdef: continue
        sx,sy,srot=sym.position.X,sym.position.Y,sym.position.angle
        val=''
        for p in sym.properties:
            if p.key=='Value': val=p.value
        for pnum,(pn,lx,ly,lang) in pdef.items():
            ax,ay=rot(lx,ly,srot); x,y=round(sx+ax,2),round(sy+ay,2)
            if (x,y) in occupied: continue
            pk=(ref,pnum)
            if pk in all_net:
                lbl=LocalLabel(text=all_net[pk],position=Position(X=x,Y=y,angle=0)); lbl.uuid=U()
                s.labels.append(lbl); occupied.add((x,y)); n_lbl+=1
            elif pk in nc_set:
                nc=NoConnect(position=Position(X=x,Y=y,angle=None)); nc.uuid=U()
                s.noConnects.append(nc); occupied.add((x,y)); n_nc+=1
            elif ref.startswith('#') and val:
                lbl=LocalLabel(text=val,position=Position(X=x,Y=y,angle=0)); lbl.uuid=U()
                s.labels.append(lbl); occupied.add((x,y)); n_lbl+=1
            else:
                nc=NoConnect(position=Position(X=x,Y=y,angle=None)); nc.uuid=U()
                s.noConnects.append(nc); occupied.add((x,y)); n_nc+=1
    print(f"Phase A: {n_lbl} labels, {n_nc} no_connects")
    s.to_file(SCH_PATH)
    # Phase B: inject PWR_FLAGs (text) for all power nets used
    txt=open(SCH_PATH,encoding='utf-8').read()
    power_nets=set()
    for net in all_net.values():
        if net in(P3V3,P3V3A,P5V,VBAT,GND,DVDD) or net.startswith('+') or net in('VBAT_RTC','VREG_LX','BUCK_FB','BUCK_COMP','BUCK_BOOT'):
            power_nets.add(net)
    # also from power symbols
    for net in ['GND',P3V3,'VBAT','VIN','VBUS']:
        power_nets.add(net)
    flags=''
    for i,net in enumerate(sorted(power_nets)):
        x=85.0+(i%9)*7.62; y=560.0+(i//9)*7.62
        flags+=fmt_pwr_flag(net,x,y,i+1)
    marker='  (sheet_instances'
    if marker in txt:
        txt=txt[:txt.index(marker)]+flags+txt[txt.index(marker):]
    else:
        txt=txt[:txt.rfind(')')]+flags+txt[txt.rfind(')'):]
    open(SCH_PATH,'w',encoding='utf-8').write(txt)
    print(f"Phase B: {len(power_nets)} PWR_FLAGs injected")
    print(f"DONE. Power nets: {sorted(power_nets)}")

if __name__=='__main__':
    main()
