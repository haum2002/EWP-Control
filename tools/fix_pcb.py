#!/usr/bin/env python3
"""
fix_pcb.py — EWP-Control 4-layer PCB builder (kiutils)

  1. Re-annotate every pad net from the fresh schematic net table
  2. Add In1.Cu (GND plane) + In2.Cu (PWR plane) -> 4-layer stack
  3. Compact placement (functional groups, ~65x60 mm board)
  4. Redraw Edge.Cuts outline
  5. Pour GND zone on In1.Cu + PWR zones (+3.3V/+5V/VBAT) on In2.Cu
  6. Route power vias (every IC power pad -> inner plane) + critical
     signal buses (SPI / I2C / UART / QSPI)

User deliverable: "struktur + zon + penghalaan asas" (structure + zones + basic routing).
"""
import sys, math, uuid as _uuid
sys.path.insert(0, '/usr/local/lib/python3.12/dist-packages')
sys.path.insert(0, '/workspace/EWP-Control/tools')
from kiutils.board import Board
from kiutils.items.brditems import Segment, Via, LayerToken
from kiutils.items.gritems import GrLine
from kiutils.items.common import Position, Net
from kiutils.items.zones import Zone, ZonePolygon, FillSettings, Hatch
import fix_schematic as FS   # reuse the net-table builders

PCB = '/workspace/EWP-Control/EWP Control.kicad_pcb'
def U(): return str(_uuid.uuid4())
P3V3=FS.P3V3; P3V3A=FS.P3V3A; P5V=FS.P5V; VBAT=FS.VBAT; VBUS=FS.VBUS; GND=FS.GND; DVDD=FS.DVDD

# ============================================================
#  1. NET TABLE (reuse schematic builders)
# ============================================================
def build_all_nets():
    # need schematic to compute cap proximity (use schematic positions)
    from kiutils.schematic import Schematic as Sch
    sch=Sch().from_file(FS.SCH_PATH)
    lib_pins={}
    for ls in sch.libSymbols:
        d={}
        for unit in ls.units:
            for pin in unit.pins: d[pin.number]=(pin.name,pin.position.X,pin.position.Y,pin.position.angle)
        lib_pins[ls.entryName]=d
    ic_pos={}
    for sym in sch.schematicSymbols:
        ref=''
        for p in sym.properties:
            if p.key=='Reference': ref=p.value
        if not ref: continue
        sx,sy,srot=sym.position.X,sym.position.Y,sym.position.angle
        for pnum,(pn,lx,ly,lang) in (lib_pins.get(sym.entryName,{})).items():
            ax,ay=FS.rot(lx,ly,srot); ic_pos[(ref,pnum)]=(sx+ax,sy+ay)
    ic=FS.build_ic_net_table()
    rs=FS.build_resistor_table()
    caps=FS.build_cap_table(sch, ic_pos, ic)
    nc=FS.build_nc_set(ic)
    alln={}
    alln.update(ic); alln.update(rs); alln.update(caps)
    return alln, nc

# ============================================================
#  2. PAD NET RE-ANNOTATION
# ============================================================
def ref_of(fp):
    for gi in fp.graphicItems:
        if type(gi).__name__=='FpText' and getattr(gi,'type',None)=='reference':
            return gi.text
    return ''

def update_pad_nets(b, all_net):
    """Assign each pad's net from (ref, pad.number) -> net name."""
    # build net name -> number (extend as needed)
    netnum={}
    nxt=200  # start new nets above existing
    for f in b.footprints:
        for pad in f.pads:
            if pad.net and pad.net.name not in netnum:
                netnum[pad.net.name]=pad.net.number
    def num_for(name):
        if name in netnum: return netnum[name]
        nonlocal nxt
        netnum[name]=nxt; nxt+=1
        return netnum[name]
    changed=0
    for f in b.footprints:
        r=ref_of(f)
        for pad in f.pads:
            key=(r, str(pad.number))
            if key in all_net:
                nn=all_net[key]
                pad.net=Net(number=num_for(nn), name=nn)
                changed+=1
            else:
                # NC pins -> clear net
                pad.net=None
        # also clear reference text position offset? leave as-is
    return netnum, changed

# ============================================================
#  3. COMPACT PLACEMENT (functional groups)
# ============================================================
# Board ~66 x 60 mm. Grid mapping of existing columns/rows to mm.
# existing cols: 213.76..407.54 step 21.53 -> idx 0..9
# existing rows: 26.65..182.35 step 17.3 -> idx 0..9
COLS=[213.76,235.29,256.82,278.36,299.88,321.42,342.94,364.48,386.00,407.54]
ROWS=[26.65,43.95,61.25,78.55,95.85,113.15,130.45,147.75,165.05,182.35]

