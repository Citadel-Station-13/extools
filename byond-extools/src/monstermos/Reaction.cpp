#include "Reaction.h"

#include "reaction_defines.h"

#include <algorithm>

using namespace monstermos::constants;

extern std::unordered_map<std::string, int> gas_id_strings;

extern int o2,plasma,co2,tritium,water_vapor,n2o,bz,no2;

bool ByondReaction::check_conditions(GasMixture& air)
{
    auto temp = air.get_temperature();
    auto ener = air.thermal_energy();
    if(temp < min_temp_req || ener < min_ener_req)
    {
        return false;
    }
    else
    {
        for(int i = 0; i < TOTAL_NUM_GASES; i++)
        {
            if(air.get_moles(i) < min_gas_reqs[i])
            {
                return false;
            }
        }
    }
    return true;
}

int ByondReaction::react(GasMixture& air,ManagedValue src,ManagedValue holder)
{
    return (int)(float)(Core::get_proc(proc_id).call({src,holder}));
}

bool PlasmaFire::check_conditions(GasMixture& air)
{
    return air.get_temperature() > PLASMA_MINIMUM_BURN_TEMPERATURE &&
    air.get_moles(plasma) > GAS_MIN_MOLES &&
    air.get_moles(o2) > GAS_MIN_MOLES;
}

int PlasmaFire::react(GasMixture& air,ManagedValue src,ManagedValue holder)
{
    bool superSaturation = false;
    float temperature_scale;
    float plasma_burn_rate;
    float oxygen_burn_rate;
    float energy_released = 0.0;
    auto initial_oxygen = air.get_moles(o2);
    auto initial_plasma = air.get_moles(plasma);
    auto old_energy = air.thermal_energy();
    auto temperature = air.get_temperature();
    Container results = src.get("reaction_results");
    results["fire"] = 0.0;
    if(temperature > PLASMA_UPPER_TEMPERATURE)
    {
        temperature_scale = 1;
    }
    else
    {
        temperature_scale = (temperature - PLASMA_MINIMUM_BURN_TEMPERATURE) / (PLASMA_UPPER_TEMPERATURE - PLASMA_MINIMUM_BURN_TEMPERATURE);
    }
    if(temperature_scale > 0)
    {
        oxygen_burn_rate = (OXYGEN_BURN_RATE_BASE-temperature_scale);
        superSaturation = (initial_oxygen/initial_plasma>SUPER_SATURATION_THRESHOLD);
        if(initial_oxygen>initial_plasma*PLASMA_OXYGEN_FULLBURN)
        {
            plasma_burn_rate = initial_plasma*temperature_scale/PLASMA_BURN_RATE_DELTA;
        }
        else
        {
            plasma_burn_rate = (temperature_scale*(initial_oxygen/PLASMA_OXYGEN_FULLBURN))/PLASMA_BURN_RATE_DELTA;
        }
        plasma_burn_rate = std::min(plasma_burn_rate,std::min(initial_plasma,initial_oxygen/oxygen_burn_rate));
        air.set_moles(plasma,initial_plasma - plasma_burn_rate);
        air.set_moles(o2,initial_oxygen - (plasma_burn_rate*oxygen_burn_rate));
        if(superSaturation)
        {
            air.set_moles(tritium,air.get_moles(tritium) + plasma_burn_rate);
        }
        else
        {
            air.set_moles(co2,air.get_moles(co2) + plasma_burn_rate);
        }
        energy_released = FIRE_PLASMA_ENERGY_RELEASED * plasma_burn_rate;
        results["fire"] = plasma_burn_rate*(1+oxygen_burn_rate);
    }
    if(energy_released > 0)
    {
        air.set_temperature((old_energy+energy_released)/air.heat_capacity());
    }
    if(holder.type == TURF)
    {
        temperature = air.get_temperature();
        if(temperature > FIRE_MINIMUM_TEMPERATURE_TO_EXIST)
        {
            holder.invoke("hotspot_expose",{temperature, CELL_VOLUME});
            IncRefCount(src.type,src.value);
            holder.invoke("temperature_expose",{src,temperature,CELL_VOLUME});
        }
    }
    return results.at("fire") > 0.0 ? REACTING : NO_REACTION;
}

