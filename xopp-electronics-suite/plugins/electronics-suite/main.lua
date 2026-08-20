-- Xournal++ Electronics Suite Plugin
function initUi()
  app.registerUi({menu="Electronics", callback="dummy", parentPath=""})
  app.registerUi({menu="Resistor (US)", callback="insert_asset_1", parentPath="Electronics/Analog & Passives"})
  app.registerUi({menu="Resistor (EU)", callback="insert_asset_2", parentPath="Electronics/Analog & Passives"})
  app.registerUi({menu="Potentiometer", callback="insert_asset_3", parentPath="Electronics/Analog & Passives"})
  app.registerUi({menu="Capacitor (Non-Polarized)", callback="insert_asset_4", parentPath="Electronics/Analog & Passives"})
  app.registerUi({menu="Capacitor (Polarized)", callback="insert_asset_5", parentPath="Electronics/Analog & Passives"})
  app.registerUi({menu="Inductor", callback="insert_asset_6", parentPath="Electronics/Analog & Passives"})
  app.registerUi({menu="DC Source (Battery)", callback="insert_asset_7", parentPath="Electronics/Analog & Passives"})
  app.registerUi({menu="DC Source (Circle)", callback="insert_asset_8", parentPath="Electronics/Analog & Passives"})
  app.registerUi({menu="AC Source", callback="insert_asset_9", parentPath="Electronics/Analog & Passives"})
  app.registerUi({menu="Current Source", callback="insert_asset_10", parentPath="Electronics/Analog & Passives"})
  app.registerUi({menu="Earth Ground", callback="insert_asset_11", parentPath="Electronics/Analog & Passives"})
  app.registerUi({menu="Chassis Ground", callback="insert_asset_12", parentPath="Electronics/Analog & Passives"})
  app.registerUi({menu="Signal Ground", callback="insert_asset_13", parentPath="Electronics/Analog & Passives"})
  app.registerUi({menu="SPST Switch", callback="insert_asset_14", parentPath="Electronics/Analog & Passives"})
  app.registerUi({menu="SPDT Switch", callback="insert_asset_15", parentPath="Electronics/Analog & Passives"})
  app.registerUi({menu="Diode", callback="insert_asset_16", parentPath="Electronics/Semiconductors & Op-Amps"})
  app.registerUi({menu="Zener Diode", callback="insert_asset_17", parentPath="Electronics/Semiconductors & Op-Amps"})
  app.registerUi({menu="Schottky Diode", callback="insert_asset_18", parentPath="Electronics/Semiconductors & Op-Amps"})
  app.registerUi({menu="LED", callback="insert_asset_19", parentPath="Electronics/Semiconductors & Op-Amps"})
  app.registerUi({menu="BJT (NPN)", callback="insert_asset_20", parentPath="Electronics/Semiconductors & Op-Amps"})
  app.registerUi({menu="BJT (PNP)", callback="insert_asset_21", parentPath="Electronics/Semiconductors & Op-Amps"})
  app.registerUi({menu="MOSFET (N-Channel)", callback="insert_asset_22", parentPath="Electronics/Semiconductors & Op-Amps"})
  app.registerUi({menu="MOSFET (P-Channel)", callback="insert_asset_23", parentPath="Electronics/Semiconductors & Op-Amps"})
  app.registerUi({menu="Op-Amp", callback="insert_asset_24", parentPath="Electronics/Semiconductors & Op-Amps"})
  app.registerUi({menu="AND Gate", callback="insert_asset_25", parentPath="Electronics/Digital Logic & Gates"})
  app.registerUi({menu="OR Gate", callback="insert_asset_26", parentPath="Electronics/Digital Logic & Gates"})
  app.registerUi({menu="NOT Gate", callback="insert_asset_27", parentPath="Electronics/Digital Logic & Gates"})
  app.registerUi({menu="NAND Gate", callback="insert_asset_28", parentPath="Electronics/Digital Logic & Gates"})
  app.registerUi({menu="NOR Gate", callback="insert_asset_29", parentPath="Electronics/Digital Logic & Gates"})
  app.registerUi({menu="XOR Gate", callback="insert_asset_30", parentPath="Electronics/Digital Logic & Gates"})
  app.registerUi({menu="XNOR Gate", callback="insert_asset_31", parentPath="Electronics/Digital Logic & Gates"})
  app.registerUi({menu="D Flip-Flop", callback="insert_asset_32", parentPath="Electronics/Digital Logic & Gates"})
  app.registerUi({menu="JK Flip-Flop", callback="insert_asset_33", parentPath="Electronics/Digital Logic & Gates"})
  app.registerUi({menu="Mixer", callback="insert_asset_34", parentPath="Electronics/Digital Logic & Gates"})
  app.registerUi({menu="Amplifier", callback="insert_asset_35", parentPath="Electronics/Digital Logic & Gates"})
  app.registerUi({menu="Low-Pass Filter", callback="insert_asset_36", parentPath="Electronics/Digital Logic & Gates"})
  app.registerUi({menu="High-Pass Filter", callback="insert_asset_37", parentPath="Electronics/Digital Logic & Gates"})
  app.registerUi({menu="Oscillator", callback="insert_asset_38", parentPath="Electronics/Digital Logic & Gates"})
  app.registerUi({menu="Antenna", callback="insert_asset_39", parentPath="Electronics/Digital Logic & Gates"})
  app.registerUi({menu="Sine Wave", callback="insert_asset_40", parentPath="Electronics/Waveforms & Graphs"})
  app.registerUi({menu="Sine Wave (2 Periods)", callback="insert_asset_41", parentPath="Electronics/Waveforms & Graphs"})
  app.registerUi({menu="Square Wave", callback="insert_asset_42", parentPath="Electronics/Waveforms & Graphs"})
  app.registerUi({menu="Triangle Wave", callback="insert_asset_43", parentPath="Electronics/Waveforms & Graphs"})
  app.registerUi({menu="Sawtooth Wave", callback="insert_asset_44", parentPath="Electronics/Waveforms & Graphs"})
  app.registerUi({menu="t vs V Axis", callback="insert_asset_45", parentPath="Electronics/Waveforms & Graphs"})
  app.registerUi({menu="f vs A Axis", callback="insert_asset_46", parentPath="Electronics/Waveforms & Graphs"})
  app.registerUi({menu="Phasor Plane", callback="insert_asset_47", parentPath="Electronics/Waveforms & Graphs"})
  app.registerUi({menu="Bode Plot (2 Decades)", callback="insert_asset_48", parentPath="Electronics/Analysis Grids (K-Maps, Timing, Bode)"})
  app.registerUi({menu="Bode Plot (3 Decades)", callback="insert_asset_49", parentPath="Electronics/Analysis Grids (K-Maps, Timing, Bode)"})
  app.registerUi({menu="K-Map 2x2", callback="insert_asset_50", parentPath="Electronics/Analysis Grids (K-Maps, Timing, Bode)"})
  app.registerUi({menu="K-Map 2x4", callback="insert_asset_51", parentPath="Electronics/Analysis Grids (K-Maps, Timing, Bode)"})
  app.registerUi({menu="K-Map 4x4", callback="insert_asset_52", parentPath="Electronics/Analysis Grids (K-Maps, Timing, Bode)"})
  app.registerUi({menu="Timing Diagram", callback="insert_asset_53", parentPath="Electronics/Analysis Grids (K-Maps, Timing, Bode)"})
  app.registerUi({menu="Truth Table", callback="insert_asset_54", parentPath="Electronics/Analysis Grids (K-Maps, Timing, Bode)"})
  app.registerUi({menu="Register (8-bit)", callback="insert_asset_55", parentPath="Electronics/Analysis Grids (K-Maps, Timing, Bode)"})
  app.registerUi({menu="Register (16-bit)", callback="insert_asset_56", parentPath="Electronics/Analysis Grids (K-Maps, Timing, Bode)"})
  app.registerUi({menu="Flowchart Start/End", callback="insert_asset_57", parentPath="Electronics/Analysis Grids (K-Maps, Timing, Bode)"})
  app.registerUi({menu="Flowchart Process", callback="insert_asset_58", parentPath="Electronics/Analysis Grids (K-Maps, Timing, Bode)"})
  app.registerUi({menu="Flowchart Decision", callback="insert_asset_59", parentPath="Electronics/Analysis Grids (K-Maps, Timing, Bode)"})
  app.registerUi({menu="Flowchart I/O", callback="insert_asset_60", parentPath="Electronics/Analysis Grids (K-Maps, Timing, Bode)"})
