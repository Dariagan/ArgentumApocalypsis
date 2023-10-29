#ifndef __ARGENTUMTILEMAP_H__
#define __ARGENTUMTILEMAP_H__
#include <godot_cpp/classes/tile_map.hpp>
#include <godot_cpp/variant/typed_array.hpp>
#include <godot_cpp/godot.hpp>
#include <unordered_set>  
#include <unordered_map>
#include "FormationGenerator.h"
#include "TilePicker.h"

namespace godot {

    class ArgentumTileMap : public TileMap{
        GDCLASS(ArgentumTileMap, TileMap)

        struct pair_hash {inline std::size_t operator()(const Vector2i & v) const {return v.x*31+v.y;}};

        private:
            std::vector<std::vector<std::vector<std::string>>> worldMatrix;
            Vector2i worldSize;
            bool worldGenerated = false;
            std::unordered_set<Vector2i, pair_hash> loadedTiles;
            std::unordered_map<std::string, std::unordered_map<StringName, Variant>> cppTilesData;
            
        protected:
            static void _bind_methods();

        public:
            ArgentumTileMap();
            ~ArgentumTileMap();

            signed int seed = 0; int get_seed(); void set_seed(signed int seed);

            Dictionary tiles_data; Dictionary get_tiles_data(); void set_tiles_data(Dictionary data);

            void generate_world_matrix(const Vector2i& size);
            void generate_formation(const Ref<FormationGenerator>& formation_generator, const Vector2i& origin, const Vector2i& size, 
                 const TileSelectionSet tileSelectionSet, signed int seed, const Dictionary& data);
            
            void load_tiles_around(const Vector2& coords, const Vector2i& chunk_size);
            void unloadExcessTiles(const Vector2i& coords);
    };
}

#endif // __ARGENTUMTILEMAP_H__