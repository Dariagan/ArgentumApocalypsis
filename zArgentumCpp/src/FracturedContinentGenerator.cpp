#include "FracturedContinentGenerator.h"
#include <godot_cpp/core/defs.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/utility_functions.hpp>
#include <unordered_set>
#include <algorithm>
#include <string>
#include <format>
using namespace godot;

FracturedContinentGenerator::FracturedContinentGenerator()
{
    continenter.set_noise_type(FastNoiseLite::NoiseType::TYPE_SIMPLEX);

    peninsuler.set_noise_type(FastNoiseLite::NoiseType::TYPE_SIMPLEX);
    
    bigLaker.set_noise_type(FastNoiseLite::NoiseType::TYPE_VALUE_CUBIC);
    smallLaker.set_noise_type(FastNoiseLite::NoiseType::TYPE_VALUE_CUBIC);
    bigBeacher.set_noise_type(FastNoiseLite::NoiseType::TYPE_SIMPLEX_SMOOTH);
    smallBeacher.set_noise_type(FastNoiseLite::NoiseType::TYPE_SIMPLEX_SMOOTH);
    forest.set_noise_type(FastNoiseLite::NoiseType::TYPE_SIMPLEX);
    
    peninsuler_cutoff = -0.1f; bigLakeCutoff = 0.33f; smallLakeCutoff = 0.25f; beachCutoff = 0.8f, treeCutoff = 4.3f;

    continenter.set_fractal_lacunarity(2.8f); continenter.set_fractal_weighted_strength(0.5f);

    peninsuler.set_fractal_gain(0.56f);

    smallBeacher.set_fractal_octaves(3);

    forest.set_fractal_lacunarity(3); forest.set_fractal_gain(0.77);
}

//! CLEAREAR COLECCIONES QUE SE REUTILIZAN CADA LLAMADA (SINO QUEDAN COMO TERMINARON EN LA EJECUCIÓN DE ANTERIOR, LLENAS DE ELEMENTOS)
void FracturedContinentGenerator::resetState()
{
    m_trees.clear(); m_bushes.clear(); 
}