end
function dummy()
end
function insert_asset_1()
  app.addImages({images = {{path = app.getPluginDir() .. "/assets/passives/resistor_us.svg", x = 100, y = 100}}})
  app.uiAction({action="ACTION_UPDATE_PAGE_BACKGROUND"})
end
function insert_asset_2()
  app.addImages({images = {{path = app.getPluginDir() .. "/assets/passives/resistor_eu.svg", x = 100, y = 100}}})
  app.uiAction({action="ACTION_UPDATE_PAGE_BACKGROUND"})
end
function insert_asset_3()
  app.addImages({images = {{path = app.getPluginDir() .. "/assets/passives/potentiometer.svg", x = 100, y = 100}}})
  app.uiAction({action="ACTION_UPDATE_PAGE_BACKGROUND"})
end
function insert_asset_4()
  app.addImages({images = {{path = app.getPluginDir() .. "/assets/passives/capacitor_np.svg", x = 100, y = 100}}})
  app.uiAction({action="ACTION_UPDATE_PAGE_BACKGROUND"})
end
function insert_asset_5()
  app.addImages({images = {{path = app.getPluginDir() .. "/assets/passives/capacitor_pol.svg", x = 100, y = 100}}})
  app.uiAction({action="ACTION_UPDATE_PAGE_BACKGROUND"})