bool TritFire::check_conditions(GasMixture& air)
{
    return air.get_temperature() > FIRE_MINIMUM_TEMPERATURE_TO_EXIST &&
    air.get_moles(tritium) > GAS_MIN_MOLES &&
    air.get_moles(o2) > GAS_MIN_MOLES;
}

#include <random>

#include "../core/proc_management.h"

int TritFire::react(GasMixture& air,ManagedValue src,ManagedValue holder)
{
    float energy_released = 0.0;
    float old_heat_capacity = air.heat_capacity();
    float temperature = air.get_temperature();
    Container results = src.get("reaction_results");
    results["fire"] = 0.0;
    float burned_fuel = 0.0;
    if(air.get_moles(o2) < air.get_moles(tritium))
    {
		burned_fuel = air.get_moles(o2)/TRITIUM_BURN_OXY_FACTOR;
		air.set_moles(tritium, air.get_moles(tritium)-burned_fuel);
    }
    else
    {
		burned_fuel = air.get_moles(tritium)*TRITIUM_BURN_TRIT_FACTOR;
		air.set_moles(tritium, air.get_moles(tritium)-air.get_moles(tritium)/TRITIUM_BURN_TRIT_FACTOR);
		air.set_moles(o2,air.get_moles(o2)-air.get_moles(tritium));
    }
    if(burned_fuel > 0.0)
    {
        energy_released += FIRE_HYDROGEN_ENERGY_RELEASED * burned_fuel;
        if(holder.type == TURF && burned_fuel > TRITIUM_MINIMUM_RADIATION_ENERGY)
        {
            std::random_device rd;
            std::mt19937 gen(rd());
            if(std::generate_canonical<float,10>(gen) < 0.1)
            {
                IncRefCount(holder.type,holder.value);
                Core::get_proc("/proc/radiation_pulse").call({holder, energy_released/TRITIUM_BURN_RADIOACTIVITY_FACTOR});
            }
            air.set_moles(water_vapor, air.get_moles(water_vapor) + burned_fuel/TRITIUM_BURN_OXY_FACTOR);
            results["fire"] = burned_fuel;
        }
    }
    if(energy_released > 0.0)
    {
        auto new_heat_capacity = air.heat_capacity();
        if(new_heat_capacity > MINIMUM_HEAT_CAPACITY)
        {
            air.set_temperature((temperature*old_heat_capacity + energy_released)/new_heat_capacity);
        }
    }
    if(holder.type == TURF)
    {
        temperature = air.get_temperature();
        if(temperature > FIRE_MINIMUM_TEMPERATURE_TO_EXIST)
        {
            holder.invoke("hotspot_expose",{temperature, CELL_VOLUME});
            IncRefCount(src.type,src.value);
            holder.invoke("temperature_expose",{src,temperature,CELL_VOLUME});
        }
    }
    return results.at("fire") > 0.0 ? REACTING : NO_REACTION;
}

#define _USE_MATH_DEFINES

#include <math.h>

#include <cmath>

bool Fusion::check_conditions(GasMixture& air)
{
    return air.get_temperature() > FUSION_TEMPERATURE_THRESHOLD &&
    air.get_moles(tritium) > FUSION_TRITIUM_MOLES_USED &&
    air.get_moles(plasma) > FUSION_MOLE_THRESHOLD &&
    air.get_moles(co2) > FUSION_MOLE_THRESHOLD;
}

extern float gas_fusion_power[TOTAL_NUM_GASES];

