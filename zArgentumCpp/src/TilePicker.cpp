
#include "TilePicker.h"

#include <godot_cpp/variant/utility_functions.hpp>
#include <string>
#include <vector>
#include <stdexcept>

using namespace godot;

std::vector<std::string> TilePicker::getTiles(
    const TileSelectionSet tileSelectionSet, std::unordered_set<std::string> &data, unsigned int seed)
{
    std::vector<std::string> tilesToPlace;
    switch (tileSelectionSet)
    {
        case TEMPERATE:
        {
            if (data.count("tree"))
            {
                //TODO USAR RESOURCELOADER PARA VER LA DATA DE LA TILE EN VEZ DE HACER ESTO
                std::string result = "tree_temp_" + std::to_string(rand() % 8);
                tilesToPlace.push_back(result);
            } 
            if (data.count("continental") && !data.count("peninsuler_caved") && data.count("away_from_coast") && data.count("lake"))
            {
                tilesToPlace.push_back("lake");
            }
            else if (data.count("continental") && !data.count("peninsuler_caved") && data.count("beach"))
            {
                tilesToPlace.push_back("beach_sand");
            }
            else if (!data.count("continental") || data.count("peninsuler_caved"))
            {
                tilesToPlace.push_back("ocean");
            }
            else if (data.count("continental") && !data.count("peninsuler_caved"))
            {
                std::string result = "grass_" + std::to_string(rand() % 4);
                tilesToPlace.push_back(result);
            }
            return tilesToPlace;
        }break;

        case DESERT:
        {
            if (data.count("continental"))
                tilesToPlace.push_back("beach_sand");
            return tilesToPlace;
        }break;

        default:
            godot::UtilityFunctions::printerr("passed TileSelectionSet not implemented");
            throw std::logic_error("passed TileSelectionSet not implemented");
        break;
    }     
    

}

TilePicker::TilePicker()
{
    //TODO USAR RESOURCELOADER
}

TilePicker::~TilePicker(){
    
}