end
function insert_asset_6()
  app.addImages({images = {{path = app.getPluginDir() .. "/assets/passives/inductor.svg", x = 100, y = 100}}})
  app.uiAction({action="ACTION_UPDATE_PAGE_BACKGROUND"})
end
function insert_asset_7()
  app.addImages({images = {{path = app.getPluginDir() .. "/assets/passives/source_dc_batt.svg", x = 100, y = 100}}})
  app.uiAction({action="ACTION_UPDATE_PAGE_BACKGROUND"})
end
function insert_asset_8()
  app.addImages({images = {{path = app.getPluginDir() .. "/assets/passives/source_dc.svg", x = 100, y = 100}}})
  app.uiAction({action="ACTION_UPDATE_PAGE_BACKGROUND"})
end
function insert_asset_9()
  app.addImages({images = {{path = app.getPluginDir() .. "/assets/passives/source_ac.svg", x = 100, y = 100}}})
  app.uiAction({action="ACTION_UPDATE_PAGE_BACKGROUND"})
end
function insert_asset_10()
  app.addImages({images = {{path = app.getPluginDir() .. "/assets/passives/source_current.svg", x = 100, y = 100}}})
  app.uiAction({action="ACTION_UPDATE_PAGE_BACKGROUND"})
end
function insert_asset_11()
  app.addImages({images = {{path = app.getPluginDir() .. "/assets/passives/gnd_earth.svg", x = 100, y = 100}}})
  app.uiAction({action="ACTION_UPDATE_PAGE_BACKGROUND"})
end
function insert_asset_12()
  app.addImages({images = {{path = app.getPluginDir() .. "/assets/passives/gnd_chassis.svg", x = 100, y = 100}}})
  app.uiAction({action="ACTION_UPDATE_PAGE_BACKGROUND"})
end
function insert_asset_13()
  app.addImages({images = {{path = app.getPluginDir() .. "/assets/passives/gnd_signal.svg", x = 100, y = 100}}})
  app.uiAction({action="ACTION_UPDATE_PAGE_BACKGROUND"})
