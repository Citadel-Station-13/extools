#pragma once

#include "GasMixture.h"

#include "../core/core.h"

#include "../core/proc_management.h"

extern std::vector<Value> gas_id_to_type;

extern std::unordered_map<unsigned int,float> gas_rarities;

class Reaction
{
    public:
        virtual bool check_conditions(GasMixture& mix) = 0;
        virtual int react(GasMixture& mix,Value src,Value holder) = 0;
        inline float get_priority() { return priority; }
    protected:
        float priority;
};

class ByondReaction : public Reaction
{
    public:
        virtual bool check_conditions(GasMixture& mix);
        virtual int react(GasMixture& mix,Value src,Value holder);
        ByondReaction(Value v) {
            List min_reqs = v.get("min_requirements");
            if(min_reqs.at("TEMP").type == DataType::NUMBER) min_temp_req = min_reqs.at("TEMP");
            if(min_reqs.at("ENER").type == DataType::NUMBER) min_ener_req = min_reqs.at("ENER");
            size_t next_thing = 0;
            for(int i=0;i < TOTAL_NUM_GASES;i++)
            {
                auto gasReq = min_reqs.at(gas_id_to_type[i]);
                if(gasReq.type == DataType::NUMBER)
                {
                    min_gas_reqs[i] = (float)gasReq;
                    next_thing++;
                }
            }
            priority = v.get("priority");
            auto proc = Core::try_get_proc(Core::stringify(v.get("type")) + "/react");
            if(!proc)
            {
                Core::alert_dd("Could not find proc for reaction! " + Core::stringify(v.get("type")) + "/react");
            }
            else
            {
                Core::alert_dd("Found proc for reaction: " + Core::stringify(v.get("type")) + "/react");
            }
            proc_id = proc->id;
        }
    private:
        float min_temp_req = -1.0;
        float min_ener_req = -1.0;
        float min_gas_reqs[TOTAL_NUM_GASES] = {-1.0};
        unsigned int proc_id;
};

class PlasmaFire : public Reaction
{
    public:
        virtual bool check_conditions(GasMixture& mix);
        virtual int react(GasMixture& mix,Value src,Value holder);
    protected:
        int priority = -2;
};

class TritFire : public Reaction
{
    public:
        virtual bool check_conditions(GasMixture& mix);
        virtual int react(GasMixture& mix,Value src,Value holder);
    protected:
        int priority = -1;
};

class Fusion : public Reaction
{
    public:
        virtual bool check_conditions(GasMixture& mix);
        virtual int react(GasMixture& mix,Value src,Value holder);
    protected:
        int priority = 2;
};