int Fusion::react(GasMixture& air,ManagedValue src,ManagedValue holder)
{
    Container analyzer_results = src.get("analyzer_results");
    auto old_heat_capacity = air.heat_capacity();
    auto initial_plasma = air.get_moles(plasma);
    auto initial_carbon = air.get_moles(co2);
    auto scale_factor = air.get_volume() / M_PI;
    auto toroidal_size = 2*M_PI+atan((air.get_volume()-TOROID_VOLUME_BREAKEVEN)/TOROID_VOLUME_BREAKEVEN);
    auto gas_power = 0.0;
    for(int i = 0;i < TOTAL_NUM_GASES;i++)
    {
        gas_power += gas_fusion_power[i] * air.get_moles(i);
    }
    auto instability = fmod(pow(gas_power*INSTABILITY_GAS_POWER_FACTOR,2), toroidal_size);
    auto plasma_2 = (initial_plasma - FUSION_MOLE_THRESHOLD)/scale_factor;
    auto carbon_2 = (initial_carbon - FUSION_MOLE_THRESHOLD)/scale_factor;
    plasma_2 = fmod(plasma_2 - (instability * sin(carbon_2)),toroidal_size);
    carbon_2 = fmod(carbon_2 - plasma_2, toroidal_size);
    air.set_moles(plasma,plasma_2*scale_factor + FUSION_MOLE_THRESHOLD);
    air.set_moles(co2,carbon_2*scale_factor + FUSION_MOLE_THRESHOLD);
    auto delta_plasma = initial_plasma - air.get_moles(plasma);

    float reaction_energy = delta_plasma * PLASMA_BINDING_ENERGY;
    if(instability < FUSION_INSTABILITY_ENDOTHERMALITY)
    {
        reaction_energy = std::max((double)reaction_energy,0.0);
    }
    else if(reaction_energy < 0.0)
    {
        reaction_energy *= std::sqrtf(instability-FUSION_INSTABILITY_ENDOTHERMALITY);
    }
    if(air.thermal_energy() + reaction_energy < 0)
    {
        air.set_moles(plasma,initial_plasma);
        air.set_moles(co2,initial_carbon);
        return NO_REACTION;
    }
    air.set_moles(tritium,-FUSION_TRITIUM_MOLES_USED);
    if(reaction_energy > 0)
    {
        air.set_moles(o2,air.get_moles(o2)+FUSION_TRITIUM_MOLES_USED*(reaction_energy*FUSION_TRITIUM_CONVERSION_COEFFICIENT));
        air.set_moles(n2o,air.get_moles(o2)+FUSION_TRITIUM_MOLES_USED*(reaction_energy*FUSION_TRITIUM_CONVERSION_COEFFICIENT));
    }
    else
    {
        air.set_moles(bz,air.get_moles(o2)+FUSION_TRITIUM_MOLES_USED*(reaction_energy*FUSION_TRITIUM_CONVERSION_COEFFICIENT));
        air.set_moles(no2,air.get_moles(o2)+FUSION_TRITIUM_MOLES_USED*(reaction_energy*FUSION_TRITIUM_CONVERSION_COEFFICIENT));
    }
    if(reaction_energy != 0.0)
    {
        if(holder.type != DataType::NULL_D)
        {
            auto particle_chance = (PARTICLE_CHANCE_CONSTANT/(reaction_energy/PARTICLE_CHANCE_CONSTANT)) + 1;
            auto rad_power = std::max((FUSION_RAD_COEFFICIENT/instability) + FUSION_RAD_MAX,0.0);
            IncRefCount(holder.type,holder.value);
            Core::get_proc("/proc/fusion_effects").call({holder,(float)particle_chance,(float)rad_power});
        }
        auto new_heat_capacity = air.heat_capacity();
        if(new_heat_capacity > MINIMUM_HEAT_CAPACITY)
        {
            air.set_temperature(std::clamp(air.get_temperature()*old_heat_capacity+reaction_energy,TCMB,INFINITY));
        }
        return REACTING;
    }
    return NO_REACTION;
}