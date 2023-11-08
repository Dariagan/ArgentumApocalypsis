#include "ArgentumTileMap.h"

#include <godot_cpp/core/defs.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/typed_array.hpp>
#include <godot_cpp/variant/utility_functions.hpp>
#include <regex>
#include <string>
#include <typeinfo>

using namespace godot;

void ArgentumTileMap::generate_formation(const Ref<FormationGenerator>& formation_generator, const Vector2i& origin, 
    const Vector2i& size, const Ref<Resource>& tileSelectionSet, signed int seed, const Dictionary& data)
{
    if (size.x < 0 || size.y < 0)
    {
        UtilityFunctions::printerr("Negatively sized formation not allowed");
        return;
    }

    bool outOfBoundsEast = origin.x + size.x > worldSize.x;
    bool outOfBoundsSouth = origin.y + size.y > worldSize.y;
    bool negativeOrigin = origin.x < 0 || origin.y < 0;
    bool outOfBounds = outOfBoundsEast || outOfBoundsSouth || negativeOrigin;

    if (outOfBounds){
        UtilityFunctions::printerr("CANCELLED FORMATION: out of bounds in the world matrix. make the world matrix bigger, move the origin, or resize the formation. Reasons:");
        if(negativeOrigin) UtilityFunctions::printerr("-Negative origin is never allowed for any formation");
        if(outOfBoundsEast||outOfBoundsSouth){
            UtilityFunctions::printerr("-Formation starting at:",origin," of size:",size," is out of worldbounds: ");
            if(outOfBoundsEast) UtilityFunctions::printerr("    -East");
            if(outOfBoundsSouth) UtilityFunctions::printerr("   -South");
        }
    }
    else
    {
        formation_generator->generate(worldMatrix, MatrixCoords(origin), MatrixCoords(size), tileSelectionSet, seed, data);
        emit_signal("formation_formed");
    }
}

void ArgentumTileMap::load_tiles_around(const Vector2& coords, const Vector2i& chunk_size)
{    
    Vector2i beingCoords = local_to_map(coords);
    
    for (int i = -chunk_size.x/2; i < chunk_size.x/2; i++) {
    for (int j = -chunk_size.y/2; j < chunk_size.y/2; j++) 
    {
        const Vector2i matrixPos(beingCoords.x + i, beingCoords.y + j);
        if (matrixPos.x < worldSize.x && matrixPos.y < worldSize.y && matrixPos.x >= 0 && matrixPos.y >= 0)
        {
            const MatrixCoords tileMapTileCoords(beingCoords.x + i, beingCoords.y + j);
            if (loadedTiles.count(tileMapTileCoords) == 0)
            {
                if (worldMatrix[matrixPos.x][matrixPos.y].size() > 0)
                {
                    for (const std::string& TILE_ID : worldMatrix[matrixPos.x][matrixPos.y])
                    {					
                    std::unordered_map<StringName, Variant> tileData;
                    try{tileData = cppTilesData.at(TILE_ID);}
                    catch(const std::out_of_range& e)
                    {UtilityFunctions::printerr(TILE_ID.c_str(), 
                    " not found in cppTilesData (at ArgentumTileMap.cpp::load_tiles_around())");}

                    //TODO HACER EL AUTOTILING MANUALMENTE EN ESTA PARTE SEGÚN LAS 4 TILES Q SE TENGA ADYACENTES EN LA WORLDMATRIX
                    //PONER EL AGUA EN UNA LAYER INFERIOR? 

                    MatrixCoords atlasOriginPosition; 
                        
                    try{
                        atlasOriginPosition = ((Vector2i)tileData.at("op"));
                    }
                    catch(...){UtilityFunctions::printerr("couldn't get atlas origin position for tile_id \"",TILE_ID.c_str(),"\" (at ArgentumTileMap.cpp::load_tiles_around())");
                    continue;}

                    MatrixCoords atlasPositionOffset(0, 0);

                    try{
                        MatrixCoords moduloTilePickingArea = (Vector2i)tileData.at("ma");
                        if(moduloTilePickingArea.i >= 1 && moduloTilePickingArea.j >= 1)
                        {
                            atlasPositionOffset.i = matrixPos.x % moduloTilePickingArea.i;
                            atlasPositionOffset.j = matrixPos.y % moduloTilePickingArea.j;
                        }
                        else UtilityFunctions::printerr("non-positive moduloTilePickingArea ",moduloTilePickingArea.c_str(),"is not admitted. tile_id:",TILE_ID.c_str()," (at ArgentumTileMap.cpp::load_tiles_around())");
                    }
                    catch(...){UtilityFunctions::printerr("couldn't access \"ma\" key for ", TILE_ID.c_str()," (at ArgentumTileMap.cpp::load_tiles_around())");}
            
                    //hacer q la atlas positionV se mueva según el mod de la global position, dentro de la tile 4x4
                    set_cell(tileData.at("layer"), tileMapTileCoords, tileData.at("source_id"), 
                            atlasOriginPosition + atlasPositionOffset, tileData.at("alt_id"));
                    }
                }else
                {
                    //TODO (matrixpos.x)%4 && (matrixpos.y)%4->  
                    set_cell(0, tileMapTileCoords, 2, Vector2i(6,0), 0);
                }                   
                loadedTiles.insert(tileMapTileCoords);
            }
        }
    }}unloadExcessTiles(beingCoords);
}