end
function insert_asset_14()
  app.addImages({images = {{path = app.getPluginDir() .. "/assets/passives/switch_spst.svg", x = 100, y = 100}}})
  app.uiAction({action="ACTION_UPDATE_PAGE_BACKGROUND"})
end
function insert_asset_15()
  app.addImages({images = {{path = app.getPluginDir() .. "/assets/passives/switch_spdt.svg", x = 100, y = 100}}})
  app.uiAction({action="ACTION_UPDATE_PAGE_BACKGROUND"})
end
function insert_asset_16()
  app.addImages({images = {{path = app.getPluginDir() .. "/assets/semiconductors/diode.svg", x = 100, y = 100}}})
  app.uiAction({action="ACTION_UPDATE_PAGE_BACKGROUND"})
end
function insert_asset_17()
  app.addImages({images = {{path = app.getPluginDir() .. "/assets/semiconductors/diode_zener.svg", x = 100, y = 100}}})
  app.uiAction({action="ACTION_UPDATE_PAGE_BACKGROUND"})
end
function insert_asset_18()
  app.addImages({images = {{path = app.getPluginDir() .. "/assets/semiconductors/diode_schottky.svg", x = 100, y = 100}}})
  app.uiAction({action="ACTION_UPDATE_PAGE_BACKGROUND"})
end
function insert_asset_19()
  app.addImages({images = {{path = app.getPluginDir() .. "/assets/semiconductors/diode_led.svg", x = 100, y = 100}}})
  app.uiAction({action="ACTION_UPDATE_PAGE_BACKGROUND"})
end
function insert_asset_20()
  app.addImages({images = {{path = app.getPluginDir() .. "/assets/semiconductors/bjt_npn.svg", x = 100, y = 100}}})
  app.uiAction({action="ACTION_UPDATE_PAGE_BACKGROUND"})
end
function insert_asset_21()
  app.addImages({images = {{path = app.getPluginDir() .. "/assets/semiconductors/bjt_pnp.svg", x = 100, y = 100}}})
  app.uiAction({action="ACTION_UPDATE_PAGE_BACKGROUND"})
end
function insert_asset_22()
  app.addImages({images = {{path = app.getPluginDir() .. "/assets/semiconductors/mosfet_n.svg", x = 100, y = 100}}})
  app.uiAction({action="ACTION_UPDATE_PAGE_BACKGROUND"})
end
function insert_asset_23()
  app.addImages({images = {{path = app.getPluginDir() .. "/assets/semiconductors/mosfet_p.svg", x = 100, y = 100}}})
  app.uiAction({action="ACTION_UPDATE_PAGE_BACKGROUND"})
end
function insert_asset_24()
  app.addImages({images = {{path = app.getPluginDir() .. "/assets/semiconductors/opamp.svg", x = 100, y = 100}}})
  app.uiAction({action="ACTION_UPDATE_PAGE_BACKGROUND"})
end
function insert_asset_25()
  app.addImages({images = {{path = app.getPluginDir() .. "/assets/digital/gate_and.svg", x = 100, y = 100}}})
  app.uiAction({action="ACTION_UPDATE_PAGE_BACKGROUND"})
end
function insert_asset_26()
  app.addImages({images = {{path = app.getPluginDir() .. "/assets/digital/gate_or.svg", x = 100, y = 100}}})
  app.uiAction({action="ACTION_UPDATE_PAGE_BACKGROUND"})
end
function insert_asset_27()
  app.addImages({images = {{path = app.getPluginDir() .. "/assets/digital/gate_not.svg", x = 100, y = 100}}})
  app.uiAction({action="ACTION_UPDATE_PAGE_BACKGROUND"})
end
function insert_asset_28()
  app.addImages({images = {{path = app.getPluginDir() .. "/assets/digital/gate_nand.svg", x = 100, y = 100}}})
  app.uiAction({action="ACTION_UPDATE_PAGE_BACKGROUND"})
end
function insert_asset_29()
  app.addImages({images = {{path = app.getPluginDir() .. "/assets/digital/gate_nor.svg", x = 100, y = 100}}})
  app.uiAction({action="ACTION_UPDATE_PAGE_BACKGROUND"})
