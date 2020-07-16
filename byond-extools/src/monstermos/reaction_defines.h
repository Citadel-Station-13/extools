//Defines used in atmos gas reactions. Used to be located in ..\modules\atmospherics\gasmixtures\reactions.dm, but were moved here because fusion added so fucking many.

const int MINIMUM_MOLE_COUNT						= 0.01;
//Plasma fire properties
const float OXYGEN_BURN_RATE_BASE					= 1.4;
const int PLASMA_BURN_RATE_DELTA					= 9;
const int PLASMA_MINIMUM_OXYGEN_NEEDED				= 2;
const int PLASMA_MINIMUM_OXYGEN_PLASMA_RATIO		= 30;
const int FIRE_CARBON_ENERGY_RELEASED				= 100000;	//Amount of heat released per mole of burnt carbon into the tile
const int FIRE_HYDROGEN_ENERGY_RELEASED				= 280000;  //Amount of heat released per mole of burnt hydrogen and/or tritium(hydrogen isotope)
const int FIRE_PLASMA_ENERGY_RELEASED				= 3000000;	//Amount of heat released per mole of burnt plasma into the tile
//General assmos defines.
const int WATER_VAPOR_FREEZE						= 200;
const int NITRYL_FORMATION_ENERGY					= 100000;
const int TRITIUM_BURN_OXY_FACTOR					= 100;
const int TRITIUM_BURN_TRIT_FACTOR					= 10;
const int TRITIUM_BURN_RADIOACTIVITY_FACTOR			= 50000; 	//The neutrons gotta go somewhere. Completely arbitrary number.
const float TRITIUM_MINIMUM_RADIATION_ENERGY		= 0.1;  	//minimum 0.01 moles trit or 10 moles oxygen to start producing rads
const int SUPER_SATURATION_THRESHOLD				= 96;
const int STIMULUM_HEAT_SCALE						= 100000;
const float STIMULUM_FIRST_RISE						= 0.65;
const float STIMULUM_FIRST_DROP						= 0.065;
const float STIMULUM_SECOND_RISE					= 0.0009;
const float STIMULUM_ABSOLUTE_DROP					= 0.00000335;
const int REACTION_OPPRESSION_THRESHOLD				= 5;
const int NOBLIUM_FORMATION_ENERGY					= 2e9; 	//1 Mole of Noblium takes the planck energy to condense.
//Research point amounts
const int NOBLIUM_RESEARCH_AMOUNT					= 1000;
const int BZ_RESEARCH_SCALE							= 4;
const int BZ_RESEARCH_MAX_AMOUNT					= 400;
const int MIASMA_RESEARCH_AMOUNT					= 6;
const int STIMULUM_RESEARCH_AMOUNT					= 50;
//Plasma fusion properties
const float FUSION_ENERGY_THRESHOLD					= 3e9; 	//Amount of energy it takes to start a fusion reaction
const int FUSION_MOLE_THRESHOLD						= 250; 	//Mole count required (tritium/plasma) to start a fusion reaction
const float FUSION_TRITIUM_CONVERSION_COEFFICIENT	= (1e-10);
const float INSTABILITY_GAS_POWER_FACTOR			= 0.003;
const int FUSION_TRITIUM_MOLES_USED					= 1;
const int PLASMA_BINDING_ENERGY						= 20000000;
const int TOROID_VOLUME_BREAKEVEN					= 1000;
const int FUSION_TEMPERATURE_THRESHOLD				= 10000;
const int PARTICLE_CHANCE_CONSTANT					= (-20000000);
const int FUSION_RAD_MAX							= 2000;
const int FUSION_RAD_COEFFICIENT					= (-1000);
const int FUSION_INSTABILITY_ENDOTHERMALITY			= 2;