use std::thread::current;

use crate::safe_vec::SafeVec;
pub use crate::tiling::TileSelection;
use crate::uns_vec::UnsVec;
use crate::world_matrix::*;
pub use crate::{world_matrix::WorldMatrix};
use fastnoise_lite::FastNoiseLite;
pub use godot::builtin::Dictionary;
use enum_primitive_derive::Primitive;
use godot::obj::{Base, Gd, GdRef};
use godot::prelude::*;

#[derive(GodotConvert, Var, Export, Primitive)]
#[godot(via = i64)]
pub enum FormGenEnum {
    FracturedFormationGenerator = 0,
}


pub trait IFormationGenerator {
    fn generate(
        world: WorldMatrix,
        origin: UnsVec,
        size: UnsVec,
        tile_selection: Gd<TileSelection>,
        seed: i32,
        data: Dictionary,
    ) -> WorldMatrix;
}



pub fn overwrite_formation_tile(mut world: SendMutPtr<WorldMatrix>, (origin, relative): (UnsVec, UnsVec), (nid, z_level) : (TileTypeNid, TileZLevel), instantiation_data: Option<Dictionary>){
    unsafe{
        world.drf().overwrite_tile(nid, origin+relative, z_level);
        
        return;
        
    }
}

pub fn generate_stateful_instance(world_matrix: *mut WorldMatrix, (origin, relative): (UnsVec, UnsVec), nid: NidOrDist, instantiation_data: Dictionary){
    todo!()
    //llamar señal o algo
}

pub struct CenteredIterator{
    current: SafeVec,
    limits: (i32, i32),
}
impl CenteredIterator{

    pub fn new(size: UnsVec) -> Self {
        Self{
            current: SafeVec {lef: -((size.lef/2) as i32), right: -((size.right/2) as i32)}, 
            limits: ((size.lef/2) as i32, (size.right/2) as i32)
        }
    }
}
//creo que estos ifs lo hacen más lento que un for loop crudo, va a haber que hacer un benchmark comparativo
impl Iterator for CenteredIterator{
    type Item = SafeVec;

    fn next(&mut self) -> Option<Self::Item> {
        
        if self.current.lef >= self.limits.0 {
            return None;
        }
        let result = self.current.clone();

        if self.current.right + 1 >= self.limits.1 {
            self.current.lef += 1;
            self.current.right = 0;
        } else {
            self.current.right += 1;
        }

        Some(result)
    }
}

pub struct SizeIterator{
    current: UnsVec,
    limits: (u32, u32),
}

impl SizeIterator{

    pub fn mono_thread(size: UnsVec) -> Self {
        Self{
            current: UnsVec {lef: 0, right: 0}, 
            limits: (size.lef, size.right)
        }
    }

    pub fn iterate_centered(size: UnsVec) -> Self {
        Self{
            current: UnsVec {lef: 0, right: 0}, 
            limits: (size.lef, size.right)
        }
    }

    pub fn for_thread(size: UnsVec, thread_i: u32, n_threads: u32) -> Self {
        let horizontal_range: UnsVec = UnsVec { 
            lef: (thread_i*size.lef)/n_threads, 
            right: (thread_i*size.right)/n_threads
        };
        Self{
            current: UnsVec {lef: horizontal_range.lef, right: 0}, 
            limits: (horizontal_range.right, size.right)
        }
    }
}
//creo que estos ifs lo hacen más lento que un for loop crudo, va a haber que hacer un benchmark comparativo
impl Iterator for SizeIterator{
    type Item = UnsVec;

    fn next(&mut self) -> Option<Self::Item> {
        
        if self.current.lef >= self.limits.0 {
            return None;
        }
        let result = self.current.clone();

        if self.current.right + 1 >= self.limits.1 {
            self.current.lef += 1;
            self.current.right = 0;
        } else {
            self.current.right += 1;
        }

        Some(result)
    }
}
#[inline]
pub fn is_continental(continenter: SendPtr<FastNoiseLite>, rel_coords: UnsVec, size: UnsVec, continenter_cutoff: f32, power: Option<f32>) -> bool {
    get_continentness(continenter, rel_coords, size, power) > continenter_cutoff
}
#[inline]
pub fn nv_surpasses_cutoff(fast_noise_lite: SendPtr<FastNoiseLite>, rel_coords: UnsVec, cutoff: f32) -> bool{
    get_noise_value(fast_noise_lite, rel_coords) > cutoff
}
#[inline]
pub fn get_noise_value(fast_noise_lite: SendPtr<FastNoiseLite>, rel_coords: UnsVec) -> f32 {
    unsafe{(&*fast_noise_lite.0).get_noise_2d(rel_coords.lef as f64, rel_coords.right as f64)}
}
#[inline]
pub fn get_continentness(continenter: SendPtr<FastNoiseLite>, rel_coords: UnsVec, size: UnsVec, power: Option<f32>) -> f32 {
    let bff = crate::formation_generator::get_border_farness_factor(rel_coords, size, power);
    get_noise_value(continenter, rel_coords) * bff
}
pub fn get_border_farness_factor(
    rel_coords: UnsVec,
    world_size: UnsVec,
    power: Option<f32>,
) -> f32 {
    let power = power.unwrap_or(3.0);

    let horizontal_border_closeness: f32 = ((rel_coords.lef as f32 - world_size.lef as f32 / 2.0)
        / (world_size.lef as f32 / 2.0))
        .abs()
        .powf(power);
    let vertical_border_closeness: f32 = ((rel_coords.right as f32 - world_size.right as f32 / 2.0)
        / (world_size.right as f32 / 2.0))
        .abs()
        .powf(power);

    1.0 - horizontal_border_closeness.max(vertical_border_closeness)
}

pub struct SendPtr<T>(pub *const T);
unsafe impl<T> Send for SendPtr<T> where T: Send {} unsafe impl<T> Sync for SendPtr<T> where T: Sync {}
impl<T> Clone for SendPtr<T> {
    fn clone(&self) -> Self {
        SendPtr(self.0)
    }
}
impl<T> Copy for SendPtr<T> {}
pub struct SendMutPtr<T>(pub *mut T);
unsafe impl<T> Send for SendMutPtr<T> where T: Send {} unsafe impl<T> Sync for SendMutPtr<T> where T: Sync {}
impl<T> Clone for SendMutPtr<T> {
    fn clone(&self) -> Self {
        SendMutPtr(self.0)
    }
}
impl<T> Copy for SendMutPtr<T> {}
macro_rules! make_ptr {
    ($ref:expr) => {
        SendPtr {
            0: $ref as *const _,
        }
    };
}pub(crate) use make_ptr;  
macro_rules! make_mut_ptr {
    ($ref:expr) => {
        SendMutPtr {
            0: $ref as *mut _,
        }
    };
}pub(crate) use make_mut_ptr;  



pub trait DerefPtr {
    type Target;
    unsafe fn drf(&self) -> &Self::Target;
}

pub trait DerefMutPtr {
    type Target;
    unsafe fn drf(&mut self) -> &mut Self::Target;
}

impl<T> DerefPtr for SendPtr<T> {
    type Target = T;
    unsafe fn drf(&self) -> &T {
        &*self.0
    }
}

impl<T> DerefMutPtr for SendMutPtr<T> {
    type Target = T;
    unsafe fn drf(&mut self) -> &mut T {
        &mut *self.0
    }
}