end
function insert_asset_30()
  app.addImages({images = {{path = app.getPluginDir() .. "/assets/digital/gate_xor.svg", x = 100, y = 100}}})
  app.uiAction({action="ACTION_UPDATE_PAGE_BACKGROUND"})
end
function insert_asset_31()
  app.addImages({images = {{path = app.getPluginDir() .. "/assets/digital/gate_xnor.svg", x = 100, y = 100}}})
  app.uiAction({action="ACTION_UPDATE_PAGE_BACKGROUND"})
end
function insert_asset_32()
  app.addImages({images = {{path = app.getPluginDir() .. "/assets/digital/ff_d.svg", x = 100, y = 100}}})
  app.uiAction({action="ACTION_UPDATE_PAGE_BACKGROUND"})
end
function insert_asset_33()
  app.addImages({images = {{path = app.getPluginDir() .. "/assets/digital/ff_jk.svg", x = 100, y = 100}}})
  app.uiAction({action="ACTION_UPDATE_PAGE_BACKGROUND"})
end
function insert_asset_34()
  app.addImages({images = {{path = app.getPluginDir() .. "/assets/blocks/mixer.svg", x = 100, y = 100}}})
  app.uiAction({action="ACTION_UPDATE_PAGE_BACKGROUND"})
end
function insert_asset_35()
  app.addImages({images = {{path = app.getPluginDir() .. "/assets/blocks/amplifier.svg", x = 100, y = 100}}})
  app.uiAction({action="ACTION_UPDATE_PAGE_BACKGROUND"})
end
function insert_asset_36()
  app.addImages({images = {{path = app.getPluginDir() .. "/assets/blocks/filter_lpf.svg", x = 100, y = 100}}})
  app.uiAction({action="ACTION_UPDATE_PAGE_BACKGROUND"})
end
function insert_asset_37()
  app.addImages({images = {{path = app.getPluginDir() .. "/assets/blocks/filter_hpf.svg", x = 100, y = 100}}})
  app.uiAction({action="ACTION_UPDATE_PAGE_BACKGROUND"})
end
function insert_asset_38()
  app.addImages({images = {{path = app.getPluginDir() .. "/assets/blocks/oscillator.svg", x = 100, y = 100}}})
  app.uiAction({action="ACTION_UPDATE_PAGE_BACKGROUND"})
end
function insert_asset_39()
  app.addImages({images = {{path = app.getPluginDir() .. "/assets/blocks/antenna.svg", x = 100, y = 100}}})
  app.uiAction({action="ACTION_UPDATE_PAGE_BACKGROUND"})
end
function insert_asset_40()
  app.addImages({images = {{path = app.getPluginDir() .. "/assets/waveforms/wave_sine.svg", x = 100, y = 100}}})
  app.uiAction({action="ACTION_UPDATE_PAGE_BACKGROUND"})
end
function insert_asset_41()
  app.addImages({images = {{path = app.getPluginDir() .. "/assets/waveforms/wave_sine_2p.svg", x = 100, y = 100}}})
  app.uiAction({action="ACTION_UPDATE_PAGE_BACKGROUND"})
end
function insert_asset_42()
  app.addImages({images = {{path = app.getPluginDir() .. "/assets/waveforms/wave_square.svg", x = 100, y = 100}}})
  app.uiAction({action="ACTION_UPDATE_PAGE_BACKGROUND"})
end
function insert_asset_43()
  app.addImages({images = {{path = app.getPluginDir() .. "/assets/waveforms/wave_triangle.svg", x = 100, y = 100}}})
  app.uiAction({action="ACTION_UPDATE_PAGE_BACKGROUND"})
end
function insert_asset_44()
  app.addImages({images = {{path = app.getPluginDir() .. "/assets/waveforms/wave_sawtooth.svg", x = 100, y = 100}}})
  app.uiAction({action="ACTION_UPDATE_PAGE_BACKGROUND"})
