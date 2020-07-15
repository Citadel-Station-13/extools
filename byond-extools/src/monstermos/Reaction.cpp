#include "Reaction.h"

extern int str_id_react;

bool ByondReaction::check_conditions(GasMixture& mix)
{
	auto temp = mix.get_temperature();
	auto ener = mix.thermal_energy();
    if(temp < min_temp_req || ener < min_ener_req)
    {
        return false;
    }
    else
    {
        for(int i = 0; i < TOTAL_NUM_GASES; i++)
        {
            if(mix.get_moles(i) < min_gas_reqs[i])
            {
                return false;
            }
        }
    }
    return true;
}

int ByondReaction::react(GasMixture& mix,ManagedValue src,ManagedValue holder)
{
    return (int)(float)(Core::get_proc(proc_id).call({src,holder}));
}