# new compact positions (mm) — wider cells so 10 mm ICs don't grossly overlap
NEWCOL=[4,11.5,19,26.5,34,42,50,57.5,65,70]
NEWROW=[4,10.5,17,23.5,30,36.5,43,49.5,56,62.5]

def compact_pos(x, y):
    ci=min(range(10), key=lambda i:abs(COLS[i]-x))
    ri=min(range(10), key=lambda i:abs(ROWS[i]-y))
    return NEWCOL[ci], NEWROW[ri]

def compact_placement(b):
    for f in b.footprints:
        nx,ny=compact_pos(f.position.X, f.position.Y)
        f.position.X=nx; f.position.Y=ny

# ============================================================
#  4. BOARD OUTLINE
# ============================================================
def redraw_outline(b, w=68, h=60):
    # remove old edge cuts
    kept=[g for g in b.graphicItems if getattr(g,'layer',None)!='Edge.Cuts']
    b.graphicItems=kept
    m=1.0
    edges=[(m,m),(w-m,m),(w-m,h-m),(m,h-m),(m,m)]
    for i in range(4):
        b.graphicItems.append(GrLine(
            start=Position(X=edges[i][0],Y=edges[i][1],angle=None),
            end=Position(X=edges[i+1][0],Y=edges[i+1][1],angle=None),
            layer='Edge.Cuts', width=0.1, tstamp=U()))
    return w,h

# ============================================================
#  5. ZONES (GND on In1, PWR on In2)
# ============================================================
def add_gnd_zone(b, w, h, netnum):
    m=1.0
    coords=[Position(X=m,Y=m,angle=None),Position(X=w-m,Y=m,angle=None),
            Position(X=w-m,Y=h-m,angle=None),Position(X=m,Y=h-m,angle=None)]
    z=Zone(locked=False, net=netnum.get(GND,0), netName=GND,
           layers=['In1.Cu'], name='', clearance=0.3, minThickness=0.254,
           hatch=Hatch(), priority=0, connectPads='thermal_reliefs',
           fillSettings=FillSettings(yes=False, mode='hatch',
                thermalGap=0.5, thermalBridgeWidth=0.5))
    zp=ZonePolygon(coordinates=coords)
    z.polygons=[zp]
    b.zones.append(z)

def add_pwr_zones(b, w, h, netnum):
    m=1.0
    # +3.3V occupies left 55%, +5V mid 30%, VBAT right 15%
    thirds_x=[(m,w*0.55),(w*0.56,w*0.84),(w*0.85,w-m)]
    rails=[("+3.3V",P3V3),("+5V",P5V),("VBAT",VBAT)]
    for (x0,x1),(label,netn) in zip(thirds_x,rails):
        coords=[Position(X=x0,Y=m,angle=None),Position(X=x1,Y=m,angle=None),
                Position(X=x1,Y=h-m,angle=None),Position(X=x0,Y=h-m,angle=None)]
        z=Zone(locked=False, net=netnum.get(netn,0), netName=netn,
               layers=['In2.Cu'], name='', clearance=0.3, minThickness=0.254,
               hatch=Hatch(), priority=0, connectPads='thermal_reliefs',
               fillSettings=FillSettings(yes=False, mode='hatch',
                    thermalGap=0.5, thermalBridgeWidth=0.5))
        z.polygons=[ZonePolygon(coordinates=coords)]
        b.zones.append(z)

# ============================================================
#  6. ROUTING: power vias + critical signals
# ============================================================
def add_4layers(b):
    """Insert In1.Cu (GND) and In2.Cu (PWR) after F.Cu."""
    have={l.name for l in b.layers}
    new=[]
    for l in b.layers:
        new.append(l)
        if l.name=='F.Cu' and 'In1.Cu' not in have:
            new.append(LayerToken(ordinal=1, name='In1.Cu', type='power', userName='GND'))
            new.append(LayerToken(ordinal=2, name='In2.Cu', type='power', userName='PWR'))
    b.layers=new

def pad_abspos(fp, pad):
    """Absolute pad position (mm)."""
    a=math.radians(fp.position.angle if hasattr(fp,'position') and hasattr(fp.position,'angle') and fp.position.angle is not None else 0)
    # pad position is relative to footprint
    px,py=pad.position.X, pad.position.Y
    c=math.cos(a); s=math.sin(a)
    rx=px*c-py*s; ry=px*s+py*c
    return fp.position.X+rx, fp.position.Y+ry

