#ifndef __FRACTUREDCONTINENT_GENERATOR_H__
#define __FRACTUREDCONTINENT_GENERATOR_H__

#include "WorldMatrix.cpp"
#include "FormationGenerator.h"

#include <godot_cpp/godot.hpp>
#include <godot_cpp/classes/fast_noise_lite.hpp>
#include <godot_cpp/classes/random_number_generator.hpp>
#include <godot_cpp/classes/os.hpp>
#include <mutex>

namespace godot 
{//NO PONER CUERPOS DE MÉTODOS EN LOS HEADER FILES (AUNQUE ESTÉN VACÍOS). PUEDE CAUSAR PROBLEMAS DE LINKING
class FracturedContinentGenerator : public FormationGenerator
{
    GDCLASS(FracturedContinentGenerator, FormationGenerator)

    private:
        SafeVec m_origin;
        SafeVec m_size;
        std::mutex mtx;

        static constexpr short int DEBUG_RANGE_MAX = 50, DEBUG_RANGE_MIN = -DEBUG_RANGE_MAX; 

        RandomNumberGenerator m_rng;    
        std::unordered_set<SafeVec, SafeVec::hash> m_trees, m_bushes;
        void build(const SafeVec& rangelef, godot::ArgentumTileMap &argentumTileMap, const godot::SafeVec &origin, 
            std::unordered_set<SafeVec, SafeVec::hash>& myBushes, std::unordered_set<SafeVec, SafeVec::hash>& myTrees);

        bool clearOf(const std::unordered_set<SafeVec, SafeVec::hash>& setToCheck, SafeVec coords, uint16_t radius, bool checkForwards = false) const;
        bool isPeninsulerCaved(SafeVec coords) const;
        bool isLake(SafeVec coords) const;
        bool isContinental(SafeVec coords) const;
        float getContinentness(SafeVec coords) const;
        float getBeachness(SafeVec coords) const;
        void placeDungeonEntrances(ArgentumTileMap& argentumTileMap, const unsigned char DUNGEONS_TO_PLACE);
        void resetState();                                        //add to the left of cave
        enum Target { beach,  lake,  cont,  tree,  bush,  ocean,  cave_0,  cave_1,  cave_2, N_TARGETS}; static constexpr std::array<const char*, N_TARGETS>
            TARGETS={"beach","lake","cont","tree","bush","ocean","cave_0","cave_1","cave_2"};//DON'T FORGET TO ADD ANY MISSING ENUM LITERALS
        

        static constexpr unsigned char N_CAVES = Target::N_TARGETS - Target::cave_0;   
        
        OS os;
        const char N_THREADS = std::clamp(os.get_processor_count(), 1, 32);
        //static constexpr unsigned char N_THREADS = 16;//TIENE Q TENER RAIZ CUADRADA ENTERA, 16,4,2
        //static_assert(N_THREADS==1||N_THREADS==4||N_THREADS==9||N_THREADS==16);
        

    protected:
        static void _bind_methods();

    public:
        FracturedContinentGenerator();
        ~FracturedContinentGenerator();

        FastNoiseLite continenter, peninsuler, bigLaker, smallLaker, bigBeacher, smallBeacher, forest;
        float continental_cutoff, peninsuler_cutoff, bigLakeCutoff, smallLakeCutoff, beachCutoff, treeCutoff;

        float get_continental_cutoff() const; void set_continental_cutoff(float cutoff);

//en vez de poner optional parameters así, declarar varios métodos overloaded, hacer q el de menos llame al de más, y bindear los dos
        void generate(ArgentumTileMap& argentumTileMap, 
            const SafeVec& origin, const SafeVec& size, const Ref<Resource>& tileSelectionSet,
            const unsigned int seed = 0, const Dictionary& data = Dictionary()) override;


};//ENDCLASS
}


#endif // __FRACTUREDCONTINENT_GENERATOR_H__