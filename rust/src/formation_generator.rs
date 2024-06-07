use std::any::Any;
use std::borrow::Borrow;
use std::ops::Index;

pub use crate::tiling::TileSelection;
use crate::uns_vec::UnsVec;
use crate::world_matrix::{GdTileSelectionIterator, TileDistribution, TileTypeNid, NULL_TILE};
pub use crate::{safe_vec::SafeVec, world_matrix::WorldMatrix};
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
        seed: i64,
        data: Dictionary,
    ) -> WorldMatrix;
}

pub fn get_border_closeness_factor(
    coords: &UnsVec,
    world_size: &UnsVec,
    power: Option<f64>,
) -> f64 {
    let power = power.unwrap_or(3.0);

    let horizontal_border_closeness: f64 = ((coords.lef as f64 - world_size.lef as f64 / 2.0)
        / (world_size.lef as f64 / 2.0))
        .abs()
        .powf(power);
    let vertical_border_closeness: f64 = ((coords.right as f64 - world_size.right as f64 / 2.0)
        / (world_size.right as f64 / 2.0))
        .abs()
        .powf(power);

    horizontal_border_closeness.max(vertical_border_closeness)
}

pub fn place_tile(world_matrix: *mut WorldMatrix, coords_relative2_formation_origin: UnsVec){
    unsafe{
        
    }
}


pub enum NidOrNidDistribution{
    Nid(TileTypeNid),
    Distribution(Vec<(TileTypeNid, i64)>)
}
impl Default for NidOrNidDistribution{fn default() -> Self {Self::Nid(TileTypeNid::default())}}


pub fn fill_targets(nids_arr: &mut[NidOrNidDistribution], target_names: &[&str], tile_selection: Gd<TileSelection>){
    assert_eq!(nids_arr.len(), target_names.len());

    GdTileSelectionIterator::new(tile_selection.clone()).zip(tile_selection.bind().targets().iter_shared())
        .for_each(|it| {
            let (nid_or_distribution, target) = it;

            if let Some(target_i) = target_names.into_iter().position(|name: &&str| *name == target.to_string().as_str()) {
                unsafe{
                    *nids_arr.get_unchecked_mut(target_i) = nid_or_distribution
                }
            }
            else {godot_script_error!("target {} not found: ", target);}
        })
}