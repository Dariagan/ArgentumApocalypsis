#ifndef GLOBAL_DATA_SINGLETON
#define GLOBAL_DATA_SINGLETON
#include <godot_cpp/classes/node.hpp>
#include <godot_cpp/variant/string.hpp>
#include <godot_cpp/variant/dictionary.hpp>
#include <godot_cpp/core/defs.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

#include <godot_cpp/variant/typed_array.hpp>
#include <godot_cpp/godot.hpp>
#include <unordered_map>
#include <algorithm>
#include <optional>
#include <limits>
#include "WorldMatrix.cpp"

namespace godot{

class GlobalData: public Node
{
    GDCLASS(GlobalData, Node)
    
    public:

        Dictionary active_beings;//guarda referencias a Beings. cuando se erasee una tile se mira este diccionario pa ver si la local_to_map(being.position) está en la pos
        //el godot side se tiene q encargar de agregar los beings


        static void add_tiles(const Dictionary& input_tiles)
        {
            tiles.merge(input_tiles, true);
            const u_int16_t NEW_TILES_COUNT = tiles.size();

            if(exceedsTileLimit(NEW_TILES_COUNT)) return;

            tilesUidMapping.reserve(NEW_TILES_COUNT);
            tilesUidMapping.resize(NEW_TILES_COUNT);
            for(u_int16_t i = 0; i < input_tiles.size(); i++)
            {
                const auto& tile_id = input_tiles.keys()[i];

                //if not mapped already
                if(std::find(tilesUidMapping.begin(), tilesUidMapping.end(), tile_id) == tilesUidMapping.end())//TA BIEN
                {//map the key
                    tilesUidMapping.push_back(tile_id);
                }
            }
        }

        static void set_tiles(const Dictionary& input_tiles)
        {
            if(tiles.is_empty())
            {
                const u_int16_t TILES_COUNT = input_tiles.size();

                if(exceedsTileLimit(TILES_COUNT)) return;

                tiles = input_tiles;

                tilesUidMapping.reserve(TILES_COUNT);
                tilesUidMapping.resize(TILES_COUNT);
                for(u_int16_t i = 0; i < TILES_COUNT; i++)
                    {tilesUidMapping[i] = input_tiles.keys()[0];}                    
            }
            else
            {
                tiles.clear();
                add_tiles(input_tiles);
            }
        }
        static Dictionary get_tiles(){return tiles;}
        
        static std::optional<u_int16_t> getTileUid(const StringName& stringId)
        {
            for(u_int16_t i = 0; i < tilesUidMapping.size(); i++)
            {
                if(tilesUidMapping[i] == stringId)
                    return i;
            }
            return {};
        }
        
        //NO USAR, PROBAR SI TE DEJA BORRARLOS:
        GlobalData(){}; ~GlobalData();

    protected:
        static void _bind_methods()
        {
            ClassDB::bind_static_method("GlobalData", D_METHOD("set_tiles", "tiles"), &GlobalData::set_tiles);
            ClassDB::bind_static_method("GlobalData", D_METHOD("add_tiles", "tiles"), &GlobalData::set_tiles);
        };
    private:
        static Dictionary item_data, 
        sprites_datas, 
        races, 
        controllable_races, 
        uncontrollable_races, 
        klasses, 
        tiles, 
        tile_selections
        ;

        static bool exceedsTileLimit(const u_int16_t count)
        {
            if (count >= WorldMatrix::NULL_UID - 1){
                UtilityFunctions::printerr("too many tiles (GlobalData.cpp::set_tiles())");
                return true;
            }
            return false;
        }

        static std::vector<StringName> tilesUidMapping;
};
}

#endif //GLOBAL_DATA_SINGLETON