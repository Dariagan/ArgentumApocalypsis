#include "BeingsModule.h"

using namespace godot;


void BeingsModule::placeNaturalSpawningWeight(
    const SafeVec formationOrigin, const SafeVec coordsRelativeToFormationOrigin, 
    const uint16_t& beingKindId, const unsigned char weight, bool deleteOthers)
{
    let absoluteCoordinates = formationOrigin + coordsRelativeToFormationOrigin;

    if(deleteOthers) {mSpawnWeightsMatrix->clearAt(absoluteCoordinates);}
    
    (*mSpawnWeightsMatrix)[absoluteCoordinates][beingKindId] = weight;
}

//ojo estas coords son absolutas, no relativas al origin de la formation
void BeingsModule::birthBeing(const Vector2i coords, const BeingBuilder& beingBuilder)
{
    if(beingBuilder.getResult().has_value())
    {
        mArgentumTileMap->emit_signal("birth_being_w_init_data", coords, beingBuilder.getResult().value());
    }
}

void BeingsModule::birthBeingOfKind(const Vector2i local_coords, const String& being_kind_id)
{mArgentumTileMap->emit_signal("birth_of_being_of_kind", local_coords, being_kind_id);}

void BeingsModule::updateChunkBeingCounts()
{
    
}

BeingsModule::BeingsModule(godot::ArgentumTileMap* argentumTileMap, const SafeVec size)
{
    this->mArgentumTileMap = argentumTileMap;
    this->mSpawnWeightsMatrix = std::make_unique<SpawnWeightsMatrix>(size);
    this->mBeingsInChunkCount = std::make_unique<matrix<u_int16_t>>(size, MACROSCOPIC_SPAWNING_CHUNK_SIZE);
}

BeingsModule::~BeingsModule()
{
    
}