//definir spawn points para bosses?
//es alpedo definir spawn points para mobs comúnes basados en condiciones super específicas porq merondean igualmente
void FracturedContinentGenerator::generate(
    ArgentumTileMap& argentumTileMap, //HACER Q SEA DE UN SOLO USE ONLY? (PARA PREVENIR BUGS)
    const SafeVec& origin, const SafeVec& size, const Ref<Resource>& tileSelectionSet, 
    const unsigned int SEED, const Dictionary& data)
{
    //TODO posible randomización leve de parámetros
    this->m_origin = origin; this->m_size = size;

    this->m_tileSelector = std::make_unique<TileSelector>(tileSelectionSet, SEED);

    {
    continenter.set_seed(SEED); peninsuler.set_seed(SEED+1); bigLaker.set_seed(SEED+2); smallLaker.set_seed(SEED+3);
    bigBeacher.set_seed(SEED+4); smallBeacher.set_seed(SEED+5); m_rng.set_seed(SEED); forest.set_seed(SEED + 9);
    
    continenter.set_frequency(0.15f/powf(size.length(), 0.995f)); peninsuler.set_frequency(5.f/powf(size.length(), 0.995f));
    bigBeacher.set_frequency(4.3f/powf(size.length(), 0.995f)); smallBeacher.set_frequency(8.f/powf(size.length(), 0.995f));
    bigLaker.set_frequency(40.f/powf(size.length(), 0.995f)); smallLaker.set_frequency(80.f/powf(size.length(), 0.995f));
    forest.set_frequency(1.8f/powf(size.length(), 0.995f));
    //TODO hacer cada frequency ajustable desde gdscript

    continental_cutoff = 0.61f * powf(size.length() / 1600.f, 0.05f);;
    }

    while(continenter.get_noise_2dv(origin+size/(short int)2) < continental_cutoff + 0.13f){//! NO METER EL PENINSULER EN ESTA CONDICIÓN, DESCENTRA LA FORMACIÓN
//EN EL CENTRO PUEDE ESTAR PENINSULEADO, HACIENDO Q EL PLAYER SPAWNEE EN EL AGUA SI EL CENTRO TIENE AGUA (EL PLAYER SPAWNEARÍA EN EL CENTRO).
// ASÍ QUE, PARA SPAWNEAR AL PLAYER ELEGIR UN PUNTO RANDOM HASTA Q TENGA UNA TILE TIERRA (COMO HAGO CON LOS DUNGEONS) 
        continenter.set_offset(continenter.get_offset() + Vector3(3,3,0));
    }


//TODO hacer 4 threads?

//CÓMO HACER RIOS: ELEGIR PUNTO RANDOM DE ALTA CONTINENTNESS -> "CAMINAR HACIA LA TILE ADYACENTE CON CONTINENTNESS MAS BAJA" -> HACER HASTA LLEGAR AL AGUA O LAKE
    for (uint16_t x = 0; x < size.lef; x++){
    for (uint16_t y = 0; y < size.RIGHT; y++)
    {
        const SafeVec coords(x, y);

        const bool CONTINENTAL = isContinental(coords);

        const bool PENINSULER_CAVED = isPeninsulerCaved(coords);

        constexpr char MAX_POSSIBLE_ENTRIES = 3;

//! POTENCIAL BUG: STACK OVERFLOW SI SE ESCRIBE EN UN addedTargetsCount[i] CON i SIENDO MAYOR QUE EL TAMAÑO DEL ARRAY - 1
        std::array<std::array<char, 32>, MAX_POSSIBLE_ENTRIES> targetsToFill;
        
        unsigned char addedTargetsCount = 0;

        if (CONTINENTAL && ! PENINSULER_CAVED)
        {            
            const float BEACHNESS = getBeachness(coords);
            const bool BEACH = BEACHNESS > beachCutoff;
            
            // un array de chars = un string

            // lo que hace el strncpy es copiar el array de chars "beach" dentro de la posición <addedTargetsCount> del array de arrays de chars "targetsToFill"
            // el string "beach" (un string es un array de chars)
            if (BEACH) 
                strncpy(&targetsToFill[addedTargetsCount++][0], "beach", sizeof(targetsToFill[0]));
            else
            {
                const bool AWAY_FROM_COAST = getContinentness(coords) > continental_cutoff + 0.01f && peninsuler.get_noise_2dv(coords) > peninsuler_cutoff + 0.27f;
                
                const bool LAKE = isLake(coords) && AWAY_FROM_COAST;

                if (LAKE) 
                    strncpy(&targetsToFill[addedTargetsCount++][0], "lake", sizeof(targetsToFill[0]));//! addedTargetsCount == 1
                else 
                {
                    strncpy(&targetsToFill[addedTargetsCount++][0], "cont", sizeof(targetsToFill[0]));//! addedTargetsCount == 1
                    if(!BEACHNESS < beachCutoff - 0.05f ) 
                    {
                        // HAY Q USAR UNA DISCRETE DISTRIBUTION PLANA EN EL MEDIO, MU BAJA PROBABILIDAD EN LOS EXTREMOS
                        const bool GOOD_DICE_ROLL = m_rng.randf_range(0, 4) + forest.get_noise_2dv(coords) * 1.4f > treeCutoff;
                        const bool LUCKY_TREE = m_rng.randi_range(0, 1000) == 0;

                        const bool TREE = (LUCKY_TREE || GOOD_DICE_ROLL) && clearOf(m_trees, coords, 3);
                        if (TREE)
                        {
                            m_trees.insert(coords);
                            strncpy(&targetsToFill[addedTargetsCount++][0], "tree", sizeof(targetsToFill[0]));//! addedTargetsCount == 2
                        }
                        else if(m_rng.randi_range(0, 400) == 0 && clearOf(m_bushes, coords, 1)) {
                            m_bushes.insert(coords);
                            strncpy(&targetsToFill[addedTargetsCount++][0], "bush", sizeof(targetsToFill[0]));//! addedTargetsCount == 2
                        }
                        //! NO ESCRIBIR POR LA DERECHA DEL PROPIO ARRAY 
                        //! AL ESCRIBIR EN EL ARRAY targetsToFill,
                        //! PREVENIR QUE: addedTargetsCount >= MAX_POSSIBLE_ENTRIES,
                        //! EN TODOS LOS POSIBLES RECORRIDOS DE EJECUCIÓN
                    }
                }
            }
        } else 
            {strncpy(&targetsToFill[addedTargetsCount++][0], "ocean", sizeof(targetsToFill[0]));};// addedTargetsCount=0 //TODO HACER ESTO UN MACRO

        //todo poner los spawnweights con targets, como haces con las tiles
    
        std::array<char, 32> temp = {'a','s','d',0}; 
        argentumTileMap.placeSpawnWeight(origin, coords, temp, 10);

//shallow ocean: donde continentness está high. deep ocean: donde continentness está low o si se es una empty tile fuera de cualquier generation
        for(unsigned char k = 0; k < addedTargetsCount; k++)
        {
            const auto tileId = this->m_tileSelector->getTileId(targetsToFill[k]);
            argentumTileMap.placeFormationTile(origin, coords, tileId);
        }
        
    }}
    placeDungeonEntrances(argentumTileMap, 3);

    this->resetState();
}