end
function insert_asset_45()
  app.addImages({images = {{path = app.getPluginDir() .. "/assets/waveforms/coord_t_V.svg", x = 100, y = 100}}})
  app.uiAction({action="ACTION_UPDATE_PAGE_BACKGROUND"})
end
function insert_asset_46()
  app.addImages({images = {{path = app.getPluginDir() .. "/assets/waveforms/coord_f_A.svg", x = 100, y = 100}}})
  app.uiAction({action="ACTION_UPDATE_PAGE_BACKGROUND"})
end
function insert_asset_47()
  app.addImages({images = {{path = app.getPluginDir() .. "/assets/waveforms/phasor_plane.svg", x = 100, y = 100}}})
  app.uiAction({action="ACTION_UPDATE_PAGE_BACKGROUND"})
end
function insert_asset_48()
  app.addImages({images = {{path = app.getPluginDir() .. "/assets/waveforms/bode_grid_2dec.svg", x = 100, y = 100}}})
  app.uiAction({action="ACTION_UPDATE_PAGE_BACKGROUND"})
end
function insert_asset_49()
  app.addImages({images = {{path = app.getPluginDir() .. "/assets/waveforms/bode_grid_3dec.svg", x = 100, y = 100}}})
  app.uiAction({action="ACTION_UPDATE_PAGE_BACKGROUND"})
end
function insert_asset_50()
  app.addImages({images = {{path = app.getPluginDir() .. "/assets/digital/kmap_2x2.svg", x = 100, y = 100}}})
  app.uiAction({action="ACTION_UPDATE_PAGE_BACKGROUND"})
end
function insert_asset_51()
  app.addImages({images = {{path = app.getPluginDir() .. "/assets/digital/kmap_2x4.svg", x = 100, y = 100}}})
  app.uiAction({action="ACTION_UPDATE_PAGE_BACKGROUND"})
end
function insert_asset_52()
  app.addImages({images = {{path = app.getPluginDir() .. "/assets/digital/kmap_4x4.svg", x = 100, y = 100}}})
  app.uiAction({action="ACTION_UPDATE_PAGE_BACKGROUND"})
end
function insert_asset_53()
  app.addImages({images = {{path = app.getPluginDir() .. "/assets/digital/timing_grid.svg", x = 100, y = 100}}})
  app.uiAction({action="ACTION_UPDATE_PAGE_BACKGROUND"})
end
function insert_asset_54()
  app.addImages({images = {{path = app.getPluginDir() .. "/assets/digital/truth_table.svg", x = 100, y = 100}}})
  app.uiAction({action="ACTION_UPDATE_PAGE_BACKGROUND"})
end
function insert_asset_55()
  app.addImages({images = {{path = app.getPluginDir() .. "/assets/digital/register_8bit.svg", x = 100, y = 100}}})
  app.uiAction({action="ACTION_UPDATE_PAGE_BACKGROUND"})
end
function insert_asset_56()
  app.addImages({images = {{path = app.getPluginDir() .. "/assets/digital/register_16bit.svg", x = 100, y = 100}}})
  app.uiAction({action="ACTION_UPDATE_PAGE_BACKGROUND"})
end
function insert_asset_57()
  app.addImages({images = {{path = app.getPluginDir() .. "/assets/digital/flow_start.svg", x = 100, y = 100}}})
  app.uiAction({action="ACTION_UPDATE_PAGE_BACKGROUND"})
end
function insert_asset_58()
  app.addImages({images = {{path = app.getPluginDir() .. "/assets/digital/flow_process.svg", x = 100, y = 100}}})
  app.uiAction({action="ACTION_UPDATE_PAGE_BACKGROUND"})
end
function insert_asset_59()
  app.addImages({images = {{path = app.getPluginDir() .. "/assets/digital/flow_decision.svg", x = 100, y = 100}}})
  app.uiAction({action="ACTION_UPDATE_PAGE_BACKGROUND"})
end
function insert_asset_60()
  app.addImages({images = {{path = app.getPluginDir() .. "/assets/digital/flow_io.svg", x = 100, y = 100}}})
  app.uiAction({action="ACTION_UPDATE_PAGE_BACKGROUND"})
end
