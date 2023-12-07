#ifndef __ARGENTUMTILEMAP_H__
#define __ARGENTUMTILEMAP_H__

#include "WorldMatrix.cpp"
#include "FormationGenerator.h"

#include "SpawningModule.h"

#include <godot_cpp/classes/tile_map.hpp>
#include <godot_cpp/variant/typed_array.hpp>
#include <godot_cpp/godot.hpp>
#include <unordered_set>  
#include <unordered_map>
#include <memory>

template<class T, size_t N> 
struct std::hash<std::array<T, N>> {
    std::hash<T> hasher;
    auto operator() (const std::array<T, N>& key) const {
        size_t result = 0;
        for(size_t i = 0; i < N; ++i) {
            result = result * 31 + hasher(key[i]); 
        }
        return result;
    }
};

namespace godot 
{//NO PONER CUERPOS DE MÉTODOS EN LOS HEADER FILES (AUNQUE ESTÉN VACÍOS). PUEDE CAUSAR PROBLEMAS DE LINKING AL COMPILAR 
static const Vector2i ERROR_VECTOR = {-999999, -999999};    
class FormationGenerator;
class ArgentumTileMap : public TileMap{ GDCLASS(ArgentumTileMap, TileMap)

public:
    ArgentumTileMap();
    ~ArgentumTileMap();

    std::optional<u_int16_t> findTileUid(const StringName& stringId) const;
    StringName getTileId(uint16_t uid) const;

    bool persist(String filename);

    //SOLO USAR PARA FORMATIONS
    void placeFormationTile(
        const SafeVec& formationOrigin, const SafeVec& tileCoordsRelativeToFormationOrigin, 
        const uint16_t optTileUid, bool deleteOthers = false);
        
    
    //TODO algún método para escribir en un archivo el estado del mapa actual (intentar escribir en el .tres?)
    //TODO algún método para cargar el worldMatrix a partir de un archivo
    
    int get_seed(); void set_seed(int seed);//global seed (picks random seeds for generations with a seeded gdscript RNG)

    Dictionary get_tiles_data(); void set_tiles_data(const Dictionary& data); void add_tiles_data(const Dictionary& data);

    Vector2i get_random_coord_with_tile_id(const Vector2i& top_left_corner, const Vector2i& bottom_right_corner, const String& tile_id) const;

//todo algun método para placear/cambiar/modificar/buildear tiles cuando ya se está ingame y q se guarden en un hashmap con las modificadas

    
    void freeze_and_store_being(const Vector2& glb_coords, const int individual_unique_id);

    void generate_world_matrix(const Vector2i& size, const Dictionary& tiles_data);
    void generate_formation(const Ref<FormationGenerator>& formation_generator, const Vector2i& origin, const Vector2i& size, 
            const Ref<Resource>& tileSelectionSet, signed int seed, const Dictionary& data);
    
    void load_tiles_around(const Vector2& coords, const Vector2i& chunk_size, const int uid);
    void unloadExcessTiles(const SafeVec& topLeftCornerCoords, const SafeVec& CHUNK_SIZE, const int uid);

    void set_beings_in_chunk_count(const TypedArray<Array> beings_in_chunk_count);
    TypedArray<Array> get_beings_in_chunk_count();
    


private:
    int seed = 0;
    Dictionary m_tiles_data;
    void replaceTilesDataProperly(const Dictionary& input_tiles_data);
    std::vector<StringName> mTilesUidMapping;   

    std::unordered_map<std::string, SafeVec> mTrackedBeingsCoords;//updateado cada

    std::unique_ptr<WorldMatrix> mWorldMatrixPtr = nullptr;
    std::unique_ptr<SpawningModule> mNaturalSpawningModule = nullptr;//HACER SUBCLASS DE mWorldMatrixPtr?

    //chunk size: 7x7 puntos de spawnweights
    //TypedArray<TypedArray<long>>
    TypedArray<Array> beings_in_chunk_count;
    
    //contiene ids de BeingKinds

    std::unordered_map<SafeVec, std::vector<std::pair<Vector2, int>>, SafeVec::hash> mFrozenBeings;

    //el String es la uniqueid del being específico (individuo). esta unique id es pasada al GDscript-side cuando toca spawnear, 
    //en donde según la id extrae el being de un dictionary q tiene guardado

    void decrementSharedCount(const SafeVec& tileCoord);

    //keeps track of tracks of the tiles loaded by the being with the specific uid (int)
    std::unordered_map<int, std::unordered_set<SafeVec, SafeVec::hash>> mBeingLoadedTiles;

    std::unordered_map<SafeVec, int, SafeVec::hash> mTileSharedLoadsCount;

    std::unordered_map<StringName, std::unordered_map<StringName, Variant>> mCppTilesData;
    static bool withinChunkBounds(const SafeVec &loadedCoordToCheck, const SafeVec &topLeftCorner, const SafeVec &chunkSize);

    bool setCell(const uint16_t uid, const SafeVec& coords);

    void doGlobalSpawnAttempts();

    static bool exceedsTileLimit(const u_int16_t count)
    {
        if (count >= WorldMatrix::NULL_TILE_UID - 1){
            UtilityFunctions::printerr("too many tiles (GlobalDataInterface.cpp::set_tiles())");
            return true;
        }
        return false;
    }
protected: static void _bind_methods();
};
}

#endif // __ARGENTUMTILEMAP_H__