bool FracturedContinentGenerator::isContinental(SafeVec coords) const
{return getContinentness(coords) > continental_cutoff;}


float FracturedContinentGenerator::getContinentness(SafeVec coords) const
{
    const float BCF = FormationGenerator::getBorderClosenessFactor(coords, m_size);

    return continenter.get_noise_2dv(coords) * (1-BCF);
}
float FracturedContinentGenerator::getBeachness(SafeVec coords) const
{
    return std::max(
    0.72f + bigBeacher.get_noise_2dv(coords) / 2.3f - powf(getContinentness(coords) - continental_cutoff, 0.6f), 
    0.8f + smallBeacher.get_noise_2dv(coords) / 2.3f - powf(peninsuler.get_noise_2dv(coords) - peninsuler_cutoff, 0.45f));
}

bool FracturedContinentGenerator::clearOf(
    const std::unordered_set<SafeVec, SafeVec::hash>& setToCheck, SafeVec coords, uint16_t radius, bool checkForwards) const
{ 
    for (int x = -radius; x <= checkForwards * radius; x++)
        for (int y = -radius; y <= radius; y++){
            if (setToCheck.count(SafeVec(coords.lef+x, coords.RIGHT+y)))
                return false;
        }
    return true;
}

bool FracturedContinentGenerator::isPeninsulerCaved(SafeVec coords) const
{return peninsuler.get_noise_2dv(coords) < peninsuler_cutoff;}

bool FracturedContinentGenerator::isLake(SafeVec coords) const
{
    return (((smallLaker.get_noise_2dv(coords) + 1)*0.65f) - getBeachness(coords) > smallLakeCutoff) 
        || (((bigLaker.get_noise_2dv(coords) + 1)*0.65f) - getBeachness(coords) > bigLakeCutoff);
}

//MUST BE CALLED AFTER TREES/ROCKS/WHATEVER BLOCKING OBJECTS ARE INSERTED
void FracturedContinentGenerator::placeDungeonEntrances(
    godot::ArgentumTileMap& argentumTileMap, const unsigned char dungeonsToPlace)
{
    constexpr int MAX_TRIES = 1'000'000;

    std::vector<SafeVec> placedDungeonsCoords(dungeonsToPlace);

    float minDistanceMultiplier = 1;
    float triesCount = 1;
    for (; placedDungeonsCoords.size() < dungeonsToPlace; triesCount++)
    {
        const SafeVec rCoords(m_rng.randi_range(0, m_size.lef), m_rng.randi_range(0, m_size.RIGHT));

        if (getContinentness(rCoords) > continental_cutoff + 0.005 
        && peninsuler.get_noise_2dv(rCoords) > peninsuler_cutoff + 0.1f 
        && !isLake(rCoords) && clearOf(m_trees, rCoords, 3, true))
        {
            const float minDistanceBetweenDungeons = m_size.length() * 0.25f * minDistanceMultiplier;

            const auto isTooClose = [&](const auto& coord){return rCoords.distanceTo(coord) <  minDistanceBetweenDungeons;};

            if (std::find_if(placedDungeonsCoords.begin(), placedDungeonsCoords.end(), isTooClose) 
                == placedDungeonsCoords.end())
            {
                placedDungeonsCoords.push_back(rCoords);
                std::array<char, 32> buffer;
                sprintf(&buffer[0], "cave_%lu", placedDungeonsCoords.size()-1);

                const auto tileId = this->m_tileSelector->getTileId(buffer);
                argentumTileMap.placeFormationTile(m_origin, rCoords, tileId);
                UtilityFunctions::print((Vector2i)rCoords);
            }
            else{minDistanceMultiplier = std::clamp(1500.f / triesCount, 0.f, 1.f);}
        }
        if (triesCount > MAX_TRIES){
            UtilityFunctions::printerr("Dungeon placement condition unmeetable! (FracturedContinentGenerator::placeDungeonEntrances())");
            break;
        }
    }
}
float FracturedContinentGenerator::get_continental_cutoff()const{return continental_cutoff;}
void FracturedContinentGenerator::set_continental_cutoff(float cutoff){continental_cutoff = cutoff;} 
void FracturedContinentGenerator::_bind_methods()
{
    ClassDB::bind_method(D_METHOD("set_continental_cutoff", "continental_cutoff"), &FracturedContinentGenerator::set_continental_cutoff);
    ClassDB::bind_method(D_METHOD("get_continental_cutoff"), &FracturedContinentGenerator::get_continental_cutoff);
    ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "continental_cutoff"), "set_continental_cutoff", "get_continental_cutoff");
}
FracturedContinentGenerator::~FracturedContinentGenerator(){}