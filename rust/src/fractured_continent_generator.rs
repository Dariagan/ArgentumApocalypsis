use std::borrow::{BorrowMut};
use std::thread::{self, JoinHandle};

use fastnoise_lite::{FastNoiseLite, NoiseType};
use godot::{builtin::Dictionary};

use godot::prelude::*;
use rand::{thread_rng, Rng};
use strum::{EnumCount, VariantNames}; use strum_macros::{EnumCount as EnumCountMacro};
use crate::utils::*;
use crate::formation_generator::*;
use crate::tiling::*;

pub struct FracturedContinentGenerator{}

#[derive(strum_macros::VariantNames, EnumCount)]
#[strum(serialize_all = "snake_case")]
enum Target{ Beach = 0, Lake, Cont, Tree, Bush, Ocean, Cave0, Cave1, Cave2}

impl IFormationGenerator for FracturedContinentGenerator {
    fn generate(mut world: WorldMatrix, origin: UnsVec, size: UnsVec,
        tile_selection: Gd<TileSelection>, seed: i32, data: Dictionary,
    ) -> WorldMatrix{unsafe{
        
        let world_ptr: SendMutPtr<WorldMatrix> = make_mut_ptr!(world.borrow_mut());
        
        let mut nidordist_mapped_to_targets: [NidOrDist; Target::COUNT] = Default::default();
        crate::tiling::fill_targets(&mut nidordist_mapped_to_targets, Target::VARIANTS, tile_selection);

        let nids_mapped_to_targets: SendPtr<[NidOrDist; Target::COUNT]> = make_ptr!(&nidordist_mapped_to_targets);
        
        //https://github.com/Auburn/FastNoiseLite/tree/master/Rust
        //

        let continenter_cutoff: f32 = 0.61*f32::powf((size.length()/1600.0) as f32,0.05);
        let mut continenter=FastNoiseLite::new();continenter.noise_type=NoiseType::OpenSimplex2;continenter.seed=seed;
        continenter.set_fractal_lacunarity(Some(2.8));continenter.set_fractal_weighted_strength(Some(0.5));
        continenter.frequency = 0.15/f32::powf(size.length() as f32, 0.995);

        let mut continenter_sampling_offset: UnsVec = UnsVec { lef: 0, right: 0 };
        {
            let center: UnsVec = (origin + size)/2;
            while continenter.get_noise_2d((center+continenter_sampling_offset).lef as f64, (center+continenter_sampling_offset).right as f64) < continenter_cutoff + 0.13{
                continenter_sampling_offset += UnsVec{lef:3, right:3}
            }
        }
        let continenter_sampling_offset = continenter_sampling_offset;

        let continenter: SendPtr<FastNoiseLite> = make_ptr!(&continenter);

        const PENINSULER_CUTOFF: f32 = -0.1;
        let mut peninsuler=FastNoiseLite::new();peninsuler.noise_type=NoiseType::OpenSimplex2;peninsuler.seed=seed+1;
        peninsuler.set_fractal_gain(Some(0.56));
        peninsuler.frequency = 5.9/f32::powf(size.length() as f32, 0.995);
        let peninsuler: SendPtr<FastNoiseLite> = make_ptr!(&peninsuler);

        //let arcednoisexample: std::sync::Arc<FastNoiseLite> = FastNoiseLite::new().into();

        const BIG_LAKER_CUTOFF: f32 = 0.33;
        let mut big_laker=FastNoiseLite::new();big_laker.noise_type=NoiseType::ValueCubic;big_laker.seed=seed+2;
        big_laker.frequency = 40.0/f32::powf(size.length() as f32, 0.995);
        let big_laker: SendPtr<FastNoiseLite> = make_ptr!(&big_laker);
        const SMALL_LAKER_CUTOFF: f32 = 0.25; 
        let mut small_laker=FastNoiseLite::new();small_laker.noise_type=NoiseType::ValueCubic;small_laker.seed=seed+3;
        small_laker.frequency = 80.0/f32::powf(size.length() as f32, 0.995);
        let small_laker: SendPtr<FastNoiseLite> = make_ptr!(&small_laker);

        const BEACHER_CUTOFF: f32 = 0.8; 
        let mut big_beacher=FastNoiseLite::new();big_beacher.noise_type=NoiseType::OpenSimplex2S;big_beacher.seed=seed+4;
        big_beacher.frequency = 4.3/f32::powf(size.length() as f32, 0.995);
        let big_beacher: SendPtr<FastNoiseLite> = make_ptr!(&big_beacher);

        let mut small_beacher=FastNoiseLite::new();small_beacher.noise_type=NoiseType::OpenSimplex2S;small_beacher.seed=seed+5;
        small_beacher.set_fractal_octaves(Some(3));
        small_beacher.frequency = 8.0/f32::powf(size.length() as f32, 0.995);
        let small_beacher: SendPtr<FastNoiseLite> = make_ptr!(&small_beacher);

        const FORESTER_CUTOFF: f32 = 4.3; 
        let mut forester=FastNoiseLite::new();forester.noise_type=NoiseType::OpenSimplex2;forester.seed=seed+6;
        forester.set_fractal_lacunarity(Some(3.0));forester.set_fractal_gain(Some(0.77));
        forester.frequency = 1.6/f32::powf(size.length() as f32, 0.995);
        let forester: SendPtr<FastNoiseLite> = make_ptr!(&forester);

        const N_THREADS: usize = 4;
        let mut threads: [Option<JoinHandle<()>>; N_THREADS] = Default::default();

        for thread_i in 0..N_THREADS {threads[thread_i] = Some(thread::spawn(move || {
            let hori_range = (((thread_i*size.lef as usize)/N_THREADS) as u32, (((thread_i+1)*size.lef as usize)/N_THREADS) as u32);
            for rel_coords in (hori_range.0..hori_range.1).flat_map(|i| (0..size.right).map(move |j| UnsVec::from((i,j)))){
            
                let mut tiles_2b_placed: [(TileUnid, TileZLevel); TileZLevel::COUNT] = Default::default();
                
                let continenter=continenter;let peninsuler=peninsuler;let big_laker=big_laker;let small_laker=small_laker;let big_beacher=big_beacher;let small_beacher=small_beacher;let forester=forester;
                let continental = is_continental(continenter, rel_coords, size, continenter_cutoff, None, Some(continenter_sampling_offset));

                let peninsuler_caved = nv_surpasses_cutoff(peninsuler, rel_coords, PENINSULER_CUTOFF);

                if continental && peninsuler_caved{
                    *tiles_2b_placed.get_unchecked_mut(TileZLevel::Bottom as usize) = nids_mapped_to_targets.drf().get_unchecked(Target::Cont as usize).get_a_nid();
                }
                else {
                    *tiles_2b_placed.get_unchecked_mut(TileZLevel::Bottom as usize) = nids_mapped_to_targets.drf().get_unchecked(Target::Ocean as usize).get_a_nid();
                }
  
                overwrite_formation_tile(world_ptr, (origin, rel_coords), *tiles_2b_placed.get_unchecked(0), None)
                
            }
        }))}  
        
        for thread in threads {
            if let Some(thread) = thread{
                let _ = thread.join();
            }
        }
        world
        
    }}
}