def route_power_vias(b, all_net, netnum):
    """For every IC power/ground pad, drop a via to the inner plane + short stub."""
    power_nets={P3V3,P3V3A,P5V,VBAT,GND,DVDD,VBUS}
    via_layer_map={GND:'In1.Cu', P3V3:'In2.Cu', P5V:'In2.Cu', VBAT:'In2.Cu',
                   DVDD:'In2.Cu', P3V3A:'In2.Cu', VBUS:'In2.Cu'}
    vias=0; segs=0
    seen=set()  # avoid stacking vias on same spot
    for f in b.footprints:
        r=ref_of(f)
        if not r.startswith('U'): continue
        for pad in f.pads:
            key=(r,str(pad.number))
            if key not in all_net: continue
            net=all_net[key]
            if net not in power_nets: continue
            ax,ay=pad_abspos(f,pad)
            k=(round(ax,1),round(ay,1))
            if k in seen: continue
            seen.add(k)
            inner=via_layer_map.get(net,'In1.Cu')
            layers=['F.Cu',inner,'B.Cu']
            b.traceItems.append(Via(type='through', position=Position(X=round(ax,3),Y=round(ay,3),angle=None),
                                    size=0.6, drill=0.3, layers=layers, net=netnum.get(net,0), tstamp=U()))
            # short stub from pad to via (they coincide; add tiny F.Cu segment for DRC)
            b.traceItems.append(Segment(start=Position(X=round(ax,3),Y=round(ay,3),angle=None),
                                         end=Position(X=round(ax,3),Y=round(ay,3),angle=None),
                                         width=0.25, layer='F.Cu', net=netnum.get(net,0), tstamp=U()))
            vias+=1
    return vias, segs

def route_signals(b, all_net, netnum):
    """Route ALL signal nets (non-power, multi-pad) using L-shaped Manhattan routing.
    Alternates F.Cu/B.Cu per net to reduce crossover congestion."""
    power_nets={P3V3,P3V3A,P5V,VBAT,GND,DVDD,VBUS}
    # Collect pad positions per signal net
    netpads={}
    for f in b.footprints:
        r=ref_of(f)
        for pad in f.pads:
            key=(r,str(pad.number))
            if key in all_net:
                net=all_net[key]
                if net not in power_nets:
                    netpads.setdefault(net,[]).append((r,pad_abspos(f,pad)))
    routed=0; vias_added=0
    layer_idx=0
    for net in sorted(netpads):
        pads=netpads[net]
        if len(pads)<2: continue
        # Nearest-neighbor chain sort (greedy TSP)
        chain=[pads[0]]; remaining=list(pads[1:])
        while remaining:
            last=chain[-1]
            nearest=min(remaining, key=lambda p:(p[1][0]-last[1][0])**2+(p[1][1]-last[1][1])**2)
            chain.append(nearest); remaining.remove(nearest)
        # Alternate layers: even nets on F.Cu, odd on B.Cu
        layer='B.Cu' if (layer_idx%3==2) else 'F.Cu'
        layer_idx+=1
        nn=netnum.get(net,0)
        need_via = (layer!='F.Cu')
        for i in range(len(chain)-1):
            x1,y1=chain[i][1]; x2,y2=chain[i+1][1]
            # L-route: decide bend point to minimise overlap (horizontal-then-vertical)
            # Use mid-Y = y1 for first segment, then vertical to y2
            mid_y=y1
            # If horizontal distance < 0.3mm, route straight vertical
            if abs(x2-x1)<0.3:
                b.traceItems.append(Segment(
                    start=Position(X=round(x1,3),Y=round(y1,3),angle=None),
                    end=Position(X=round(x2,3),Y=round(y2,3),angle=None),
                    width=0.2, layer=layer, net=nn, tstamp=U()))
                routed+=1
            elif abs(y2-y1)<0.3:
                b.traceItems.append(Segment(
                    start=Position(X=round(x1,3),Y=round(y1,3),angle=None),
                    end=Position(X=round(x2,3),Y=round(y2,3),angle=None),
                    width=0.2, layer=layer, net=nn, tstamp=U()))
                routed+=1
            else:
                # L-shape: horizontal to x2 at y1, then vertical to y2
                b.traceItems.append(Segment(
                    start=Position(X=round(x1,3),Y=round(y1,3),angle=None),
                    end=Position(X=round(x2,3),Y=round(mid_y,3),angle=None),
                    width=0.2, layer=layer, net=nn, tstamp=U()))
                b.traceItems.append(Segment(
                    start=Position(X=round(x2,3),Y=round(mid_y,3),angle=None),
                    end=Position(X=round(x2,3),Y=round(y2,3),angle=None),
                    width=0.2, layer=layer, net=nn, tstamp=U()))
                routed+=2
            # Vias at pad endpoints for B.Cu nets
            if need_via:
                for px,py in [(x1,y1),(x2,y2)]:
                    b.traceItems.append(Via(type='through',
                        position=Position(X=round(px,3),Y=round(py,3),angle=None),
                        size=0.6, drill=0.3, layers=['F.Cu','In1.Cu','In2.Cu','B.Cu'],
                        net=nn, tstamp=U()))
                    vias_added+=1
                need_via=False  # only add vias at first transition
    return routed, vias_added

