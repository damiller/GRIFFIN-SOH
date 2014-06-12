//
// Name: evid.h
// Description: definition for MIDAS event IDs
//

// EVID ranges assigned by Renee
//
// Subsystem      Evid range
// FGD            1 - 19
// TPC            30 - 49
// Magnet         60 - 69
// POD            80 - 89
// Ecal           100 - 109
// SMRD           120 - 129
// Infrastructure 140- 149

// definitions for the event ids

// misc equipments

#define EVID_DCC     1
#define EVID_DVM     2
#define EVID_VME     3
#define EVID_ASUM    4
#define EVID_MSCB    5
#define EVID_WIENER     8
#define EVID_PWRSWITCH  9
#define EVID_FGDCOOLING 10

// FGD equipments range 1-19

#define EVID_FGDWATER   7
#define EVID_FGDDCCPWR 11
#define EVID_FGDSC     12
#define EVID_FGDWIENER 13

// TPC equipments 30-49

#define EVID_TPCDCCPWR 31
#define EVID_TPCWIENER 32
#define EVID_TPCISEG   33
#define EVID_TPCGASPLC 34
#define EVID_TPC1TEMP  35
#define EVID_TPC2TEMP  36
#define EVID_TPC3TEMP  37
#define EVID_TPCXANTREX 38
#define EVID_CCBertan  39

// Infastructure 140-149

#define EVID_ENVTEMP 140
#define EVID_DETCOOLING 141

// end
