#include "BeingBuilder.h"
using namespace godot;

void BeingBuilder::_gd_set_name(const String& name){setName(name);}
void BeingBuilder::_gd_randomize_name(){randomizeName();}
void BeingBuilder::_gd_set_faction_id(const String& faction_id){setFactionId(faction_id);};
void BeingBuilder::_gd_set_race_id(const String& race_id, const bool controllable_race){setRaceId(race_id, controllable_race);}
void BeingBuilder::_gd_set_klass_id(const String& klass_id){setKlassId(klass_id);}
void BeingBuilder::_gd_randomize_klass_id(){randomizeKlassId();}
void BeingBuilder::_gd_set_head_id(const String& head_id){setHeadId(headId);}
void BeingBuilder::_gd_randomize_head_id(){randomizeHeadId();}
void BeingBuilder::_gd_set_body_id(const String& body_id){setBodyId(body_id);}
void BeingBuilder::_gd_randomize_body_id(){randomizeBodyId();}
void BeingBuilder::_gd_set_head_scale(const Vector3& head_scale){setHeadScale(head_scale);}
void BeingBuilder::_gd_set_body_scale(const Vector3& body_scale){setBodyScale(body_scale);}
void BeingBuilder::_gd_set_extra_health_multiplier(const float multiplier){setExtraHealthMultiplier(extraHealthMultiplier);}

//TODO CARGAR TODAS LAS RACES, KLASSES AL C++ SIDE PA VER SI EXISTE LA ID PASADA AL CONSTRUIR DENTRO DEL DICT. VAN A HABER MENOS BUGS DE IF MAL-ESCRITAS EN GENERAL

BeingBuilder& BeingBuilder::setName(const String& name)
{this->name = name; return *this;}
BeingBuilder& BeingBuilder::randomizeName()
{this->name = "random"; return *this;}

BeingBuilder& BeingBuilder::setFactionId(const String& factionId)
{this->factionId = factionId; return *this;}

BeingBuilder& BeingBuilder::setRaceId(const String& raceId, bool controllableRace)
{this->raceId = raceId; this->controllableRace = controllableRace; return *this;}

BeingBuilder& BeingBuilder::setKlassId(const String& klassId)
{this->klassId = klassId; return *this;}
BeingBuilder& BeingBuilder::randomizeKlassId()
{this->klassId = "random"; return *this;}

BeingBuilder& BeingBuilder::setHeadId(const String& headId)
{this->headId = headId; return *this;}
BeingBuilder& BeingBuilder::randomizeHeadId()
{this->headId = "random"; return *this;}

BeingBuilder& BeingBuilder::setBodyId(const String& bodyId)
{this->bodyId = bodyId; return *this;}
BeingBuilder& BeingBuilder::randomizeBodyId()
{this->bodyId = "random"; return *this;}

BeingBuilder& BeingBuilder::setHeadScale(const Vector3& headScale)//agregar reset
{this->headScale = headScale; return *this;}

BeingBuilder& BeingBuilder::setBodyScale(const Vector3& bodyScale)//agregar reset
{this->bodyScale = bodyScale; return *this;}

BeingBuilder& BeingBuilder::setExtraHealthMultiplier(const float extraHealthMultiplier)//agregar reset
{this->extraHealthMultiplier = extraHealthMultiplier; return *this;}

// also sets validationFailed to true
void BeingBuilder::print_failed_validation(bool& validationFailedWasAlreadyPrinted) const
{
    if(validationFailedWasAlreadyPrinted == false){
        validationFailedWasAlreadyPrinted = true;
        UtilityFunctions::printerr("Couldn't validate (BeingBuilder.cpp::build()). causes:");
    }
}

bool BeingBuilder::isValidBeing() const
{
    bool validationFailed = false;

    if(extraHealthMultiplier <= 0)
    {
        print_failed_validation(validationFailed);
        UtilityFunctions::printerr("-specified extraHealthMultiplier less or equal to zero");
    }

    if(raceId == "")
    {
        print_failed_validation(validationFailed);
        UtilityFunctions::printerr("-race id not set");
    }
    else if(!controllableRace && klassId != "random")
    {
        print_failed_validation(validationFailed);
        UtilityFunctions::printerr("-klass was specified, but race is not a controllable one");
    }

    if(factionId == "")
    {
        print_failed_validation(validationFailed);
        UtilityFunctions::printerr("-faction id not set");
    }

    if (validationFailed == false) {return true;}

    return false;
}

bool BeingBuilder::build()
{
    if (isValidBeing())
    {
        Dictionary data;
        data["name"] = name;
        data["fac"] = factionId;
        data["race"] = raceId;
        if (controllableRace) {data["klass"] = klassId;}
        
        data["head"] = headId;
        data["body"] = bodyId;
        data["head_scale"] = headScale;
        data["body_scale"] = bodyScale;
        data["eh"] = extraHealthMultiplier;

        data.make_read_only();
        builtBeing = std::make_optional<Dictionary>(data);
        return true;
    }
    // builtBeing is reset to null
    builtBeing = std::nullopt;
    return false;
}

std::optional<Dictionary> BeingBuilder::getResult() const
{return builtBeing;}

// USE THIS METHOD ONLY in GDscript, not in c++
Dictionary BeingBuilder::_gd_get_result() const
{
    if (builtBeing.has_value())
    {
        return builtBeing.value();
    }
    Dictionary emptyDict; emptyDict.make_read_only();
    return emptyDict;
}
BeingBuilder::BeingBuilder(){}
BeingBuilder::~BeingBuilder(){}
void BeingBuilder::_bind_methods()
{
    ClassDB::bind_method(D_METHOD("set_name", "name"), &BeingBuilder::_gd_set_name);
    ClassDB::bind_method(D_METHOD("set_faction_id", "faction_id"), &BeingBuilder::_gd_set_faction_id);
    ClassDB::bind_method(D_METHOD("set_race_id", "race_id", "controllable_race"), &BeingBuilder::_gd_set_race_id);
    ClassDB::bind_method(D_METHOD("set_klass_id", "klass_id"), &BeingBuilder::_gd_set_klass_id);
    ClassDB::bind_method(D_METHOD("randomize_klass_id"), &BeingBuilder::_gd_randomize_klass_id);
    ClassDB::bind_method(D_METHOD("set_head_id", "head_id"), &BeingBuilder::_gd_set_head_id);
    ClassDB::bind_method(D_METHOD("randomize_head_id"), &BeingBuilder::_gd_randomize_head_id);
    ClassDB::bind_method(D_METHOD("set_body_id", "body_id"), &BeingBuilder::_gd_set_body_id);
    ClassDB::bind_method(D_METHOD("randomize_body_id"), &BeingBuilder::_gd_randomize_body_id);
    ClassDB::bind_method(D_METHOD("set_head_scale", "head_scale"), &BeingBuilder::_gd_set_head_scale);
    ClassDB::bind_method(D_METHOD("set_body_scale", "body_scale"), &BeingBuilder::_gd_set_body_scale);
    ClassDB::bind_method(D_METHOD("set_extra_health_multiplier", "extra_health_multiplier"), &BeingBuilder::_gd_set_extra_health_multiplier);
    //TODO PONER LOS ATTRIBUTES COMO PROPERTIES

    ClassDB::bind_method(D_METHOD("build"), &BeingBuilder::build);

    ClassDB::bind_method(D_METHOD("get_result"), &BeingBuilder::_gd_get_result);
}