//todo hacer en vez de por distancia q se borren las tiles de loadedtiles cuyas coords no esten dentro del cuadrado actual
void ArgentumTileMap::unloadExcessTiles(const Vector2i& coords)//coords PUEDE SER NEGATIVO, ARREGLAR
{
    const int MAX_LOADED_TILES = 30000;

    if (loadedTiles.size() > MAX_LOADED_TILES)
    {
        std::vector<MatrixCoords> tilesToErase;
        for (const MatrixCoords& tileCoord : loadedTiles)
        {
            if (tileCoord.distanceSquaredTo(coords) > 27000)
            {
                for (int layer_i = 0; layer_i < get_layers_count(); layer_i++)
                {
                    erase_cell(layer_i, tileCoord);
                }
                tilesToErase.push_back(tileCoord);
            }			
        }
        for (const MatrixCoords& tileCoord : tilesToErase){
            loadedTiles.erase(tileCoord);
        }
    }
}

void ArgentumTileMap::generate_world_matrix(const Vector2i& size)
{
    if (size.x < 0 || size.y < 0)
    {
        UtilityFunctions::printerr("Negatively sized world matrix not allowed");
        return;
    }

    if(worldMatrix.size() == 0)
    {
        worldMatrix.resize(size.x, std::vector<std::vector<std::string>>(size.y, std::vector<std::string>()));

        this->worldSize = size;
    } else{
        UtilityFunctions::printerr("World matrix was already generated, cannot be re-generated.");
    }
}

int ArgentumTileMap::get_seed(){return seed;}; 
void ArgentumTileMap::set_seed(signed int seed){this->seed = seed;};

Dictionary ArgentumTileMap::get_tiles_data(){return tiles_data;}; 
void ArgentumTileMap::set_tiles_data(Dictionary tiles_data)
{
    this->tiles_data = tiles_data;
    for(auto &tileData : cppTilesData)
        tileData.second.clear();
    cppTilesData.clear();
    
    for(int i = 0; i < tiles_data.values().size(); i++)
    {
        Ref<Resource> tile = Object::cast_to<Resource>(tiles_data.values()[i]);

        Dictionary tile_data = tile->call("get_data");

        std::unordered_map<StringName, Variant> tileData;

        for(int j = 0; j < tile_data.values().size(); j++)
        {
            tileData.insert({tile_data.keys()[j], tile_data.values()[j]});
        }
        std::string keyAsCppString = ((String)tiles_data.keys()[i]).utf8().get_data();
        cppTilesData.insert({keyAsCppString, tileData});                
    }
};

ArgentumTileMap::ArgentumTileMap(){}

ArgentumTileMap::~ArgentumTileMap(){}

void ArgentumTileMap::_bind_methods()
{   
    //TODO hacer el tileselector compartible de alguna forma así no hay q hacer las godot calls cada vez q se llame un formation generator?
    ClassDB::bind_method(D_METHOD("generate_formation", "formation_generator", "origin", "size", "tile_selection_set", "seed", "data"), &ArgentumTileMap::generate_formation);
    ClassDB::bind_method(D_METHOD("generate_world_matrix", "size"), &ArgentumTileMap::generate_world_matrix);
    
    ClassDB::bind_method(D_METHOD("load_tiles_around", "coords", "chunk_size"), &ArgentumTileMap::load_tiles_around);

    ClassDB::bind_method(D_METHOD("set_seed", "seed"), &ArgentumTileMap::set_seed);
    ClassDB::bind_method(D_METHOD("get_seed"), &ArgentumTileMap::get_seed);
    ADD_PROPERTY(PropertyInfo(Variant::INT, "seed"), "set_seed", "get_seed");

    ClassDB::bind_method(D_METHOD("set_tiles_data", "tiles_data"), &ArgentumTileMap::set_tiles_data);
    ClassDB::bind_method(D_METHOD("get_tiles_data"), &ArgentumTileMap::get_tiles_data);
    ADD_PROPERTY(PropertyInfo(Variant::DICTIONARY, "tiles_data"), "set_tiles_data", "get_tiles_data");

    ADD_SIGNAL(MethodInfo("formation_formed"));
}