def route_power_rails(b, all_net, netnum):
    """Route short F.Cu stubs for power nets that connect adjacent ICs."""
    power_nets={P3V3,P3V3A,P5V,VBAT,DVDD}
    netpads={}
    for f in b.footprints:
        r=ref_of(f)
        for pad in f.pads:
            key=(r,str(pad.number))
            if key in all_net:
                net=all_net[key]
                if net in power_nets:
                    netpads.setdefault(net,[]).append((r,pad_abspos(f,pad)))
    routed=0
    for net,pads in netpads.items():
        if len(pads)<2: continue
        nn=netnum.get(net,0)
        chain=[pads[0]]; remaining=list(pads[1:])
        while remaining:
            last=chain[-1]
            nearest=min(remaining, key=lambda p:(p[1][0]-last[1][0])**2+(p[1][1]-last[1][1])**2)
            chain.append(nearest); remaining.remove(nearest)
        for i in range(len(chain)-1):
            x1,y1=chain[i][1]; x2,y2=chain[i+1][1]
            d=((x2-x1)**2+(y2-y1)**2)**0.5
            if d<25:  # only route short hops between nearby pads
                b.traceItems.append(Segment(
                    start=Position(X=round(x1,3),Y=round(y1,3),angle=None),
                    end=Position(X=round(x2,3),Y=round(y2,3),angle=None),
                    width=0.4, layer='F.Cu', net=nn, tstamp=U()))
                routed+=1
    return routed

def add_design_rules(b, netnum):
    """Inject net class + design rules via setup attributes and text post-fix."""
    b.setup.packToMaskClearance=0.2
    b.setup.solderMaskMinWidth=0.0
    b.setup.gridOrigin=Position(X=0,Y=0,angle=None)
    b.setup.auxAxisOrigin=Position(X=0,Y=0,angle=None)

def inject_net_classes(filepath, netnum):
    """Inject (net_class ...) + design rules block before (setup ...)."""
    txt=open(filepath,'r').read()
    if '(net_class' in txt:
        return  # already has net classes
    nc_block = '''  (net_class "Default" "Default routing rules"
    (clearance 0.2) (track_width 0.2) (via_dia 0.6) (via_drill 0.3)
    (uvia_dia 0.3) (uvia_drill 0.15) (diff_pair_gap 0.25)
    (diff_pair_width 0.2))
  (net_class "Power" "Power rails"
    (clearance 0.3) (track_width 0.4) (via_dia 0.8) (via_drill 0.4)
    (uvia_dia 0.4) (uvia_drill 0.2) (diff_pair_gap 0.3)
    (diff_pair_width 0.4))
'''
    idx=txt.find('  (setup')
    if idx<0: return
    txt=txt[:idx]+nc_block+txt[idx:]
    open(filepath,'w').write(txt)

# ============================================================
#  MAIN
# ============================================================
def main():
    b=Board().from_file(PCB)
    print(f"Loaded PCB: {len(b.footprints)} fps, {len(b.traceItems)} traces, {len(b.zones)} zones, {len(b.layers)} layers")
    all_net,nc=build_all_nets()
    print(f"Net table: {len(all_net)} assignments")
    netnum,changed=update_pad_nets(b, all_net)
    print(f"Re-annotated {changed} pads; {len(netnum)} nets")
    compact_placement(b)
    w,h=redraw_outline(b, w=78, h=66)
    add_4layers(b)
    add_gnd_zone(b, w, h, netnum)
    add_pwr_zones(b, w, h, netnum)
    add_design_rules(b, netnum)
    vp,_=route_power_vias(b, all_net, netnum)
    pr=route_power_rails(b, all_net, netnum)
    rs,via_sig=route_signals(b, all_net, netnum)
    print(f"Power vias: {vp}; power stubs: {pr}; signal segs: {rs}; signal vias: {via_sig}")
    b.to_file(PCB)
    inject_net_classes(PCB, netnum)
    cu=[l.name for l in b.layers if l.type in('signal','power','mixed')]
    segs=sum(1 for t in b.traceItems if type(t).__name__=='Segment')
    vias=sum(1 for t in b.traceItems if type(t).__name__=='Via')
    print(f"Final layers: {cu}")
    print(f"Final: {len(b.footprints)} fps, {segs} segs + {vias} vias = {segs+vias} traces, {len(b.zones)} zones, {w}x{h}mm")

if __name__=='__main__':
    main()
